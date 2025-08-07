/**
\class AudioDelay
\ingroup FX-Objects
\brief
The AudioDelay object implements a stereo audio delay with multiple delay algorithms.

Audio I/O:
- Processes mono input to mono output OR stereo output.

Control I/F:
- Use AudioDelayParameters structure to get/set object params.

\author Will Pirkle http://www.willpirkle.com
\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/

#pragma once
#include "IAudioSignalProcessor.h"
#include "CircularBuffer.h"
#include "EnumsAndStructs.h"
#include "helperfunctions.h"

namespace fxobjects
{
    class AudioDelay : public IAudioSignalProcessor
    {
    public:
        AudioDelay() {}		/* C-TOR */
        ~AudioDelay() {}	/* D-TOR */
    
    public:
        /** reset members to initialized state */
        virtual bool reset(double _sampleRate)
        {
            // --- if sample rate did not change
            if (sampleRate == _sampleRate)
            {
                // --- just flush buffer and return
                delayBuffer_L.flushBuffer();
                delayBuffer_R.flushBuffer();
                return true;
            }
    
            // --- create new buffer, will store sample rate and length(mSec)
            createDelayBuffers(_sampleRate, bufferLength_mSec);
    
            return true;
        }
    
        /** process MONO audio delay */
        /**
        \param xn input
        \return the processed sample
        */
        virtual double processAudioSample(double xn)
        {
            // --- read delay
            double yn = delayBuffer_L.readBuffer(delayInSamples_L);
    
            // --- create input for delay buffer
            double dn = xn + (parameters.leftFeedback_Pct / 100.0) * yn; // use left feedback for mono processing
    
            // --- write to delay buffer
            delayBuffer_L.writeBuffer(dn);
    
            // --- form mixture out = dry*xn + wet*yn
            double output = dryMix*xn + wetMix*yn;
    
            return output;
        }
    
        /** return true: this object can also process frames */
        virtual bool canProcessAudioFrame() { return true; }
    
        /** process STEREO audio delay in frames */
        virtual bool processAudioFrame(const float* inputFrame,		/* ptr to one frame of data: pInputFrame[0] = left, pInputFrame[1] = right, etc...*/
            float* outputFrame,
            uint32_t inputChannels,
            uint32_t outputChannels)
        {
            // --- make sure we have input and outputs
            if (inputChannels == 0 || outputChannels == 0)
                return false;
    
            // --- make sure we support this delay algorithm
            if (parameters.algorithm != delayAlgorithm::kNormal &&
                parameters.algorithm != delayAlgorithm::kPingPong)
                return false;
    
            // --- if only one output channel, revert to mono operation
            if (outputChannels == 1)
            {
                // --- process left channel only
                outputFrame[0] = (float)processAudioSample(inputFrame[0]);
                return true;
            }
    
            // --- if we get here we know we have 2 output channels
            //
            // --- pick up inputs
            //
            // --- LEFT channel
            double xnL = inputFrame[0];
    
            // --- RIGHT channel (duplicate left input if mono-in)
            double xnR = inputChannels > 1 ? inputFrame[1] : xnL;
    
            // --- read delay LEFT
            double ynL = delayBuffer_L.readBuffer(delayInSamples_L);
    
            // --- read delay RIGHT
            double ynR = delayBuffer_R.readBuffer(delayInSamples_R);
    
            // --- create input for delay buffer with LEFT channel info
            double dnL = xnL + (parameters.leftFeedback_Pct / 100.0) * ynL;
    
            // --- create input for delay buffer with RIGHT channel info
            double dnR = xnR + (parameters.rightFeedback_Pct / 100.0) * ynR;
    
            // --- decode
            if (parameters.algorithm == delayAlgorithm::kNormal)
            {
                // --- write to LEFT delay buffer with LEFT channel info
                delayBuffer_L.writeBuffer(dnL);
    
                // --- write to RIGHT delay buffer with RIGHT channel info
                delayBuffer_R.writeBuffer(dnR);
            }
            else if (parameters.algorithm == delayAlgorithm::kPingPong)
            {
                // --- write to LEFT delay buffer with RIGHT channel info
                delayBuffer_L.writeBuffer(dnR);
    
                // --- write to RIGHT delay buffer with LEFT channel info
                delayBuffer_R.writeBuffer(dnL);
            }
    
            // --- form mixture out = dry*xn + wet*yn
            double outputL = dryMix*xnL + wetMix*ynL;
    
            // --- form mixture out = dry*xn + wet*yn
            double outputR = dryMix*xnR + wetMix*ynR;
    
            // --- set left channel
            outputFrame[0] = (float)outputL;
    
            // --- set right channel
            outputFrame[1] = (float)outputR;
    
            return true;
        }
    
        /** get parameters: note use of custom structure for passing param data */
        /**
        \return AudioDelayParameters custom data structure
        */
        AudioDelayParameters getParameters() { return parameters; }
    
        /** set parameters: note use of custom structure for passing param data */
        /**
        \param AudioDelayParameters custom data structure
        */
        void setParameters(AudioDelayParameters _parameters)
        {
            // --- check mix in dB for calc
            if (_parameters.dryLevel_dB != parameters.dryLevel_dB)
                dryMix = pow(10.0, _parameters.dryLevel_dB / 20.0);
            if (_parameters.wetLevel_dB != parameters.wetLevel_dB)
                wetMix = pow(10.0, _parameters.wetLevel_dB / 20.0);
    
            // --- save; rest of updates are cheap on CPU
            parameters = _parameters;
    
            // --- check update type first:
            if (parameters.updateType == delayUpdateType::kLeftAndRight)
            {
                // --- set left and right delay times
                // --- calculate total delay time in samples + fraction
                double newDelayInSamples_L = parameters.leftDelay_mSec*(samplesPerMSec);
                double newDelayInSamples_R = parameters.rightDelay_mSec*(samplesPerMSec);
    
                // --- new delay time with fraction
                delayInSamples_L = newDelayInSamples_L;
                delayInSamples_R = newDelayInSamples_R;
            }
            else if (parameters.updateType == delayUpdateType::kLeftPlusRatio)
            {
                // --- get and validate ratio
                double delayRatio = parameters.delayRatio_Pct / 100.0;
                boundValue(delayRatio, 0.0, 1.0);
    
                // --- calculate total delay time in samples + fraction
                double newDelayInSamples = parameters.leftDelay_mSec*(samplesPerMSec);
    
                // --- new delay time with fraction
                delayInSamples_L = newDelayInSamples;
                delayInSamples_R = delayInSamples_L*delayRatio;
            }
        }
    
        /** creation function */
        void createDelayBuffers(double _sampleRate, double _bufferLength_mSec)
        {
            // --- store for math
            bufferLength_mSec = _bufferLength_mSec;
            sampleRate = _sampleRate;
            samplesPerMSec = sampleRate / 1000.0;
    
            // --- total buffer length including fractional part
            bufferLength = (unsigned int)(bufferLength_mSec*(samplesPerMSec)) + 1; // +1 for fractional part
    
                                                                                   // --- create new buffer
            delayBuffer_L.createCircularBuffer(bufferLength);
            delayBuffer_R.createCircularBuffer(bufferLength);
        }
    
    private:
        AudioDelayParameters parameters; ///< object parameters
    
        double sampleRate = 0.0;		///< current sample rate
        double samplesPerMSec = 0.0;	///< samples per millisecond, for easy access calculation
        double delayInSamples_L = 0.0;	///< double includes fractional part
        double delayInSamples_R = 0.0;	///< double includes fractional part
        double bufferLength_mSec = 0.0;	///< buffer length in mSec
        unsigned int bufferLength = 0;	///< buffer length in samples
        double wetMix = 0.707; ///< wet output default = -3dB
        double dryMix = 0.707; ///< dry output default = -3dB
    
        // --- delay buffer of doubles
        CircularBuffer<double> delayBuffer_L;	///< LEFT delay buffer of doubles
        CircularBuffer<double> delayBuffer_R;	///< RIGHT delay buffer of doubles
    };
} // namespace fxobjects