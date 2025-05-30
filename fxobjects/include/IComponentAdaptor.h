/**
\class IComponentAdaptor
\ingroup Interfaces
\brief
Use this interface for objects in the WDF Ladder Filter library; see book for more information.

\author Will Pirkle http://www.willpirkle.com
\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/

#pragma once

namespace fxobjects {

	class IComponentAdaptor
	{
	public:
		/** initialize with source resistor R1 */
		virtual void initialize(double _R1) {}
	
		/** initialize all downstream adaptors in the chain */
		virtual void initializeAdaptorChain() {}
	
		/** set input value into component port  */
		virtual void setInput(double _in) {}
	
		/** get output value from component port  */
		virtual double getOutput() { return 0.0; }
	
		// --- for adaptors
		/** ADAPTOR: set input port 1  */
		virtual void setInput1(double _in1) = 0;
	
		/** ADAPTOR: set input port 2  */
		virtual void setInput2(double _in2) = 0;
	
		/** ADAPTOR: set input port 3 */
		virtual void setInput3(double _in3) = 0;
	
		/** ADAPTOR: get output port 1 value */
		virtual double getOutput1() = 0;
	
		/** ADAPTOR: get output port 2 value */
		virtual double getOutput2() = 0;
	
		/** ADAPTOR: get output port 3 value */
		virtual double getOutput3() = 0;
	
		/** reset the object with new sample rate */
		virtual void reset(double _sampleRate) {}
	
		/** get the commponent resistance from the attached object at Port3 */
		virtual double getComponentResistance() { return 0.0; }
	
		/** get the commponent conductance from the attached object at Port3 */
		virtual double getComponentConductance() { return 0.0; }
	
		/** update the commponent resistance at Port3 */
		virtual void updateComponentResistance() {}
	
		/** set an individual component value (may be R, L, or C */
		virtual void setComponentValue(double _componentValue) { }
	
		/** set LC combined values */
		virtual void setComponentValue_LC(double componentValue_L, double componentValue_C) { }
	
		/** set RL combined values */
		virtual void setComponentValue_RL(double componentValue_R, double componentValue_L) { }
	
		/** set RC combined values */
		virtual void setComponentValue_RC(double componentValue_R, double componentValue_C) { }
	
		/** get a component value */
		virtual double getComponentValue() { return 0.0; }
	};
} // namespace fxobjects