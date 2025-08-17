/*  Chapter 14 - Homework 4
*
*   Implement the ducking delay in Figure 14.18.
* 
*   Descritpion:
* 
*   Introduced in 1985, the TC-2290 Dynamic Delay helped define a decade of electric guitar solos.
*   The use of delays on electric guitar was already in vogue, but a problem was that the delays tended
*   to muddy the virtuosic guitar work, especially with high delay feedback values. Some guitarists
*   such as Steve Morse used a volume pedal on the wet channel of the delay to fade the echoes
*   in and out as needed. One popular approach was to remove the delays while the guitar signal's
*   amplitude was high, and then fade in the echoes as the guitar signal diminished in amplitude.
*   The TC-2290 accomplished the same technique (and many others) by using an envelope detector
*   in a side chain. When the input signal's envelope is above the threshold, the echo amplitude is
*   lowered significantly, or even reduced to nothing. When the signal falls below the threshold, the
*   amplitude of the echoes is increased, making the notes swell or "bloom" as the echo signals grow.
*   This is known as a "ducking delay" because the echo signal is reduced or "ducked" in response to
*   the playing strength. The effect works very well on many other instruments or audio loops where
*   intense echoes are not desirable during high amplitude periods. Figure 14.18 shows the block
*   diagram of a ducking delay like the TC-2290. In this version, the "wet (min)" control sets the
*   ducked amplitude and the "wet (max)" sets the un-attenuated maximum echo level. These controls
*   will work best in dB. The side-chain gain (SC Gain) control is optional. Notice also that the logic
*   may be reversed to create an "expanding delay" as well.
*/

/**
 * @file DuckingDelay.h
 * @brief Implementation of a ducking delay effect inspired by the TC-2290 Dynamic Delay.
 * 
 * This file contains the DuckingDelay class which implements a stereo audio delay effect
 * with ducking functionality. The effect reduces the delay level when the input signal
 * exceeds a threshold, allowing the dry signal to be more prominent during loud passages.
 */

#pragma once

#include <IPlugAPIBase.h>
#include <Smoothers.h>
#include "include/AudioDelay.h"
#include "include/EnvelopeFollower.h"

using namespace fxobjects;

constexpr double PERCENT_TO_DECIMAL = 0.01;

/** 
 * @enum EParams
 * @brief Enumerates the plugin's parameters.
 */
enum EParams
{
	kGain = 0,
	kGainSC,
	kDelayTimeLeft,
	kDelayTimeRight,
	kDelayFeedbackLeft,
	kDelayFeedbackRight,
	kEnvAttack,
	kEnvRelease,
	kThresh_dB,
	kSensitivity,
	kWetMin,
	kWetMax,
	kNumParams
};

/**
 * @enum EModulations
 * @brief Enumerates the modulation targets for parameter smoothing.
 */
enum EModulations
{
	kModGain = 0,
	kModGainSC,
	kModDelayTimeLeft,
	kModDelayTimeRight,
	kModDelayFeedbackLeft,
	kModDelayFeedbackRight,
	kModEnvAttack,
	kModEnvRelease,
	kModThresh_Linear,
	kModSensitivity,
	kModWetMin,
	kModWetMax,
	kNumModulations
};

/**
 * @class DuckingDelay
 * @brief Implements a ducking delay effect with envelope-followed sidechain control.
 * 
 * The DuckingDelay class provides a stereo delay effect where the wet signal level
 * is automatically reduced when the input signal exceeds a threshold, allowing the
 * dry signal to be more prominent during loud passages.
 */
class DuckingDelay : public EnvelopeFollower
{
public:
	DuckingDelay() = default;
	~DuckingDelay() = default;

	/**
     * @brief Resets the effect with new sample rate and block size.
     * @param _sampleRate The audio sample rate in Hz
     * @param _blockSize The audio block size in samples
     * @return True if successful, false otherwise
     */

	bool reset(double _sampleRate, int _blockSize)
	{
		// smoothing buffer
		mModulationsData.Resize(_blockSize * kNumModulations);
		mModulations.Empty();

		for (int i = 0; i < kNumModulations; i++)
		{
			mModulations.Add(mModulationsData.Get() + (_blockSize * i));
		}

		mAudioDelay.reset(_sampleRate);
		mAudioDelay.createDelayBuffers(_sampleRate, 2000.0);
		EnvelopeFollower::reset(_sampleRate);
		
		mWetSmoother.SetSmoothTime(5.0, _sampleRate);

		return true;
	}

	/**
     * @brief Indicates that this effect processes blocks, not individual samples.
     * @return Always returns false (processes blocks)
     */
	virtual bool canProcessAudioFrame() override
	{
		return false;
	}

	/**
     * @brief Placeholder for sample processing (not used, processing is done in blocks).
     * @param xn Input sample
     * @return Always returns 0 (processing is done in blocks)
     */
	virtual double processAudioSample(double xn) override
	{
		return 0; // dummy - processing is done in blocks
	}

	/**
     * @brief Processes a block of audio samples.
     * @param inputs Array of input sample buffers
     * @param outputs Array of output sample buffers
     * @param nFrames Number of frames to process
     * @param nChannels Number of audio channels (must be 2 for stereo)
     */
	void processBlock(iplug::sample** inputs, iplug::sample** outputs, int nFrames, int nChannels = 2)
	{
		// set outputs zero to avoid fragments
		memset(outputs[0], 0, nFrames * sizeof(iplug::sample));
		memset(outputs[1], 0, nFrames * sizeof(iplug::sample));

		// process the smoothing
		mParameterSmoother.ProcessBlock(mParamsToSmooth, mModulations.GetList(), nFrames);

		// get pointers to the smoothed parameters
		const double* gain = mModulations.GetList()[kModGain]; // pointer to smoothed gain
		const double* gainSC = mModulations.GetList()[kModGainSC]; // pointer to smoothed side chain gain
		// delay
		const double* delayTimeLeft = mModulations.GetList()[kModDelayTimeLeft]; // pointer to smoothed delay time left
		const double* delayTimeRight = mModulations.GetList()[kModDelayTimeRight]; // pointer to smoothed delay time right
		const double* delayFeedbackLeft = mModulations.GetList()[kModDelayFeedbackLeft]; // pointer to smoothed delay feedback left
		const double* delayFeedbackRight = mModulations.GetList()[kModDelayFeedbackRight]; // pointer to smoothed delay feedback right
		// env
		const double* envAttack = mModulations.GetList()[kModEnvAttack];  // pointer to smoothed Envelope Follower Attack
		const double* envRelease = mModulations.GetList()[kModEnvRelease]; // pointer to smoothed Envelope Follower Release
		const double* thresh_Linear = mModulations.GetList()[kModThresh_Linear]; // pointer to smoothed Envelope Follower Threshold
		const double* sensitivity = mModulations.GetList()[kModSensitivity]; // pointer to smoothed Envelope Follower Sensitivity
		// wet/moist
		const double* wetMin_Linear = mModulations.GetList()[kModWetMin]; // pointer to smoothed wet min
		const double* wetMax_Linear = mModulations.GetList()[kModWetMax]; // pointer to smoothed wet max

		float frames[2] = { 0.0, 0.0 }; // array for left and right frame to process in AudioDelay processAudioFrame()

		for (int s = 0; s < nFrames; s++)
		{
			// Update envelope follower parameters
			// Note: Converting threshold from linear to dB here since EnvelopeFollower expects dB
			mEnvelopeFollowerParameters.attackTime_mSec = envAttack[s];
			mEnvelopeFollowerParameters.releaseTime_mSec = envRelease[s];
			mEnvelopeFollowerParameters.threshold_dB = 20.0 * log10(thresh_Linear[s]);
			mEnvelopeFollowerParameters.sensitivity = sensitivity[s];
			// update the base class parameters especially for AudioDetector class parameters
			EnvelopeFollower::setParameters(mEnvelopeFollowerParameters);

			// Calculate threshold in linear domain for signal detection
			double threshValue = pow(10.0, parameters.threshold_dB / 20.0); // threshold converted to linear

			// --- detect the signal
			double detect_L = pow(10.0, detector.processAudioSample(inputs[0][s] * gainSC[s] * PERCENT_TO_DECIMAL) / 20.0); // convert to linear and apply side chain gain pre processing
			double detect_R = pow(10.0, detector.processAudioSample(inputs[1][s] * gainSC[s] * PERCENT_TO_DECIMAL) / 20.0); // convert to linear and apply side chain gain pre processing

			// Use the louder channel for ducking decision
			double detectValue = detect_L > detect_R ? detect_L : detect_R;

			// Calculate how much we're above the threshold
			double deltaValue = detectValue - threshValue;

			double wetMin = wetMin_Linear[s];
			double wetMax = wetMax_Linear[s];
			double newWetValue = wetMax; // Default to max wet level when no signal is present

			// --- if above the threshold, modulate the audio delay wet value
			if (deltaValue > 0.0)// || delta_dB > 0.0)
			{
				// --- wet value Computer
				double modulatorValue = 0.0;

				// --- best results are with linear values when detector is in dB mode
				modulatorValue = (deltaValue * parameters.sensitivity);

				// --- calculate modulated wet value
				boundValue(modulatorValue, 0.0, 1.0);
				newWetValue = wetMax - (wetMax - wetMin) * modulatorValue;
				// or this but requires inverted modulatorValue as parameter to the function
				//newWetValue = doUnipolarModulationFromMax(1.0 - modulatorValue, wetMin, wetMax);
			}

			// smooth new wet value
			double smoothedWet = mWetSmoother.Process(newWetValue);
			// --- update with new modulated and smoothed wet level in dB
			double newWetValue_dB = 20.0 * log10(smoothedWet);
			mAudioDelayParameters.wetLevel_dB = newWetValue_dB;

			mAudioDelayParameters.leftDelay_mSec = delayTimeLeft[s];
			mAudioDelayParameters.rightDelay_mSec = delayTimeRight[s];
			mAudioDelayParameters.leftFeedback_Pct = delayFeedbackLeft[s];
			mAudioDelayParameters.rightFeedback_Pct = delayFeedbackRight[s];

			// update delay parameters for delay processing
			mAudioDelay.setParameters(mAudioDelayParameters);

			// --- perform the delay processing
			frames[0] = inputs[0][s];
			frames[1] = inputs[1][s];

			mAudioDelay.processAudioFrame(frames, frames, 2, 2);
			// apply gain and write to output buffer
			outputs[0][s] = frames[0] * gain[s] * PERCENT_TO_DECIMAL; // post gain
			outputs[1][s] = frames[1] * gain[s] * PERCENT_TO_DECIMAL; // post gain
		}
	}

	/**
     * @brief Sets a parameter value by index.
     * @param _paramIdx The parameter index (from EParams)
     * @param _value The new parameter value
     */
	void SetParameters(const int _paramIdx, const double _value)
	{
		switch (_paramIdx) {
		case kGain:
			mParamsToSmooth[kModGain] = _value; // %
			break;
		case kGainSC:
			mParamsToSmooth[kModGainSC] = _value; // %
			break;
		case kDelayTimeLeft:
			mParamsToSmooth[kModDelayTimeLeft] = _value; // ms
			break;
		case kDelayTimeRight:
			mParamsToSmooth[kModDelayTimeRight] = _value; // ms
			break;
		case kDelayFeedbackLeft:
			mParamsToSmooth[kModDelayFeedbackLeft] = _value; // %
			break;
		case kDelayFeedbackRight:
			mParamsToSmooth[kModDelayFeedbackRight] = _value; // %
			break;
		case kEnvAttack:
			mParamsToSmooth[kModEnvAttack] = _value; // ms
			break;
		case kEnvRelease:
			mParamsToSmooth[kModEnvRelease] = _value; // ms
			break;
		case kThresh_dB:
			mParamsToSmooth[kModThresh_Linear] = pow(10.0, _value / 20.0); // dB, then converted to linear
			break;
		case kSensitivity:
			mParamsToSmooth[kModSensitivity] = _value; // raw double [0.25, 5.0]
			break;
		case kWetMin:
			mParamsToSmooth[kModWetMin] = pow(10.0, _value / 20.0); //dB, then converted to linear
			break;
		case kWetMax:
			mParamsToSmooth[kModWetMax] = pow(10.0, _value / 20.0); // dB, then converted to linear
			break;
		default:
			break;
		}
	}

private:
	AudioDelay mAudioDelay;              ///< Audio delay processor
	AudioDelayParameters mAudioDelayParameters; ///< Current delay parameters
	EnvelopeFollowerParameters mEnvelopeFollowerParameters; ///< Envelope follower parameters

	// Parameter smoothing
	WDL_TypedBuf<double> mModulationsData;              ///< Buffer for smoothed parameter values
	WDL_PtrList<double> mModulations;                   ///< List of parameter smoothers
	double mParamsToSmooth[kNumModulations];            ///< Target values for parameter smoothing
	iplug::LogParamSmooth<double, kNumModulations> mParameterSmoother;  ///< Parameter smoother
	iplug::LogParamSmooth<double> mWetSmoother;         ///< Smoother for wet level modulation
};