/**
\class WdfSeriesRC
\ingroup WDF-Objects
\brief
The WdfSeriesRC object implements the reflection coefficient and signal flow through
a WDF simulated series RC pair.

\author Will Pirkle http://www.willpirkle.com
\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
SEE: p143 "Design of Wave Digital Filters" Psenicka, Ugalde, Romero M.
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/

#pragma once
#include "IComponentAdaptor.h"

namespace fxobjects {

	class WdfSeriesRC : public IComponentAdaptor
	{
	public:
		WdfSeriesRC() {}
		WdfSeriesRC(double _componentValue_R, double _componentValue_C)
		{
			componentValue_C = _componentValue_C;
			componentValue_R = _componentValue_R;
		}
		virtual ~WdfSeriesRC() {}
	
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
			RC = 1.0 / (2.0*componentValue_C*sampleRate);
			componentResistance = RR + RC;
			K = RR / componentResistance;
		}
	
		/** set both RC components at once */
		virtual void setComponentValue_RC(double _componentValue_R, double _componentValue_C)
		{
			componentValue_R = _componentValue_R;
			componentValue_C = _componentValue_C;
			updateComponentResistance();
		}
	
		/** set R component */
		virtual void setComponentValue_R(double _componentValue_R)
		{
			componentValue_R = _componentValue_R;
			updateComponentResistance();
		}
	
		/** set C component */
		virtual void setComponentValue_C(double _componentValue_C)
		{
			componentValue_C = _componentValue_C;
			updateComponentResistance();
		}
	
		/** get R component value */
		virtual double getComponentValue_R() { return componentValue_R; }
	
		/** get C component value */
		virtual double getComponentValue_C() { return componentValue_C; }
	
		/** reset the component; clear registers */
		virtual void reset(double _sampleRate) { setSampleRate(_sampleRate); zRegister_L = 0.0; zRegister_C = 0.0; }
	
		/** set input value into component */
		virtual void setInput(double in){ zRegister_L = in; }
	
		/** get output value; NOTE: see FX book for details */
		virtual double getOutput()
		{
			double NL = zRegister_L;
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
		double zRegister_L = 0.0; ///< storage register for L
		double zRegister_C = 0.0; ///< storage register for C
		double K = 0.0;
	
		double componentValue_R = 0.0;///< component value R
		double componentValue_C = 0.0;///< component value C
	
		double RL = 0.0;	///< RL value
		double RC = 0.0;	///< RC value
		double RR = 0.0;	///< RR value
	
		double componentResistance = 0.0; ///< equivalent resistance of pair of components
		double sampleRate = 0.0; ///< sample rate
	};
} // namespace fxobjects