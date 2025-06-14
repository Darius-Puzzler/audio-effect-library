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

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2018 / 09 / 7
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

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2018 / 09 / 7
	*/
	enum class generatorWaveform { kTriangle, kSin, kSaw };

	/**
	\struct OscillatorParameters
	\ingroup FX-Objects
	\brief
	Custom parameter structure for the LFO and DFOscillator objects.

	\author Will Pirkle http://www.willpirkle.com
	\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
	\version Revision : 1.0
	\date Date : 2018 / 09 / 7
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
			return *this;
		}

		// --- individual parameters
		generatorWaveform waveform = generatorWaveform::kTriangle; ///< the current waveform
		double frequency_Hz = 0.0;	///< oscillator frequency
	};
} // namespace fxobjects
