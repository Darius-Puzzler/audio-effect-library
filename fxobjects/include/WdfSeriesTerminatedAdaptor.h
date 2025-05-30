/**
\class WdfSeriesTerminatedAdaptor
\ingroup WDF-Objects
\brief
The WdfSeriesTerminatedAdaptor object implements the series terminated (non-reflection-free) adaptor

\author Will Pirkle http://www.willpirkle.com
\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/
// --- Series terminated adaptor

#pragma once
#include "WdfAdaptorBase.h"

namespace fxobjects {
    
    class WdfSeriesTerminatedAdaptor : public WdfAdaptorBase
    {
    public:
        WdfSeriesTerminatedAdaptor() {}
        virtual ~WdfSeriesTerminatedAdaptor() {}
    
        /** get the resistance at port 2; R2 = R1 + component (series)*/
        virtual double getR2()
        {
            double componentResistance = 0.0;
            if (getPort3_CompAdaptor())
                componentResistance = getPort3_CompAdaptor()->getComponentResistance();
    
            R2 = R1 + componentResistance;
            return R2;
        }
    
        /** initialize adaptor with input resistance */
        virtual void initialize(double _R1)
        {
            // --- source impedance
            R1 = _R1;
    
            double componentResistance = 0.0;
            if (getPort3_CompAdaptor())
                componentResistance = getPort3_CompAdaptor()->getComponentResistance();
    
            B1 = (2.0*R1) / (R1 + componentResistance + terminalResistance);
            B3 = (2.0*terminalResistance) / (R1 + componentResistance + terminalResistance);
    
            // --- init downstream
            if (getPort2_CompAdaptor())
                getPort2_CompAdaptor()->initialize(getR2());
    
            // --- not used in this implementation but saving for extended use
            R3 = componentResistance;
        }
    
        /** push audio input sample into incident wave input*/
        virtual void setInput1(double _in1)
        {
            // --- save
            in1 = _in1;
    
            N2 = 0.0;
            if (getPort3_CompAdaptor())
                N2 = getPort3_CompAdaptor()->getOutput();
    
            double N3 = in1 + N2;
    
            // --- calc out2 y(n)
            out2 = -B3*N3;
    
            // --- form output1
            out1 = in1 - B1*N3;
    
            // --- form N1
            N1 = -(out1 + out2 + N3);
    
            // --- deliver upstream to input2
            if (getPort1_CompAdaptor())
                getPort1_CompAdaptor()->setInput2(out1);
    
            // --- set component state
            if (getPort3_CompAdaptor())
                getPort3_CompAdaptor()->setInput(N1);
        }
    
        /** push audio input sample into reflected wave input
            for terminated adaptor, this is dead end, just store it */
        virtual void setInput2(double _in2) { in2 = _in2;}
    
        /** set input 3 always connects to component */
        virtual void setInput3(double _in3) { in3 = _in3;}
    
        /** get OUT1 = reflected output pin on Port 1 */
        virtual double getOutput1() { return out1; }
    
        /** get OUT2 = incident (normal) output pin on Port 2 */
        virtual double getOutput2() { return out2; }
    
        /** get OUT3 always connects to component */
        virtual double getOutput3() { return out3; }
    
    private:
        double N1 = 0.0;	///< node 1 value, internal use only
        double N2 = 0.0;	///< node 2 value, internal use only
        double B1 = 0.0;	///< B1 coefficient value
        double B3 = 0.0;	///< B3 coefficient value
    };
} // namespace fxobjects