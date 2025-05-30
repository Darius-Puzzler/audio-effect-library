/**
\class WdfSeriesAdaptor
\ingroup WDF-Objects
\brief
The WdfSeriesAdaptor object implements the series reflection-free (non-terminated) adaptor

\author Will Pirkle http://www.willpirkle.com
\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/

#pragma once
#include "WdfAdaptorBase.h"

namespace fxobjects {

    class WdfSeriesAdaptor : public WdfAdaptorBase
    {
    public:
        WdfSeriesAdaptor() {}
        virtual ~WdfSeriesAdaptor() {}
    
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
            // --- R1 is source resistance for this adaptor
            R1 = _R1;
    
            double componentResistance = 0.0;
            if (getPort3_CompAdaptor())
                componentResistance = getPort3_CompAdaptor()->getComponentResistance();
    
            // --- calculate B coeff
            B = R1 / (R1 + componentResistance);
    
            // --- init downstream adaptor
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
    
            // --- read component value
            N2 = 0.0;
            if (getPort3_CompAdaptor())
                N2 = getPort3_CompAdaptor()->getOutput();
    
            // --- form output
            out2 = -(in1 + N2);
    
            // --- deliver downstream
            if (getPort2_CompAdaptor())
                getPort2_CompAdaptor()->setInput1(out2);
        }
    
        /** push audio input sample into reflected wave input */
        virtual void setInput2(double _in2)
        {
            // --- save
            in2 = _in2;
    
            // --- calc N1
            N1 = -(in1 - B*(in1 + N2 + in2) + in2);
    
            // --- calc out1
            out1 = in1 - B*(N2 + in2);
    
            // --- deliver upstream
            if (getPort1_CompAdaptor())
                getPort1_CompAdaptor()->setInput2(out1);
    
            // --- set component state
            if (getPort3_CompAdaptor())
                getPort3_CompAdaptor()->setInput(N1);
        }
    
        /** set input 3 always connects to component */
        virtual void setInput3(double _in3){ }
    
        /** get OUT1 = reflected output pin on Port 1 */
        virtual double getOutput1() { return out1; }
    
        /** get OUT2 = incident (normal) output pin on Port 2 */
        virtual double getOutput2() { return out2; }
    
        /** get OUT3 always connects to component */
        virtual double getOutput3() { return out3; }
    
    private:
        double N1 = 0.0;	///< node 1 value, internal use only
        double N2 = 0.0;	///< node 2 value, internal use only
        double B = 0.0;		///< B coefficient value
    };
} // namespace fxobjects
