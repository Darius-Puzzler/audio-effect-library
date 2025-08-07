/**
\class LinearBuffer
\ingroup FX-Objects
\brief
The LinearBuffer object implements a linear buffer of type T. It allows easy wrapping of a smart pointer object.

\author Will Pirkle http://www.willpirkle.com
\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/

#pragma once

namespace fxobjects
{
    template <typename T>
    class LinearBuffer
    {
    public:
        LinearBuffer() {}	/* C-TOR */
        ~LinearBuffer() {}	/* D-TOR */
    
        /** flush buffer by resetting all values to 0.0 */
        void flushBuffer() { memset(&buffer[0], 0, bufferLength * sizeof(T)); }
    
        /** Create a buffer based on a target maximum in SAMPLES
        //	   do NOT call from realtime audio thread; do this prior to any processing */
        void createLinearBuffer(unsigned int _bufferLength)
        {
            // --- find nearest power of 2 for buffer, save it as bufferLength
            bufferLength = _bufferLength;
    
            // --- create new buffer
            buffer.reset(new T[bufferLength]);
    
            // --- flush buffer
            flushBuffer();
        }
    
        /** write a value into the buffer; this overwrites the previous oldest value in the buffer */
        void writeBuffer(unsigned int index, T input)
        {
            if (index >= bufferLength) return;
    
            // --- write and increment index counter
            buffer[index] = input;
        }
    
        /**  read an arbitrary location that is delayInSamples old */
        T readBuffer(unsigned int index)//, bool readBeforeWrite = true)
        {
            if (index >= bufferLength) return 0.0;
    
            // --- read it
            return buffer[index];
        }
    
    private:
        std::unique_ptr<T[]> buffer = nullptr;	///< smart pointer will auto-delete
        unsigned int bufferLength = 1024; ///< buffer length
    };
} // namespace fxobjects