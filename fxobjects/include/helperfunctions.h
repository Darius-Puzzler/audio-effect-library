#pragma once

#include "Constants.h"
#include <math.h>

namespace fxobjects
	{
		/**
	@checkFloatUnderflow
	\ingroup FX-Functions

	@brief Perform underflow check; returns true if we did underflow (user may not care)

	\param value - the value to check for underflow
	\return true if overflowed, false otherwise
	*/
	inline bool checkFloatUnderflow(double& value)
	{
		bool retValue = false;
		if (value > 0.0 && value < kSmallestPositiveFloatValue)
		{
			value = 0;
			retValue = true;
		}
		else if (value < 0.0 && value > kSmallestNegativeFloatValue)
		{
			value = 0;
			retValue = true;
		}
		return retValue;
	}

	/**
	@doLinearInterpolation
	\ingroup FX-Functions

	@brief performs linear interpolation of x distance between two (x,y) points;
	returns interpolated value

	\param x1 - the x coordinate of the first point
	\param x2 - the x coordinate of the second point
	\param y1 - the y coordinate of the first point
	\param y2 - the 2 coordinate of the second point
	\param x - the interpolation location
	\return the interpolated value or y1 if the x coordinates are unusable
	*/
	inline double doLinearInterpolation(double x1, double x2, double y1, double y2, double x)
	{
		double denom = x2 - x1;
		if (denom == 0)
			return y1; // --- should not ever happen

		// --- calculate decimal position of x
		double dx = (x - x1) / (x2 - x1);

		// --- use weighted sum method of interpolating
		return dx*y2 + (1 - dx)*y1;
	}

	/**
	@doLinearInterpolation
	\ingroup FX-Functions

	@brief performs linear interpolation of fractional x distance between two adjacent (x,y) points;
	returns interpolated value

	\param y1 - the y coordinate of the first point
	\param y2 - the 2 coordinate of the second point
	\param x - the interpolation location as a fractional distance between x1 and x2 (which are not needed)
	\return the interpolated value or y2 if the interpolation is outside the x interval
	*/
	inline double doLinearInterpolation(double y1, double y2, double fractional_X)
	{
		// --- check invalid condition
		if (fractional_X >= 1.0) return y2;

		// --- use weighted sum method of interpolating
		return fractional_X*y2 + (1.0 - fractional_X)*y1;
	}

	/**
	@doLagrangeInterpolation
	\ingroup FX-Functions

	@brief implements n-order Lagrange Interpolation

	\param x - Pointer to an array containing the x-coordinates of the input values
	\param y - Pointer to an array containing the y-coordinates of the input values
	\param n - the order of the interpolator, this is also the length of the x,y input arrays
	\param xbar - The x-coorinates whose y-value we want to interpolate
	\return the interpolated value
	*/
	inline double doLagrangeInterpolation(double* x, double* y, int n, double xbar)
	{
		int i, j;
		double fx = 0.0;
		double l = 1.0;
		for (i = 0; i<n; i++)
		{
			l = 1.0;
			for (j = 0; j<n; j++)
			{
				if (j != i)
					l *= (xbar - x[j]) / (x[i] - x[j]);
			}
			fx += l*y[i];
		}
		return (fx);
	}


	/**
	@boundValue
	\ingroup FX-Functions

	@brief  Bound a value to min and max limits

	\param value - value to bound
	\param minValue - lower bound limit
	\param maxValue - upper bound limit
	*/
	inline void boundValue(double& value, double minValue, double maxValue)
	{
		value = fmin(value, maxValue);
		value = fmax(value, minValue);
	}

	/**
	@doUnipolarModulationFromMin
	\ingroup FX-Functions

	@brief Perform unipolar modulation from a min value up to a max value using a unipolar modulator value

	\param unipolarModulatorValue - modulation value on range [0.0, +1.0]
	\param minValue - lower modulation limit
	\param maxValue - upper modulation limit
	\return the modulated value
	*/
	inline double doUnipolarModulationFromMin(double unipolarModulatorValue, double minValue, double maxValue)
	{
		// --- UNIPOLAR bound
		boundValue(unipolarModulatorValue, 0.0, 1.0);

		// --- modulate from minimum value upwards
		return unipolarModulatorValue*(maxValue - minValue) + minValue;
	}

	/**
	@doUnipolarModulationFromMax
	\ingroup FX-Functions

	@brief Perform unipolar modulation from a max value down to a min value using a unipolar modulator value

	\param unipolarModulatorValue - modulation value on range [0.0, +1.0]
	\param minValue - lower modulation limit
	\param maxValue - upper modulation limit
	\return the modulated value
	*/
	inline double doUnipolarModulationFromMax(double unipolarModulatorValue, double minValue, double maxValue)
	{
		// --- UNIPOLAR bound
		boundValue(unipolarModulatorValue, 0.0, 1.0);

		// --- modulate from maximum value downwards
		return maxValue - (1.0 - unipolarModulatorValue)*(maxValue - minValue);
	}

	/**
	@doBipolarModulation
	\ingroup FX-Functions

	@brief Perform bipolar modulation about a center that his halfway between the min and max values

	\param bipolarModulatorValue - modulation value on range [-1.0, +1.0]
	\param minValue - lower modulation limit
	\param maxValue - upper modulation limit
	\return the modulated value
	*/
	inline double doBipolarModulation(double bipolarModulatorValue, double minValue, double maxValue)
	{
		// --- BIPOLAR bound
		boundValue(bipolarModulatorValue, -1.0, 1.0);

		// --- calculate range and midpoint
		double halfRange = (maxValue - minValue) / 2.0;
		double midpoint = halfRange + minValue;

		return bipolarModulatorValue*(halfRange) + midpoint;
	}

	/**
	@unipolarToBipolar
	\ingroup FX-Functions

	@brief calculates the bipolar [-1.0, +1.0] value FROM a unipolar [0.0, +1.0] value

	\param value - value to convert
	\return the bipolar value
	*/
	inline double unipolarToBipolar(double value)
	{
		return 2.0*value - 1.0;
	}

	/**
	@bipolarToUnipolar
	\ingroup FX-Functions

	@brief calculates the unipolar [0.0, +1.0] value FROM a bipolar [-1.0, +1.0] value

	\param value - value to convert
	\return the unipolar value
	*/
	inline double bipolarToUnipolar(double value)
	{
		return 0.5*value + 0.5;
	}

	/**
	@rawTo_dB
	\ingroup FX-Functions

	@brief calculates dB for given input

	\param raw - value to convert to dB
	\return the dB value
	*/
	inline double raw2dB(double raw)
	{
		return 20.0*log10(raw);
	}

	/**
	@dBTo_Raw
	\ingroup FX-Functions

	@brief converts dB to raw value

	\param dB - value to convert to raw
	\return the raw value
	*/
	inline double dB2Raw(double dB)
	{
		return pow(10.0, (dB / 20.0));
	}

	/**
	@peakGainFor_Q
	\ingroup FX-Functions

	@brief calculates the peak magnitude for a given Q

	\param Q - the Q value
	\return the peak gain (not in dB)
	*/
	inline double peakGainFor_Q(double Q)
	{
		// --- no resonance at or below unity
		if (Q <= 0.707) return 1.0;
		return (Q*Q) / (pow((Q*Q - 0.25), 0.5));
	}

	/**
	@dBPeakGainFor_Q
	\ingroup FX-Functions

	@brief calculates the peak magnitude in dB for a given Q

	\param Q - the Q value
	\return the peak gain in dB
	*/
	inline double dBPeakGainFor_Q(double Q)
	{
		return raw2dB(peakGainFor_Q(Q));
	}

	/**
	@doWhiteNoise
	\ingroup FX-Functions

	@brief calculates a random value between -1.0 and +1.0
	\return the random value on the range [-1.0, +1.0]
	*/
	inline double doWhiteNoise()
	{
		float noise = 0.0;

	#if defined _WINDOWS || defined _WINDLL
		// fNoise is 0 -> 32767.0
		noise = (float)rand();

		// normalize and make bipolar
		noise = 2.f*(noise / 32767.f) - 1.f;
	#else
		// fNoise is 0 -> ARC4RANDOMMAX
		//noise = (float)arc4random();

		// normalize and make bipolar
		noise = 2.0*(noise / ARC4RANDOMMAX) - 1.0;
	#endif

		return noise;
	}

	/**
	@sgn
	\ingroup FX-Functions

	@brief calculates sgn( ) of input
	\param xn - the input value
	\return -1 if xn is negative or +1 if xn is 0 or greater
	*/
	inline double sgn(double xn)
	{
		return (xn > 0) - (xn < 0);
	}

	/**
	@calcWSGain
	\ingroup FX-Functions

	@brief calculates gain of a waveshaper
	\param xn - the input value
	\param saturation  - the saturation control
	\param asymmetry  - the degree of asymmetry
	\return gain value
	*/
	inline double calcWSGain(double xn, double saturation, double asymmetry)
	{
		double g = ((xn >= 0.0 && asymmetry > 0.0) || (xn < 0.0 && asymmetry < 0.0)) ? saturation * (1.0 + 4.0*fabs(asymmetry)) : saturation;
		return g;
	}

	/**
	@atanWaveShaper
	\ingroup FX-Functions

	@brief calculates arctangent waveshaper
	\param xn - the input value
	\param saturation  - the saturation control
	\return the waveshaped output value
	*/
	inline double atanWaveShaper(double xn, double saturation)
	{
		return atan(saturation*xn) / atan(saturation);
	}

	/**
	@tanhWaveShaper
	\ingroup FX-Functions

	@brief calculates hyptan waveshaper
	\param xn - the input value
	\param saturation  - the saturation control
	\return the waveshaped output value
	*/
	inline double tanhWaveShaper(double xn, double saturation)
	{
		return tanh(saturation*xn) / tanh(saturation);
	}

	/**
	@softClipWaveShaper
	\ingroup FX-Functions

	@brief calculates hyptan waveshaper
	\param xn - the input value
	\param saturation  - the saturation control
	\return the waveshaped output value
	*/
	inline double softClipWaveShaper(double xn, double saturation)
	{
		// --- un-normalized soft clipper from Reiss book
		return sgn(xn)*(1.0 - exp(-fabs(saturation*xn)));
	}

	/**
	@fuzzExp1WaveShaper
	\ingroup FX-Functions

	@brief calculates fuzz exp1 waveshaper
	\param xn - the input value
	\param saturation  - the saturation control
	\return the waveshaped output value
	*/
	inline double fuzzExp1WaveShaper(double xn, double saturation, double asymmetry)
	{
		// --- setup gain
		double wsGain = calcWSGain(xn, saturation, asymmetry);
		return sgn(xn)*(1.0 - exp(-fabs(wsGain*xn))) / (1.0 - exp(-wsGain));
	}


	/**
	@getMagResponse
	\ingroup FX-Functions

	@brief returns the magnitude resonse of a 2nd order H(z) transfer function
	\param theta - the angular frequency to apply
	\param a0, a1, a2, b1, b2 - the transfer function coefficients
	\return the magnigtude response of the transfer function at w = theta
	*/
	inline double getMagResponse(double theta, double a0, double a1, double a2, double b1, double b2)
	{
		double magSqr = 0.0;
		double num = a1*a1 + (a0 - a2)*(a0 - a2) + 2.0*a1*(a0 + a2)*cos(theta) + 4.0*a0*a2*cos(theta)*cos(theta);
		double denom = b1*b1 + (1.0 - b2)*(1.0 - b2) + 2.0*b1*(1.0 + b2)*cos(theta) + 4.0*b2*cos(theta)*cos(theta);

		magSqr = num / denom;
		if (magSqr < 0.0)
			magSqr = 0.0;

		double mag = pow(magSqr, 0.5);

		return mag;
	}

	/**
	\struct ComplexNumber
	\ingroup FX-Objects
	\brief Structure to hold a complex value.

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2018 / 09 / 7
	*/
	struct ComplexNumber
	{
		ComplexNumber() {}
		ComplexNumber(double _real, double _imag)
		{
			real = _real;
			imag = _imag;
		}

		double real = 0.0; ///< real part
		double imag = 0.0; ///< imaginary part
	};

	/**
	@complexMultiply
	\ingroup FX-Functions

	@brief returns the complex product of two complex numbers
	\param c1, c2 - complex numbers to multiply
	\return the complex product of c1 and c2
	*/
	inline ComplexNumber complexMultiply(ComplexNumber c1, ComplexNumber c2)
	{
		ComplexNumber complexProduct;

		// --- real part
		complexProduct.real = (c1.real*c2.real) - (c1.imag*c2.imag);
		complexProduct.imag = (c1.real*c2.imag) + (c1.imag*c2.real);

		return complexProduct;
	}

	/**
	@calcEdgeFrequencies
	\ingroup FX-Functions

	@brief calculagte low and high edge frequencies of BPF or BSF

	\param fc - the center frequency of the BPF or BSF
	\param Q - the Q (fc/BW) of the filter
	\param f_Low - the returned low edge frequency
	\param f_High - the returned high edge frequency
	*/
	inline void calcEdgeFrequencies(double fc, double Q, double& f_Low, double& f_High)
	{
		bool arithmeticBW = true;
		double bandwidth = fc / Q;

		// --- geometric bw = sqrt[ (fLow)(fHigh) ]
		//     arithmetic bw = fHigh - fLow
		if (arithmeticBW)
		{
			f_Low = fc - bandwidth / 2.0;
			f_High = fc + bandwidth / 2.0;
		}
		else
		{
			; // TODO --- add geometric (for homework)
		}

	}
} // namespace fxobjects
