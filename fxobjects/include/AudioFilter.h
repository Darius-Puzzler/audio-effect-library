#pragma once
#include "EnumsAndStructs.h"
#include "IAudioSignalProcessor.h"
#include "Biquad.h"

class AudioFilter : public IAudioSignalProcessor
{
public:
	AudioFilter() {}		/* C-TOR */
	~AudioFilter() {}		/* D-TOR */

	// --- IAudioSignalProcessor
	/** --- set sample rate, then update coeffs */
	virtual bool reset(double _sampleRate)
	{
		BiquadParameters bqp = biquad.getParameters();

		// --- you can try both forms - do you hear a difference?
		bqp.biquadCalcType = biquadAlgorithm::kTransposeCanonical; //<- this is the default operation
		//	bqp.biquadCalcType = biquadAlgorithm::kDirect;
		biquad.setParameters(bqp);

		sampleRate = _sampleRate;
		return biquad.reset(_sampleRate);
	}

	/** return false: this object only processes samples */
	virtual bool canProcessAudioFrame() { return false; }

	/** process input x(n) through the filter to produce return value y(n) */
	virtual double processAudioSample(double xn);

	/** --- sample rate change necessarily requires recalculation */
	virtual void setSampleRate(double _sampleRate)
	{
		sampleRate = _sampleRate;
		calculateFilterCoeffs();
	}

	/** --- get parameters */
	AudioFilterParameters getParameters() { return audioFilterParameters; }

	AudioFilterParameters& getParametersRef() { return audioFilterParameters; }

	void updateParameters()
	{
		// --- don't allow 0 or (-) values for Q
		if (audioFilterParameters.Q <= 0)
			audioFilterParameters.Q = 0.707;

		// --- update coeffs
		calculateFilterCoeffs();
	}

	/** --- set parameters */
	void setParameters(const AudioFilterParameters& parameters)
	{
		if (audioFilterParameters.algorithm != parameters.algorithm ||
			audioFilterParameters.boostCut_dB != parameters.boostCut_dB ||
			audioFilterParameters.fc != parameters.fc ||
			audioFilterParameters.Q != parameters.Q ||
			audioFilterParameters.gr_index != parameters.gr_index ||
			audioFilterParameters.k != parameters.k)
		{
			// --- save new params
			audioFilterParameters = parameters;
		}
		else
			return;

		// --- don't allow 0 or (-) values for Q
		if (audioFilterParameters.Q <= 0)
			audioFilterParameters.Q = 0.707;

		// --- update coeffs
		calculateFilterCoeffs();
	}

	/** --- helper for Harma filters (phaser) */
	double getG_value() { return biquad.getG_value(); }

	/** --- helper for Harma filters (phaser) */
	double getS_value() { return biquad.getS_value(); }

protected:
	// --- our calculator
	Biquad biquad; ///< the biquad object

	// --- array to hold coeffs (we need them too)
	double coeffArray[numCoeffs] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 }; ///< our local copy of biquad coeffs

	// --- object parameters
	AudioFilterParameters audioFilterParameters; ///< parameters
	double sampleRate = 44100.0; ///< current sample rate

	/** --- function to recalculate coefficients due to a change in filter parameters */
	bool calculateFilterCoeffs();
private:
	static constexpr double gainReduction[10] = { 2750.0, 263.0, 124.0, 78.0, 55.0, 50.0, 30.0, 23.0, 17.0, 12.0, };
};