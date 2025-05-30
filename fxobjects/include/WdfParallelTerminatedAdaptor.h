/**
\class WdfParallelTerminatedAdaptor
\ingroup WDF-Objects
\brief
The WdfParallelTerminatedAdaptor object implements the parallel  terminated (non-reflection-free) adaptor

\author Will Pirkle http://www.willpirkle.com
\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/

#pragma once
#include "WdfAdaptorBase.h"

namespace fxobjects {

    class WdfParallelTerminatedAdaptor : public WdfAdaptorBase
    {
    public:
        WdfParallelTerminatedAdaptor() {}
        virtual ~WdfParallelTerminatedAdaptor() {}
    
        /** get the resistance at port 2;  R2 = 1.0/(sum of admittances) */
        virtual double getR2()
        {
            double componentConductance = 0.0;
            if (getPort3_CompAdaptor())
                componentConductance = getPort3_CompAdaptor()->getComponentConductance();
    
            // --- 1 / (sum of admittances)
            R2 = 1.0 / ((1.0 / R1) + componentConductance);
            return R2;
        }
    
        /** initialize adaptor with input resistance */
        virtual void initialize(double _R1)
        {
            // --- save R1
            R1 = _R1;
    
            double G1 = 1.0 / R1;
            if (terminalResistance <= 0.0)
                terminalResistance = 1e-15;
    
            double G2 = 1.0 / terminalResistance;
            double componentConductance = 0.0;
            if (getPort3_CompAdaptor())
                componentConductance = getPort3_CompAdaptor()->getComponentConductance();
    
            A1 = 2.0*G1 / (G1 + componentConductance + G2);
            A3 = openTerminalResistance ? 0.0 : 2.0*G2 / (G1 + componentConductance + G2);
    
            // --- init downstream
            if (getPort2_CompAdaptor())
                getPort2_CompAdaptor()->initialize(getR2());
    
            // --- not used in this implementation but saving for extended use
            R3 = 1.0 / componentConductance;
        }
    
        /** push audio input sample into incident wave input*/
        virtual void setInput1(double _in1)
        {
            // --- save
            in1 = _in1;
    
            N2 = 0.0;
            if (getPort3_CompAdaptor())
                N2 = getPort3_CompAdaptor()->getOutput();
    
            // --- form N1
            N1 = -A1*(-in1 + N2) + N2 - A3*N2;
    
            // --- form output1
            out1 = -in1 + N2 + N1;
    
            // --- deliver upstream to input2
            if (getPort1_CompAdaptor())
                getPort1_CompAdaptor()->setInput2(out1);
    
            // --- calc out2 y(n)
            out2 = N2 + N1;
    
            // --- set component state
            if (getPort3_CompAdaptor())
                getPort3_CompAdaptor()->setInput(N1);
        }
    
        /** push audio input sample into reflected wave input; this is a dead end for terminated adaptorsthis is a dead end for terminated adaptors  */
        virtual void setInput2(double _in2){ in2 = _in2;}
    
        /** set input 3 always connects to component */
        virtual void setInput3(double _in3) { }
    
        /** get OUT1 = reflected output pin on Port 1 */
        virtual double getOutput1() { return out1; }
    
        /** get OUT2 = incident (normal) output pin on Port 2 */
        virtual double getOutput2() { return out2; }
    
        /** get OUT3 always connects to component */
        virtual double getOutput3() { return out3; }
    
    private:
        double N1 = 0.0;	///< node 1 value, internal use only
        double N2 = 0.0;	///< node 2 value, internal use only
        double A1 = 0.0;	///< A1 coefficient value
        double A3 = 0.0;	///< A3 coefficient value
    };
} // namespace fxobjects