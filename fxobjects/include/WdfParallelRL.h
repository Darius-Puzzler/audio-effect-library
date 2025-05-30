/**
\class WdfParallelRL
\ingroup WDF-Objects
\brief
The WdfParallelRL object implements the reflection coefficient and signal flow through
a WDF simulated parallel RL pair.

\author Will Pirkle http://www.willpirkle.com
\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/

#pragma once
#include "IComponentAdaptor.h"

namespace fxobjects {

	class WdfParallelRL : public IComponentAdaptor
	{
	public:
		WdfParallelRL() {}
		WdfParallelRL(double _componentValue_R, double _componentValue_L)
		{
			componentValue_L = _componentValue_L;
			componentValue_R = _componentValue_R;
		}
		virtual ~WdfParallelRL() {}
	
		/** set sample rate and update component */
		void setSampleRate(double _sampleRate)
		{
			sampleRate = _sampleRate;
			updateComponentResistance();
		}
	
		/** get component's value as a resistance */
		virtual double getComponentResistance() { return componentResistance; }
	
		/** get component's value as a conducatance (or admittance) */
		virtual double getComponentConductance() { return 1.0 / componentResistance; }
	
		/** change the resistance of component; see FX book for details */
		virtual void updateComponentResistance()
		{
			RR = componentValue_R;
			RL = 2.0*componentValue_L*sampleRate;
			componentResistance = 1.0 / ((1.0 / RR) + (1.0 / RL));
			K = componentResistance / RR;
		}
	
	
		/** set both RL components at once */
		virtual void setComponentValue_RL(double _componentValue_R, double _componentValue_L)
		{
			componentValue_L = _componentValue_L;
			componentValue_R = _componentValue_R;
			updateComponentResistance();
		}
	
		/** set L component */
		virtual void setComponentValue_L(double _componentValue_L)
		{
			componentValue_L = _componentValue_L;
			updateComponentResistance();
		}
	
		/** set R component */
		virtual void setComponentValue_R(double _componentValue_R)
		{
			componentValue_R = _componentValue_R;
			updateComponentResistance();
		}
	
		/** get L component value */
		virtual double getComponentValue_L() { return componentValue_L; }
	
		/** get R component value */
		virtual double getComponentValue_R() { return componentValue_R; }
	
		/** reset the component; clear registers */
		virtual void reset(double _sampleRate) { setSampleRate(_sampleRate); zRegister_L = 0.0; zRegister_C = 0.0; }
	
		/** set input value into component */
		virtual void setInput(double in){ zRegister_L = in; }
	
		/** get output value; NOTE: see FX book for details */
		virtual double getOutput()
		{
			double NL = -zRegister_L;
			double out = NL*(1.0 - K) + K*zRegister_C;
			zRegister_C = out;
			return out;
		}
	
		/** get output1 value; only one resistor output (not used) */
		virtual double getOutput1() { return  getOutput(); }
	
		/** get output2 value; only one resistor output (not used) */
		virtual double getOutput2() { return  getOutput(); }
	
		/** get output3 value; only one resistor output (not used) */
		virtual double getOutput3() { return  getOutput(); }
	
		/** set input1 value; not used for components */
		virtual void setInput1(double _in1) {}
	
		/** set input2 value; not used for components */
		virtual void setInput2(double _in2) {}
	
		/** set input3 value; not used for components */
		virtual void setInput3(double _in3) {}
	
	protected:
		double zRegister_L = 0.0;	///< storage register for L
		double zRegister_C = 0.0;	///< storage register for L
		double K = 0.0;				///< K value
	
		double componentValue_L = 0.0;	///< component value L
		double componentValue_R = 0.0;	///< component value R
	
		double RL = 0.0;	///< RL value
		double RC = 0.0;	///< RC value
		double RR = 0.0;	///< RR value
	
		double componentResistance = 0.0; ///< equivalent resistance of pair of components
		double sampleRate = 0.0; ///< sample rate
	};
} // namespace fxobjects