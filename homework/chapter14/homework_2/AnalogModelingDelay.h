/*
*   Implement the analog modeling delay in Figure 14.15.
* 
*   Analog delays using the Bucket Brigade Delay ICs (BBDs) are popular for their organic sounds.
    Contrary to the name, analog delays include a sample-and-hold along with an anti-aliasing low-pass
    filter, and they sample the input, chopping it into pieces. The “analog” part is that instead of digital
    numeric values, analog charges are stored and delayed using a system of switches and capacitors,
    which pass the analog voltage slices along in the same way as a bucket brigade passes buckets of water
    for fire-fighting. Most of these analog devices also include a compander (complimentary compression
    and expansion) around the BBD since these ICs are limited in dynamic range: the signal is compressed
    going into the BBD, and then expanded coming out (note that this is true upward expansion).
    
    The filtering, companding and generally noisy operation of the BBD ICs contributes to this organic sound.
    The most obvious audible effect is that as the echoes are fed back into the BBD, they come back out
    slightly noisier, and with slightly less high frequency content. To mimic the loss of high frequencies
    with each lap around the feedback path, we place a low-pass filter in it, either before or after the
    feedback attenuation coefficient. Make sure you do not use a resonant low-pass filter or one with any
    gain above 0.0 dB as this will cause the feedback loop to become unstable. Figure 14.15 shows what
    a basic analog modeling delay algorithm might look like. Improvements would include the addition
    of a compander around the delay line and an injection of noise (perhaps EQ’d to model a specific tape
    noise) into the delay line.
*/

#pragma once
#include "include/AudioDelay.h"
#include "include/AudioFilter.h"

#define CUTOFF 3000.0; // fc in Hz

namespace fxobjects
{
    class AnalogModelingDelay : public IAudioSignalProcessor
    {
    public:
        AnalogModelingDelay() {}		/* C-TOR */
        ~AnalogModelingDelay() {}	/* D-TOR */

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

            // reset the LP filter

            mAudioFilter.reset(_sampleRate);
            mAudioFilterParameters.algorithm = filterAlgorithm::kLPF1;
            mAudioFilterParameters.fc = CUTOFF;
            mAudioFilter.setParameters(mAudioFilterParameters); // this calls also calculateFilterCoeffs();

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
            double output = dryMix * xn + wetMix * yn;

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
            double outputL = dryMix * xnL + wetMix * ynL;

            // --- form mixture out = dry*xn + wet*yn
            double outputR = dryMix * xnR + wetMix * ynR;

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
                double newDelayInSamples_L = parameters.leftDelay_mSec * (samplesPerMSec);
                double newDelayInSamples_R = parameters.rightDelay_mSec * (samplesPerMSec);

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
                double newDelayInSamples = parameters.leftDelay_mSec * (samplesPerMSec);

                // --- new delay time with fraction
                delayInSamples_L = newDelayInSamples;
                delayInSamples_R = delayInSamples_L * delayRatio;
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
            bufferLength = (unsigned int)(bufferLength_mSec * (samplesPerMSec)) + 1; // +1 for fractional part

            // --- create new buffer
            delayBuffer_L.createCircularBuffer(bufferLength);
            delayBuffer_R.createCircularBuffer(bufferLength);
        }

        // process a block and call the Low Pass Filter processblock after
        // only this processBlock is used to process the delay
        void processBlock(iplug::sample** inputs, iplug::sample** outputs, int numChannels, int nFrames)
        {
            // --- make sure we have input and outputs
            if (numChannels == 0)
                return;

            // --- make sure we support this delay algorithm
            if (parameters.algorithm != delayAlgorithm::kNormal &&
                parameters.algorithm != delayAlgorithm::kPingPong)
                return;

            // --- if only one output channel, revert to mono operation
            if (numChannels == 1)
            {
                for (int s = 0; s < nFrames; s++)
                {
                    // --- read delay
                    double yn = delayBuffer_L.readBuffer(delayInSamples_L);

                    // analog modeling L
                    double yMod = mAudioFilter.processAudioSample(yn);

                    // --- create input for delay buffer
                    double dn = inputs[0][s] + (parameters.leftFeedback_Pct / 100.0) * yMod; // use left feedback for mono processing

                    // --- write to delay buffer
                    delayBuffer_L.writeBuffer(dn);

                    // --- form mixture out = dry*xn + wet*yn
                    outputs[0][s] = dryMix * inputs[0][s] + wetMix * yn;
                }
                return;
            }

            // --- if we get here we know we have 2 output channels
            //
            // --- 
            for (int s = 0; s < nFrames; s++)
            {
                double xnL = inputs[0][s];

                // --- RIGHT channel (duplicate left input if mono-in)
                double xnR = numChannels > 1 ? inputs[1][s] : xnL;

                // --- read delay LEFT
                double ynL = delayBuffer_L.readBuffer(delayInSamples_L);

                // --- read delay RIGHT
                double ynR = delayBuffer_R.readBuffer(delayInSamples_R);
                
                // analog modeling L
                double yModL = mAudioFilter.processAudioSample(ynL);
                
                // analog modeling R
                double yModR = mAudioFilter.processAudioSample(ynR);

                // --- create input for delay buffer with LEFT channel info
                double dnL = xnL + (parameters.leftFeedback_Pct / 100.0) * yModL;

                // --- create input for delay buffer with RIGHT channel info
                double dnR = xnR + (parameters.rightFeedback_Pct / 100.0) * yModR;

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
                double outputL = dryMix * xnL + wetMix * ynL;

                // --- form mixture out = dry*xn + wet*yn
                double outputR = dryMix * xnR + wetMix * ynR;

                // --- set left channel
                outputs[0][s] = (iplug::sample)outputL;

                // --- set right channel
                outputs[1][s] = (iplug::sample)outputR;
            }
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

        // (Low Pass) Filter for Analog Modeling
        AudioFilter mAudioFilter;
        AudioFilterParameters mAudioFilterParameters;
    };
} // namespace fxobjects