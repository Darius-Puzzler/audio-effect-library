/**
\class WdfResistor
\ingroup WDF-Objects
\brief
The WdfResistor object implements the reflection coefficient and signal flow through
a WDF simulated resistor.

\author Will Pirkle http://www.willpirkle.com
\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/

#pragma once

#include "IComponentAdaptor.h"

namespace fxobjects {

	class WdfResistor : public IComponentAdaptor
	{
	public:
		WdfResistor(double _componentValue) { componentValue = _componentValue; }
		WdfResistor() { }
		virtual ~WdfResistor() {}
	
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
	
		/** get the component value */
		virtual double getComponentValue() { return componentValue; }
	
		/** set the component value */
		virtual void setComponentValue(double _componentValue)
		{
			componentValue = _componentValue;
			updateComponentResistance();
		}
	
		/** change the resistance of component */
		virtual void updateComponentResistance() { componentResistance = componentValue; }
	
		/** reset the component; clear registers */
		virtual void reset(double _sampleRate) { setSampleRate(_sampleRate);  zRegister = 0.0; }
	
		/** set input value into component; NOTE: resistor is dead-end energy sink so this function does nothing */
		virtual void setInput(double in) {}
	
		/** get output value; NOTE: a WDF resistor produces no reflected output */
		virtual double getOutput() { return 0.0; }
	
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
		double zRegister = 0.0;			///< storage register (not used with resistor)
		double componentValue = 0.0;	///< component value in electronic form (ohm, farad, henry)
		double componentResistance = 0.0;///< simulated resistance
		double sampleRate = 0.0;		///< sample rate
	};
} // namespace fxobjects
