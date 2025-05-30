/**
\class WdfParallelLC
\ingroup WDF-Objects
\brief
The WdfParallelLC object implements the reflection coefficient and signal flow through
a WDF simulated parallel LC pair.

\author Will Pirkle http://www.willpirkle.com
\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
SEE: p143 "Design of Wave Digital Filters" Psenicka, Ugalde, Romero M.
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/

#pragma once
#include "IComponentAdaptor.h"

namespace fxobjects {

	class WdfParallelLC : public IComponentAdaptor
	{
	public:
		WdfParallelLC() {}
		WdfParallelLC(double _componentValue_L, double _componentValue_C)
		{
			componentValue_L = _componentValue_L;
			componentValue_C = _componentValue_C;
		}
		virtual ~WdfParallelLC() {}
	
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
			RL = 2.0*componentValue_L*sampleRate;
			RC = 1.0 / (2.0*componentValue_C*sampleRate);
			componentResistance = (RC + 1.0 / RL);
		}
	
		/** set both LC components at once */
		virtual void setComponentValue_LC(double _componentValue_L, double _componentValue_C)
		{
			componentValue_L = _componentValue_L;
			componentValue_C = _componentValue_C;
			updateComponentResistance();
		}
	
		/** set L component */
		virtual void setComponentValue_L(double _componentValue_L)
		{
			componentValue_L = _componentValue_L;
			updateComponentResistance();
		}
	
		/** set C component */
		virtual void setComponentValue_C(double _componentValue_C)
		{
			componentValue_C = _componentValue_C;
			updateComponentResistance();
		}
	
		/** get L component value */
		virtual double getComponentValue_L() { return componentValue_L; }
	
		/** get C component value */
		virtual double getComponentValue_C() { return componentValue_C; }
	
		/** reset the component; clear registers */
		virtual void reset(double _sampleRate) { setSampleRate(_sampleRate); zRegister_L = 0.0; zRegister_C = 0.0; }
	
		/** set input value into component; NOTE: K is calculated here */
		virtual void setInput(double in)
		{
			double YL = 1.0 / RL;
			double K = (YL*RC - 1.0) / (YL*RC + 1.0);
			double N1 = K*(in - zRegister_L);
			zRegister_L = N1 + zRegister_C;
			zRegister_C = in;
		}
	
		/** get output value; NOTE: output is located in -zReg_L */
		virtual double getOutput(){ return -zRegister_L; }
	
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
		double zRegister_L = 0.0; ///< storage register for L
		double zRegister_C = 0.0; ///< storage register for C
	
		double componentValue_L = 0.0; ///< component value L
		double componentValue_C = 0.0; ///< component value C
	
		double RL = 0.0; ///< RL value
		double RC = 0.0; ///< RC value
		double componentResistance = 0.0; ///< equivalent resistance of pair of components
		double sampleRate = 0.0; ///< sample rate
	};
} // namespace fxobjects