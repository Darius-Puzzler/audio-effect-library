/**
\class WDFButterLPF3
\ingroup WDF-Objects
\brief
The WDFButterLPF3 object implements a 3rd order Butterworth ladder filter.
NOTE: designed with Elsie www.TonneSoftware.comm

Audio I/O:
- Processes mono input to mono output.

Control I/F:
- none - this object is hard-wired.

\author Will Pirkle http://www.willpirkle.com
\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/

#pragma once
#include "include/IAudioSignalProcessor.h"
#include "include/WdfAdaptorBase.h"
#include "include/WdfSeriesAdaptor.h"
#include "include/WdfParallelAdaptor.h"
#include "include/WdfSeriesTerminatedAdaptor.h"
#include "include/IComponentAdaptor.h"

using namespace fxobjects;

class WDFTunableButterLPF3 : public IAudioSignalProcessor
{
public:
	WDFTunableButterLPF3(void) { createWDF(); }	/* C-TOR */
	~WDFTunableButterLPF3(void) {}	/* D-TOR */

	/** reset members to initialized state */
	virtual bool reset(double _sampleRate)
	{
		sampleRate = _sampleRate;

		// --- rest WDF components (flush state registers)
		seriesAdaptor_L1.reset(_sampleRate);
		parallelAdaptor_C1.reset(_sampleRate);
		seriesTerminatedAdaptor_L2.reset(_sampleRate);

		// --- intialize the chain of adapters
		seriesAdaptor_L1.initializeAdaptorChain();
		return true;
	}

	/** return false: this object only processes samples */
	virtual bool canProcessAudioFrame() { return false; }

	/** process input x(n) through the WDF ladder filter to produce return value y(n) */
	/**
	\param xn input
	\return the processed sample
	*/
	virtual double processAudioSample(double xn)
	{
		// --- push audio sample into series L1
		seriesAdaptor_L1.setInput1(xn);

		// --- output is at terminated L2's output2
		return seriesTerminatedAdaptor_L2.getOutput2();
	}

	/** create the WDF structure for this object - may be called more than once */
	void createWDF()
	{
		// --- set adapter components
		seriesAdaptor_L1.setComponent(wdfComponent::L, L1_value);
		parallelAdaptor_C1.setComponent(wdfComponent::C, C1_value);
		seriesTerminatedAdaptor_L2.setComponent(wdfComponent::L, L2_value);

		// --- connect adapters
		WdfAdaptorBase::connectAdaptors(&seriesAdaptor_L1, &parallelAdaptor_C1);
		WdfAdaptorBase::connectAdaptors(&parallelAdaptor_C1, &seriesTerminatedAdaptor_L2);

		// --- set source resistance
		seriesAdaptor_L1.setSourceResistance(600.0); // --- Rs = 600

		// --- set terminal resistance
		seriesTerminatedAdaptor_L2.setTerminalResistance(600.0); // --- Rload = 600
	}

	void calculateNewComponentValues(double _fc)
	{
		// frequency warping
		double arg = (kPi * _fc) / sampleRate;
		_fc = _fc * (tan(arg) / arg);

		// frequency scaling:
		// calculate new values for L1, L2, C1

		double L1_newValue = L1_value / _fc;
		double L2_newValue = L2_value / _fc;

		seriesAdaptor_L1.setComponentValue(L1_newValue);
		parallelAdaptor_C1.setComponentValue(C1_value / _fc);
		seriesTerminatedAdaptor_L2.setComponentValue(L2_newValue);

		// if not called, the values are not updated inside the components
		// reset() only sets sampleRate after createWDF()
		// and the cutoff frequency will not change
		seriesAdaptor_L1.initializeAdaptorChain();
	}

protected:
	// --- three adapters
	WdfSeriesAdaptor seriesAdaptor_L1;			///< adaptor for L1
	WdfParallelAdaptor parallelAdaptor_C1;		///< adaptor for C1
	WdfSeriesTerminatedAdaptor seriesTerminatedAdaptor_L2;	///< adaptor for L2

	// --- component base values fc = 1Hz
	double L1_value = 95.49;		// 95.5 mH
	double C1_value = 0.5305e-3;	// 0.53 uF
	double L2_value = 95.49;		// 95.5 mH

	// if not set here, no audio output on startup
	double sampleRate = 44100.0;
};