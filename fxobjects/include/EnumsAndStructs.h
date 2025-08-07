/*
\author Will Pirkle http://www.willpirkle.com
\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/

#pragma once

#include "Constants.h"

namespace fxobjects {

	enum filterCoeff { a0, a1, a2, b1, b2, c0, d0, numCoeffs };
	
	enum stateReg { x_z1, x_z2, y_z1, y_z2, numStates };
	
	// type of calculation (algorithm)
	enum class biquadAlgorithm { kDirect, kCanonical, kTransposeDirect, kTransposeCanonical };
	
	// Filter Algoritms
	enum class filterAlgorithm {
		kLPF1P, kLPF1, kHPF1, kLPF2, kHPF2, kBPF2, kBPF2Boost, kBSF2, kButterLPF2, kButterHPF2, kButterBPF2,
		kButterBSF2, kMMALPF2A, kMMALPF2B, kMMALPF2C, kMMALPF2D, kLowShelf, kHiShelf, kNCQParaEQ, kCQParaEQ, kLWRLPF2, kLWRHPF2,
		kAPF1, kAPF2, kRM1, kRM2, kResonA, kResonB, kMatchLP2A, kMatchLP2B, kMatchBP2A, kMatchBP2B,
		kImpInvLP1, kImpInvLP2
	};
	
	// Biquad Parameters
	struct BiquadParameters
	{
		BiquadParameters() {}
	
		// all parameter objects require overloaded= operator so remember to add new entries if you add new variables.
		BiquadParameters& operator=(const BiquadParameters& params)
		{
			if (this == &params)
				return *this;
	
			biquadCalcType = params.biquadCalcType;
			return *this;
		}
	
		biquadAlgorithm biquadCalcType = biquadAlgorithm::kDirect; // biquad structure to use
	};
	
	// Audio Filter Parameters
	struct AudioFilterParameters
	{
		AudioFilterParameters() {}
		/** all FXObjects parameter objects require overloaded= operator so remember to add new entries if you add new variables. */
		AudioFilterParameters& operator=(const AudioFilterParameters& params)	// need this override for collections to work
		{
			if (this == &params)
				return *this;
			algorithm = params.algorithm;
			fc = params.fc;
			Q = params.Q;
			boostCut_dB = params.boostCut_dB;
			gr_index = params.gr_index;
			k = params.k;
	
			return *this;
		}
	
		// --- individual parameters
		filterAlgorithm algorithm = filterAlgorithm::kLPF2; ///< filter algorithm
		double fc = 100.0; ///< filter cutoff or center frequency (Hz)
		double Q = 0.707; ///< filter Q
		double boostCut_dB = 0.0; ///< filter gain; note not used in all types
		int gr_index = 1;
		double k = 0.0;
	};

	/**
	\struct AudioDetectorParameters
	\ingroup FX-Objects
	\brief
	Custom parameter structure for the AudioDetector object. NOTE: this object uses constant defintions:

	- const unsigned int TLD_AUDIO_DETECT_MODE_PEAK = 0;
	- const unsigned int TLD_AUDIO_DETECT_MODE_MS = 1;
	- const unsigned int TLD_AUDIO_DETECT_MODE_RMS = 2;
	- const double TLD_AUDIO_ENVELOPE_ANALOG_TC = -0.99967234081320612357829304641019; // ln(36.7%)
	*/
	struct AudioDetectorParameters
	{
		AudioDetectorParameters() {}
		/** all FXObjects parameter objects require overloaded= operator so remember to add new entries if you add new variables. */
		AudioDetectorParameters& operator=(const AudioDetectorParameters& params)	// need this override for collections to work
		{
			if (this == &params)
				return *this;
			attackTime_mSec = params.attackTime_mSec;
			releaseTime_mSec = params.releaseTime_mSec;
			detectMode = params.detectMode;
			detect_dB = params.detect_dB;
			clampToUnityMax = params.clampToUnityMax;
			return *this;
		}

		// --- individual parameters
		double attackTime_mSec = 0.0; ///< attack time in milliseconds
		double releaseTime_mSec = 0.0;///< release time in milliseconds
		unsigned int  detectMode = 0;///< detect mode, see TLD_ constants above
		bool detect_dB = false;	///< detect in dB  DEFAULT  = false (linear NOT log)
		bool clampToUnityMax = true;///< clamp output to 1.0 (set false for true log detectors)
	};

	// --- structure to send output data from signal gen; you can add more outputs here
	struct SignalGenData
	{
		SignalGenData() {}

		double normalOutput = 0.0;			///< normal
		double invertedOutput = 0.0;		///< inverted
		double quadPhaseOutput_pos = 0.0;	///< 90 degrees out
		double quadPhaseOutput_neg = 0.0;	///< -90 degrees out
	};

	/**
	\enum generatorWaveform
	\ingroup Constants-Enums
	\brief
	Use this strongly typed enum to easily set the oscillator waveform

	- enum  generatorWaveform { kTriangle, kSin, kSaw };
	*/
	enum class generatorWaveform { kTriangle, kSin, kSaw };

	/**
	\struct OscillatorParameters
	\ingroup FX-Objects
	\brief
	Custom parameter structure for the LFO and DFOscillator objects.
	*/
	struct OscillatorParameters
	{
		OscillatorParameters() {}
		/** all FXObjects parameter objects require overloaded= operator so remember to add new entries if you add new variables. */
		OscillatorParameters& operator=(const OscillatorParameters& params)	// need this override for collections to work
		{
			if (this == &params)
				return *this;

			waveform = params.waveform;
			frequency_Hz = params.frequency_Hz;
			amplitude_fac = (params.amplitude_fac >= 0.0 && params.amplitude_fac <= 1.0)
				? params.amplitude_fac
				: amplitude_fac; // keep current value if out of range
			return *this;
		}

		// --- individual parameters
		generatorWaveform waveform = generatorWaveform::kSin; ///< the current waveform
		double frequency_Hz = 0.0;	///< oscillator frequency
		double amplitude_fac = 1.0; // amplitude factor [0, +1], 0 is no amplitude
	};
	
	/**
	\struct EnvelopeFollowerParameters
	\ingroup FX-Objects
	\brief
	Custom parameter structure for the EnvelopeFollower object.
	*/
	struct EnvelopeFollowerParameters
	{
		EnvelopeFollowerParameters() {}
		/** all FXObjects parameter objects require overloaded= operator so remember to add new entries if you add new variables. */
		EnvelopeFollowerParameters& operator=(const EnvelopeFollowerParameters& params)	// need this override for collections to work
		{
			if (this == &params)
				return *this;

			fc = params.fc;
			Q = params.Q;
			attackTime_mSec = params.attackTime_mSec;
			releaseTime_mSec = params.releaseTime_mSec;
			threshold_dB = params.threshold_dB;
			sensitivity = params.sensitivity;

			return *this;
		}

		// --- individual parameters
		double fc = 0.0;				///< filter fc
		double Q = 0.707;				///< filter Q
		double attackTime_mSec = 10.0;	///< detector attack time
		double releaseTime_mSec = 10.0;	///< detector release time
		double threshold_dB = 0.0;		///< detector threshold in dB
		double sensitivity = 1.0;		///< detector sensitivity
	};

	/**
	\struct PhaseShifterParameters
	\ingroup FX-Objects
	\brief
	Custom parameter structure for the PhaseShifter object.
	*/
	struct PhaseShifterParameters
	{
		PhaseShifterParameters() {}
		/** all FXObjects parameter objects require overloaded= operator so remember to add new entries if you add new variables. */
		PhaseShifterParameters& operator=(const PhaseShifterParameters& params)
		{
			if (this == &params)
				return *this;

			lfoRate_Hz = params.lfoRate_Hz;
			lfoDepth_Pct = params.lfoDepth_Pct;
			lfoAmplitude_fac = (params.lfoAmplitude_fac >= 0.0 && params.lfoAmplitude_fac <= 1.0)
				? params.lfoAmplitude_fac
				: lfoAmplitude_fac; // keep current value if out of range
			intensity_Pct = params.intensity_Pct;
			quadPhaseLFO = params.quadPhaseLFO;
			return *this;
		}

		// --- individual parameters
		double lfoRate_Hz = 0.0;	///< phaser LFO rate in Hz
		double lfoDepth_Pct = 0.0;	///< phaser LFO depth in %
		double lfoAmplitude_fac = 1.0; // amplitude factor [0, +1], 0 is no amplitude
		double intensity_Pct = 0.0;	///< phaser feedback in %
		bool quadPhaseLFO = false;	///< quad phase LFO flag
	};

	// homework chapter 13-2
	enum class Polarity
	{
		kBipolar,
		kUnipolar
	};

	/**
	\enum delayAlgorithm
	\ingroup Constants-Enums
	\brief
	Use this strongly typed enum to easily set the delay algorithm

	- enum class delayAlgorithm { kNormal, kPingPong };
	*/
	enum class delayAlgorithm { kNormal, kPingPong };

	/**
	\enum delayUpdateType
	\ingroup Constants-Enums
	\brief
	Use this strongly typed enum to easily set the delay update type; this varies depending on the designer's choice
	of GUI controls. See the book reference for more details.

	- enum class delayUpdateType { kLeftAndRight, kLeftPlusRatio };
	*/
	enum class delayUpdateType { kLeftAndRight, kLeftPlusRatio };

	/**
	\struct AudioDelayParameters
	\ingroup FX-Objects
	\brief
	Custom parameter structure for the AudioDelay object.
	*/
	struct AudioDelayParameters
	{
		AudioDelayParameters() {}
		/** all FXObjects parameter objects require overloaded= operator so remember to add new entries if you add new variables. */
		AudioDelayParameters& operator=(const AudioDelayParameters& params)	// need this override for collections to work
		{
			if (this == &params)
				return *this;

			algorithm = params.algorithm;
			wetLevel_dB = params.wetLevel_dB;
			dryLevel_dB = params.dryLevel_dB;
			leftFeedback_Pct = params.leftFeedback_Pct;
			rightFeedback_Pct = params.rightFeedback_Pct;

			updateType = params.updateType;
			leftDelay_mSec = params.leftDelay_mSec;
			rightDelay_mSec = params.rightDelay_mSec;
			delayRatio_Pct = params.delayRatio_Pct;

			return *this;
		}

		// --- individual parameters
		delayAlgorithm algorithm = delayAlgorithm::kNormal; ///< delay algorithm
		double wetLevel_dB = -3.0;	///< wet output level in dB
		double dryLevel_dB = -3.0;	///< dry output level in dB
		double leftFeedback_Pct = 0.0;	///< left feedback as a % value
		double rightFeedback_Pct = 0.0;	///< right feedback as a % value

		delayUpdateType updateType = delayUpdateType::kLeftAndRight;///< update algorithm
		double leftDelay_mSec = 0.0;	///< left delay time
		double rightDelay_mSec = 0.0;	///< right delay time
		double delayRatio_Pct = 100.0;	///< dela ratio: right length = (delayRatio)*(left length)
	};
} // namespace fxobjects
