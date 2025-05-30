/**
\class WdfAdaptorBase
\ingroup WDF-Objects
\brief
The WdfAdaptorBase object acts as the base class for all WDF Adaptors; the static members allow
for simplified connection of components. See the FX book for more details.

\author Will Pirkle http://www.willpirkle.com
\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/

#pragma once
#include "IComponentAdaptor.h"
#include "WdfEnumsStructs.h"
#include "WdfCapacitor.h"
#include "WdfInductor.h"
#include "WdfResistor.h"
#include "WdfSeriesLC.h"
#include "WdfSeriesRC.h"
#include "WdfSeriesRL.h"
#include "WdfParallelLC.h"
#include "WdfParallelRC.h"
#include "WdfParallelRL.h"

namespace fxobjects {

    class WdfAdaptorBase : public IComponentAdaptor
    {
    public:
        WdfAdaptorBase() {}
        virtual ~WdfAdaptorBase() {}
    
        /** set the termainal (load) resistance for terminating adaptors */
        void setTerminalResistance(double _terminalResistance) { terminalResistance = _terminalResistance; }
    
        /** set the termainal (load) resistance as open circuit for terminating adaptors */
        void setOpenTerminalResistance(bool _openTerminalResistance = true)
        {
            // --- flag overrides value
            openTerminalResistance = _openTerminalResistance;
            terminalResistance = 1.0e+34; // avoid /0.0
        }
    
        /** set the input (source) resistance for an input adaptor */
        void setSourceResistance(double _sourceResistance) { sourceResistance = _sourceResistance; }
    
        /** set the component or connected adaptor at port 1; functions is generic and allows extending the functionality of the WDF Library */
        void setPort1_CompAdaptor(IComponentAdaptor* _port1CompAdaptor) { port1CompAdaptor = _port1CompAdaptor; }
    
        /** set the component or connected adaptor at port 2; functions is generic and allows extending the functionality of the WDF Library */
        void setPort2_CompAdaptor(IComponentAdaptor* _port2CompAdaptor) { port2CompAdaptor = _port2CompAdaptor; }
    
        /** set the component or connected adaptor at port 3; functions is generic and allows extending the functionality of the WDF Library */
        void setPort3_CompAdaptor(IComponentAdaptor* _port3CompAdaptor) { port3CompAdaptor = _port3CompAdaptor; }
    
        /** reset the connected component */
        virtual void reset(double _sampleRate)
        {
            if (wdfComponent)
                wdfComponent->reset(_sampleRate);
        }
    
        /** creates a new WDF component and connects it to Port 3 */
        void setComponent(wdfComponent componentType, double value1 = 0.0, double value2 = 0.0)
        {
            // --- decode and set
            if (componentType == wdfComponent::R)
            {
                wdfComponent = new WdfResistor;
                wdfComponent->setComponentValue(value1);
                port3CompAdaptor = wdfComponent;
            }
            else if (componentType == wdfComponent::L)
            {
                wdfComponent = new WdfInductor;
                wdfComponent->setComponentValue(value1);
                port3CompAdaptor = wdfComponent;
            }
            else if (componentType == wdfComponent::C)
            {
                wdfComponent = new WdfCapacitor;
                wdfComponent->setComponentValue(value1);
                port3CompAdaptor = wdfComponent;
            }
            else if (componentType == wdfComponent::seriesLC)
            {
                wdfComponent = new WdfSeriesLC;
                wdfComponent->setComponentValue_LC(value1, value2);
                port3CompAdaptor = wdfComponent;
            }
            else if (componentType == wdfComponent::parallelLC)
            {
                wdfComponent = new WdfParallelLC;
                wdfComponent->setComponentValue_LC(value1, value2);
                port3CompAdaptor = wdfComponent;
            }
            else if (componentType == wdfComponent::seriesRL)
            {
                wdfComponent = new WdfSeriesRL;
                wdfComponent->setComponentValue_RL(value1, value2);
                port3CompAdaptor = wdfComponent;
            }
            else if (componentType == wdfComponent::parallelRL)
            {
                wdfComponent = new WdfParallelRL;
                wdfComponent->setComponentValue_RL(value1, value2);
                port3CompAdaptor = wdfComponent;
            }
            else if (componentType == wdfComponent::seriesRC)
            {
                wdfComponent = new WdfSeriesRC;
                wdfComponent->setComponentValue_RC(value1, value2);
                port3CompAdaptor = wdfComponent;
            }
            else if (componentType == wdfComponent::parallelRC)
            {
                wdfComponent = new WdfParallelRC;
                wdfComponent->setComponentValue_RC(value1, value2);
                port3CompAdaptor = wdfComponent;
            }
        }
    
        /** connect two adapters together upstreamAdaptor --> downstreamAdaptor */
        static void connectAdaptors(WdfAdaptorBase* upstreamAdaptor, WdfAdaptorBase* downstreamAdaptor)
        {
            upstreamAdaptor->setPort2_CompAdaptor(downstreamAdaptor);
            downstreamAdaptor->setPort1_CompAdaptor(upstreamAdaptor);
        }
    
        /** initialize the chain of adaptors from upstreamAdaptor --> downstreamAdaptor */
        virtual void initializeAdaptorChain()
        {
            initialize(sourceResistance);
        }
    
        /** set value of single-component adaptor */
        virtual void setComponentValue(double _componentValue)
        {
            if (wdfComponent)
                wdfComponent->setComponentValue(_componentValue);
        }
    
        /** set LC value of mjulti-component adaptor */
        virtual void setComponentValue_LC(double componentValue_L, double componentValue_C)
        {
            if (wdfComponent)
                wdfComponent->setComponentValue_LC(componentValue_L, componentValue_C);
        }
    
        /** set RL value of mjulti-component adaptor */
        virtual void setComponentValue_RL(double componentValue_R, double componentValue_L)
        {
            if (wdfComponent)
                wdfComponent->setComponentValue_RL(componentValue_R, componentValue_L);
        }
    
        /** set RC value of mjulti-component adaptor */
        virtual void setComponentValue_RC(double componentValue_R, double componentValue_C)
        {
            if (wdfComponent)
                wdfComponent->setComponentValue_RC(componentValue_R, componentValue_C);
        }
    
        /** get adaptor connected at port 1: for extended functionality; not used in WDF ladder filter library */
        IComponentAdaptor* getPort1_CompAdaptor() { return port1CompAdaptor; }
    
        /** get adaptor connected at port 2: for extended functionality; not used in WDF ladder filter library */
        IComponentAdaptor* getPort2_CompAdaptor() { return port2CompAdaptor; }
    
        /** get adaptor connected at port 3: for extended functionality; not used in WDF ladder filter library */
        IComponentAdaptor* getPort3_CompAdaptor() { return port3CompAdaptor; }
    
    protected:
        // --- can in theory connect any port to a component OR adaptor;
        //     though this library is setup with a convention R3 = component
        IComponentAdaptor* port1CompAdaptor = nullptr;	///< componant or adaptor connected to port 1
        IComponentAdaptor* port2CompAdaptor = nullptr;	///< componant or adaptor connected to port 2
        IComponentAdaptor* port3CompAdaptor = nullptr;	///< componant or adaptor connected to port 3
        IComponentAdaptor* wdfComponent = nullptr;		///< WDF componant connected to port 3 (default operation)
    
        // --- These hold the input (R1), component (R3) and output (R2) resistances
        double R1 = 0.0; ///< input port resistance
        double R2 = 0.0; ///< output port resistance
        double R3 = 0.0; ///< component resistance
    
        // --- these are input variables that are stored;
        //     not used in this implementation but may be required for extended versions
        double in1 = 0.0;	///< stored port 1 input;  not used in this implementation but may be required for extended versions
        double in2 = 0.0;	///< stored port 2 input;  not used in this implementation but may be required for extended versions
        double in3 = 0.0;	///< stored port 3 input;  not used in this implementation but may be required for extended versions
    
        // --- these are output variables that are stored;
        //     currently out2 is the only one used as it is y(n) for this library
        //     out1 and out2 are stored; not used in this implementation but may be required for extended versions
        double out1 = 0.0;	///< stored port 1 output; not used in this implementation but may be required for extended versions
        double out2 = 0.0;	///< stored port 2 output; it is y(n) for this library
        double out3 = 0.0;	///< stored port 3 output; not used in this implementation but may be required for extended versions
    
        // --- terminal impedance
        double terminalResistance = 600.0; ///< value of terminal (load) resistance
        bool openTerminalResistance = false; ///< flag for open circuit load
    
        // --- source impedance, OK for this to be set to 0.0 for Rs = 0
        double sourceResistance = 600.0; ///< source impedance; OK for this to be set to 0.0 for Rs = 0
    };
} // namespace fxobjects
