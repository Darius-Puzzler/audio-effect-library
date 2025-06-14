/**
\class IAudioSignalGenerator
\ingroup Interfaces
\brief
Use this interface for objects that render an output without an input, such as oscillators. May also be used for envelope generators whose input is a note-on or other switchable event.

Outpput I/F:
- Use SignalGenData structure for output.

\author Will Pirkle http://www.willpirkle.com
\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/

#pragma once
#include "EnumsAndStructs.h"

namespace fxobjects
{
    class IAudioSignalGenerator
    {
    public:
        // --- pure virtual, derived classes must implement or will not compile
        //     also means this is a pure abstract base class and is incomplete,
        //     so it can only be used as a base class
        //
        /** Sample rate may or may not be required, but usually is */
        virtual bool reset(double _sampleRate) = 0;
    
        /** render the generator output */
        virtual const SignalGenData renderAudioOutput() = 0;
    };
} // namespace fxobjects