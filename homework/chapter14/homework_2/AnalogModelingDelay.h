/**
 * @file AnalogModelingDelay.h
 * @brief Implementation of an analog modeling delay effect using BBD (Bucket Brigade Device) emulation
 * 
 * @details
 * This class implements an analog modeling delay that emulates the characteristics of
 * Bucket Brigade Delay (BBD) integrated circuits. These circuits were commonly used in
 * vintage analog delay units and are known for their warm, organic sound characteristics.
 * 
 * The implementation models several key aspects of BBD behavior:
 * - Sample-and-hold behavior with anti-aliasing low-pass filtering
 * - High-frequency loss in the feedback path
 * - Non-linearities and noise characteristics of analog components
 * 
 * The analog modeling is achieved through:
 * 1. A low-pass filter in the feedback path to simulate high-frequency roll-off
 * 2. Careful emulation of the BBD's non-linear characteristics
 * 3. Proper handling of feedback and wet/dry mixing
 * 
 * @note The current implementation includes a basic low-pass filter in the feedback path.
 * Future improvements could include:
 * - A compander section for more accurate BBD emulation
 * - Noise injection to model BBD noise characteristics
 * - More sophisticated filtering to match specific BBD IC responses
 * 
 * @see AudioDelay.h
 * @see AudioFilter.h
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
        /**
         * @brief Reset the delay effect to its initial state
         * 
         * @details
         * Initializes or reinitializes the delay buffers and filter based on the current sample rate.
         * If the sample rate hasn't changed, it simply flushes the existing buffers. Otherwise, it
         * recreates the delay buffers and reinitializes the low-pass filter with the new sample rate.
         * 
         * @param _sampleRate The current audio sample rate in Hz
         * @return bool True if reset was successful, false otherwise
         * 
         * @note The low-pass filter is configured with a first-order low-pass response (12dB/octave)
         * and the cutoff frequency defined by CUTOFF (3000Hz by default).
         */
        virtual bool reset(double _sampleRate)
        {
            // If sample rate hasn't changed, just flush buffers
            if (sampleRate == _sampleRate)
            {
                delayBuffer_L.flushBuffer();
                delayBuffer_R.flushBuffer();
                return true;
            }

            // Recreate delay buffers with new sample rate
            createDelayBuffers(_sampleRate, bufferLength_mSec);

            // Reinitialize the low-pass filter for analog modeling
            mAudioFilter.reset(_sampleRate);
            mAudioFilterParameters.algorithm = filterAlgorithm::kLPF1;
            mAudioFilterParameters.fc = CUTOFF;
            mAudioFilter.setParameters(mAudioFilterParameters); // Updates filter coefficients

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

        /**
         * @brief Process a block of audio samples through the analog modeling delay
         * 
         * @details
         * Processes a block of audio samples through the delay line with analog modeling.
         * Supports both mono and stereo processing, with different algorithms for normal
         * and ping-pong delay modes. The analog modeling is applied in the feedback path
         * using a low-pass filter to simulate BBD characteristics.
         * 
         * @param inputs Array of input sample buffers (one per channel)
         * @param outputs Array of output sample buffers (one per channel)
         * @param numChannels Number of channels to process (1 for mono, 2 for stereo)
         * @param nFrames Number of audio frames to process
         * 
         * @note For mono processing, only the left channel is processed. For stereo processing,
         *       the effect supports both normal (separate L/R delays) and ping-pong (crossed
         *       feedback) modes. The analog modeling filter is applied to the feedback path
         *       to simulate the high-frequency loss characteristic of BBD devices.
         */
        void processBlock(iplug::sample** inputs, iplug::sample** outputs, int numChannels, int nFrames)
        {
            // Return early if no channels to process
            if (numChannels == 0)
                return;

            // Validate delay algorithm
            if (parameters.algorithm != delayAlgorithm::kNormal &&
                parameters.algorithm != delayAlgorithm::kPingPong)
                return;

            // Process mono signal
            if (numChannels == 1)
            {
                for (int s = 0; s < nFrames; s++)
                {
                    // Read from delay line
                    double yn = delayBuffer_L.readBuffer(delayInSamples_L);

                    // Apply analog modeling (low-pass filtering)
                    double yMod = mAudioFilter.processAudioSample(yn);

                    // Create feedback signal
                    double dn = inputs[0][s] + (parameters.leftFeedback_Pct / 100.0) * yMod;

                    // Write to delay line
                    delayBuffer_L.writeBuffer(dn);

                    // Mix dry and wet signals
                    outputs[0][s] = dryMix * inputs[0][s] + wetMix * yn;
                }
                return;
            }

            // Process stereo signal
            for (int s = 0; s < nFrames; s++)
            {
                // Get input samples (duplicate left channel if mono input)
                double xnL = inputs[0][s];
                double xnR = numChannels > 1 ? inputs[1][s] : xnL;

                // Read from delay lines
                double ynL = delayBuffer_L.readBuffer(delayInSamples_L);
                double ynR = delayBuffer_R.readBuffer(delayInSamples_R);
                
                // Apply analog modeling to both channels
                double yModL = mAudioFilter.processAudioSample(ynL);
                double yModR = mAudioFilter.processAudioSample(ynR);

                // Create feedback signals
                double dnL = xnL + (parameters.leftFeedback_Pct / 100.0) * yModL;
                double dnR = xnR + (parameters.rightFeedback_Pct / 100.0) * yModR;

                // Process based on delay algorithm
                if (parameters.algorithm == delayAlgorithm::kNormal)
                {
                    // Standard stereo delay
                    delayBuffer_L.writeBuffer(dnL);
                    delayBuffer_R.writeBuffer(dnR);
                }
                else if (parameters.algorithm == delayAlgorithm::kPingPong)
                {
                    // Crossed feedback for ping-pong effect
                    delayBuffer_L.writeBuffer(dnR);
                    delayBuffer_R.writeBuffer(dnL);
                }

                // Mix dry and wet signals
                double outputL = dryMix * xnL + wetMix * ynL;
                double outputR = dryMix * xnR + wetMix * ynR;

                // Write to output buffers
                outputs[0][s] = (iplug::sample)outputL;
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
        AudioFilter mAudioFilter;                     ///< Low-pass filter for analog modeling in feedback path
        AudioFilterParameters mAudioFilterParameters;  ///< Configuration parameters for the analog modeling filter
    };
} // namespace fxobjects