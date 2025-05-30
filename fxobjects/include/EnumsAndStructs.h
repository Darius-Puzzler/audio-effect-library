#pragma once

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
} // namespace fxobjects
