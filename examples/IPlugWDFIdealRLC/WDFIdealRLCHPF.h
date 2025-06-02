/**
\class WDFIdealRLCHPF
\ingroup WDF-Objects
\brief
The WDFIdealRLCHPF object implements an ideal RLC HPF using the WDF library.

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

class WDFIdealRLCHPF : public IAudioSignalProcessor
{
public:
	WDFIdealRLCHPF(void) { createWDF(); }	/* C-TOR */
	~WDFIdealRLCHPF(void) {}	/* D-TOR */

	/** reset members to initialized state */
	virtual bool reset(double _sampleRate)
	{
		sampleRate = _sampleRate;
		// --- rest WDF components (flush state registers)
		seriesAdaptor_RC.reset(_sampleRate);
		parallelTerminatedAdaptor_L.reset(_sampleRate);

		// --- intialize the chain of adapters
		seriesAdaptor_RC.initializeAdaptorChain();
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
		seriesAdaptor_RC.setInput1(xn);

		// --- output is at terminated L2's output2
		//     note compensation scaling by -6dB = 0.5
		//     because of WDF assumption about Rs and Rload
		return 0.5 * parallelTerminatedAdaptor_L.getOutput2();
	}

	/** create WDF structure; may be called more than once */
	void createWDF()
	{
		// --- create components, init to noramlized values fc =
		//	   initial values for fc = 1kHz Q = 0.707
		//     Holding C Constant at 1e-6
		//			   L = 2.533e-2
		//			   R = 2.251131 e2
		seriesAdaptor_RC.setComponent(wdfComponent::seriesRC, 2.251131e2, 1.0e-6);
		parallelTerminatedAdaptor_L.setComponent(wdfComponent::L, 2.533e-2);

		// --- connect adapters
		WdfAdaptorBase::connectAdaptors(&seriesAdaptor_RC, &parallelTerminatedAdaptor_L);

		// --- set source resistance
		seriesAdaptor_RC.setSourceResistance(0.0); // --- Rs = 600

		// --- set open ckt termination
		parallelTerminatedAdaptor_L.setOpenTerminalResistance(true);
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

			seriesAdaptor_RC.setComponentValue_RC(resistorValue, 1.0e-6);
			parallelTerminatedAdaptor_L.setComponentValue(inductorValue);
			seriesAdaptor_RC.initializeAdaptorChain();
		}
	}


protected:
	WDFParameters wdfParameters;	///< object parameters

	// --- three
	WdfSeriesAdaptor				seriesAdaptor_RC;				///< adaptor for RC
	WdfParallelTerminatedAdaptor	parallelTerminatedAdaptor_L;	///< adaptor for L

	double sampleRate = 1.0;	///< sample rate storage
};