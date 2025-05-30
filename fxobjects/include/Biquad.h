#pragma once

#include <cstring>
#include "IAudioSignalProcessor.h"
#include "EnumsAndStructs.h"

namespace fxobjects {

	class Biquad : public IAudioSignalProcessor
	{
	public:
		Biquad() {}		/* C-TOR */
		~Biquad() {}	/* D-TOR */
	
		// --- IAudioSignalProcessor FUNCTIONS --- //
		//
		/** reset: clear out the state array (flush delays); can safely ignore sampleRate argument - we don't need/use it */
		virtual bool reset(double _sampleRate)
		{
			memset(&stateArray[0], 0, sizeof(double) * numStates);
			return true;  // handled = true
		}
	
		/** return false: this object only processes samples */
		virtual bool canProcessAudioFrame() { return false; }
	
		/** process input x(n) through biquad to produce return value y(n) */
		/**
		\param xn input
		\return the processed sample
		*/
		virtual double processAudioSample(double xn);
	
		/** get parameters: note use of custom structure for passing param data */
		/**
		\return BiquadParameters custom data structure
		*/
		BiquadParameters getParameters() { return parameters; }
	
		/** set parameters: note use of custom structure for passing param data */
		/**
		\param BiquadParameters custom data structure
		*/
		void setParameters(const BiquadParameters& _parameters) { parameters = _parameters; }
	
		// --- MUTATORS & ACCESSORS --- //
		/** set the coefficient array NOTE: passing by pointer to array; allows us to use "array notation" with pointers i.e. [ ] */
		void setCoefficients(double* coeffs) {
			// --- fast block memory copy:
			memcpy(&coeffArray[0], &coeffs[0], sizeof(double) * numCoeffs);
		}
	
		/** get the coefficient array for read/write access to the array (not used in current objects) */
		double* getCoefficients()
		{
			// --- read/write access to the array (not used)
			return &coeffArray[0];
		}
	
		/** get the state array for read/write access to the array (used only in direct form oscillator) */
		double* getStateArray()
		{
			// --- read/write access to the array (used only in direct form oscillator)
			return &stateArray[0];
		}
	
		/** get the structure G (gain) value for Harma filters; see 2nd Ed FX book */
		double getG_value() { return coeffArray[a0]; }
	
		/** get the structure S (storage) value for Harma filters; see 2nd Ed FX book */
		double getS_value();// { return storageComponent; }
	
	protected:
		/** array of coefficients */
		double coeffArray[numCoeffs] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
	
		/** array of state (z^-1) registers */
		double stateArray[numStates] = { 0.0, 0.0, 0.0, 0.0 };
	
		/** type of calculation (algorithm  structure) */
		BiquadParameters parameters;
	
		/** for Harma loop resolution */
		double storageComponent = 0.0;
	};
} // namespace fxobjects
