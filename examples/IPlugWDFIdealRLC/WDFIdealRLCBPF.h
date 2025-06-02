/**
\class WDFIdealRLCBPF
\ingroup WDF-Objects
\brief
The WDFIdealRLCBPF object implements an ideal RLC BPF using the WDF library.

Audio I/O:
- Processes mono input to mono output.

Control I/F:
- Use WDFParameters structure to get/set object params.

\author Will Pirkle http://www.willpirkle.com
\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/

#pragma once

#include "include/IAudioSignalProcessor.h"
#include "include/WdfAdaptorBase.h"
#include "include/WdfEnumsStructs.h"
#include "include/WdfSeriesAdaptor.h"
#include "include/WdfParallelTerminatedAdaptor.h"

using namespace fxobjects;

class WDFIdealRLCBPF : public IAudioSignalProcessor
{
public:
	WDFIdealRLCBPF(void) { createWDF(); }	/* C-TOR */
	~WDFIdealRLCBPF(void) {}	/* D-TOR */

	/** reset members to initialized state */
	virtual bool reset(double _sampleRate)
	{
		sampleRate = _sampleRate;
		// --- rest WDF components (flush state registers)
		seriesAdaptor_LC.reset(_sampleRate);
		parallelTerminatedAdaptor_R.reset(_sampleRate);

		// --- intialize the chain of adapters
		seriesAdaptor_LC.initializeAdaptorChain();
		return true;
	}

	/** return false: this object only processes samples */
	virtual bool canProcessAudioFrame() { return false; }

	/** process input x(n) through the WDF Ideal RLC filter to produce return value y(n) */
	/**
	\param xn input
	\return the processed sample
	*/
	virtual double processAudioSample(double xn)
	{
		// --- push audio sample into series L1
		seriesAdaptor_LC.setInput1(xn);

		// --- output is at terminated L2's output2
		//     note compensation scaling by -6dB = 0.5
		//     because of WDF assumption about Rs and Rload
		return 0.5 * parallelTerminatedAdaptor_R.getOutput2();
	}

	/** create the WDF structure*/
	void createWDF()
	{
		// --- create components, init to noramlized values fc =
		//	   initial values for fc = 1kHz Q = 0.707
		//     Holding C Constant at 1e-6
		//			   L = 2.533e-2
		//			   R = 2.251131 e2
		seriesAdaptor_LC.setComponent(wdfComponent::seriesLC, 2.533e-2, 1.0e-6);
		parallelTerminatedAdaptor_R.setComponent(wdfComponent::R, 2.251131e2);

		// --- connect adapters
		WdfAdaptorBase::connectAdaptors(&seriesAdaptor_LC, &parallelTerminatedAdaptor_R);

		// --- set source resistance
		seriesAdaptor_LC.setSourceResistance(0.0); // --- Rs = 600

		// --- set open ckt termination
		parallelTerminatedAdaptor_R.setOpenTerminalResistance(true);
	}

	/** get parameters: note use of custom structure for passing param data */
	/**
	\return WDFParameters custom data structure
	*/
	WDFParameters getParameters() { return wdfParameters; }

	/** set parameters: note use of custom structure for passing param data */
	/**
	\param WDFParameters custom data structure
	*/
	void setParameters(const WDFParameters& _wdfParameters)
	{
		if (_wdfParameters.fc != wdfParameters.fc ||
			_wdfParameters.Q != wdfParameters.Q ||
			_wdfParameters.boostCut_dB != wdfParameters.boostCut_dB ||
			_wdfParameters.frequencyWarping != wdfParameters.frequencyWarping)
		{
			wdfParameters = _wdfParameters;
			double fc_Hz = wdfParameters.fc;

			if (wdfParameters.frequencyWarping)
			{
				double arg = (kPi * fc_Hz) / sampleRate;
				fc_Hz = fc_Hz * (tan(arg) / arg);
			}

			double inductorValue = 1.0 / (1.0e-6 * pow((2.0 * kPi * fc_Hz), 2.0));
			double resistorValue = (1.0 / wdfParameters.Q) * (pow(inductorValue / 1.0e-6, 0.5));

			seriesAdaptor_LC.setComponentValue_LC(inductorValue, 1.0e-6);
			parallelTerminatedAdaptor_R.setComponentValue(resistorValue);
			seriesAdaptor_LC.initializeAdaptorChain();
		}
	}

protected:
	WDFParameters wdfParameters;	///< object parameters

	// --- adapters
	WdfSeriesAdaptor				seriesAdaptor_LC; ///< adaptor for LC
	WdfParallelTerminatedAdaptor	parallelTerminatedAdaptor_R; ///< adaptor for R

	double sampleRate = 1.0;
};