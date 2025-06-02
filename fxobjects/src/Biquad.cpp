#include "include/Biquad.h"

using namespace fxobjects;

double Biquad::getS_value()
{
	storageComponent = 0.0;
	if (parameters.biquadCalcType == biquadAlgorithm::kDirect)
	{
		// --- 1)  form output y(n) = a0*x(n) + a1*x(n-1) + a2*x(n-2) - b1*y(n-1) - b2*y(n-2)
		storageComponent = coeffArray[a1] * stateArray[x_z1] +
			coeffArray[a2] * stateArray[x_z2] -
			coeffArray[b1] * stateArray[y_z1] -
			coeffArray[b2] * stateArray[y_z2];
	}
	else if (parameters.biquadCalcType == biquadAlgorithm::kTransposeCanonical)
	{
		// --- 1)  form output y(n) = a0*x(n) + stateArray[x_z1]
		storageComponent = stateArray[x_z1];
	}

	return storageComponent;
}

double Biquad::processAudioSample(double xn)
{
	if (parameters.biquadCalcType == biquadAlgorithm::kDirect)
	{
		// --- 1)  form output y(n) = a0*x(n) + a1*x(n-1) + a2*x(n-2) - b1*y(n-1) - b2*y(n-2)
		double yn = coeffArray[a0] * xn +
			coeffArray[a1] * stateArray[x_z1] +
			coeffArray[a2] * stateArray[x_z2] -
			coeffArray[b1] * stateArray[y_z1] -
			coeffArray[b2] * stateArray[y_z2];

		// --- 2) underflow check
		checkFloatUnderflow(yn);

		// --- 3) update states
		stateArray[x_z2] = stateArray[x_z1];
		stateArray[x_z1] = xn;

		stateArray[y_z2] = stateArray[y_z1];
		stateArray[y_z1] = yn;

		// --- return value
		return yn;
	}
	else if (parameters.biquadCalcType == biquadAlgorithm::kCanonical)
	{
		// --- 1)  form output y(n) = a0*w(n) + m_f_a1*stateArray[x_z1] + m_f_a2*stateArray[x_z2][x_z2];
		//
		// --- w(n) = x(n) - b1*stateArray[x_z1] - b2*stateArray[x_z2]
		double wn = xn - coeffArray[b1] * stateArray[x_z1] - coeffArray[b2] * stateArray[x_z2];

		// --- y(n):
		double yn = coeffArray[a0] * wn + coeffArray[a1] * stateArray[x_z1] + coeffArray[a2] * stateArray[x_z2];

		// --- 2) underflow check
		checkFloatUnderflow(yn);

		// --- 3) update states
		stateArray[x_z2] = stateArray[x_z1];
		stateArray[x_z1] = wn;

		// --- return value
		return yn;
	}
	else if (parameters.biquadCalcType == biquadAlgorithm::kTransposeDirect)
	{
		// --- 1)  form output y(n) = a0*w(n) + stateArray[x_z1]
		//
		// --- w(n) = x(n) + stateArray[y_z1]
		double wn = xn + stateArray[y_z1];

		// --- y(n) = a0*w(n) + stateArray[x_z1]
		double yn = coeffArray[a0] * wn + stateArray[x_z1];

		// --- 2) underflow check
		checkFloatUnderflow(yn);

		// --- 3) update states
		stateArray[y_z1] = stateArray[y_z2] - coeffArray[b1] * wn;
		stateArray[y_z2] = -coeffArray[b2] * wn;

		stateArray[x_z1] = stateArray[x_z2] + coeffArray[a1] * wn;
		stateArray[x_z2] = coeffArray[a2] * wn;

		// --- return value
		return yn;
	}
	else if (parameters.biquadCalcType == biquadAlgorithm::kTransposeCanonical)
	{
		// --- 1)  form output y(n) = a0*x(n) + stateArray[x_z1]
		double yn = coeffArray[a0] * xn + stateArray[x_z1];

		// --- 2) underflow check
		checkFloatUnderflow(yn);

		// --- shuffle/update
		stateArray[x_z1] = coeffArray[a1] * xn - coeffArray[b1] * yn + stateArray[x_z2];
		stateArray[x_z2] = coeffArray[a2] * xn - coeffArray[b2] * yn;

		// --- return value
		return yn;
	}
	return xn; // didn't process anything :(
}