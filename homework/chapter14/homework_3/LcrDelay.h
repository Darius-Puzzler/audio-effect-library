/**
 * @file LcrDelay.h
 * @brief Implementation of a Left-Center-Right (LCR) delay effect based on the Korg Triton delay
 * 
 * This file implements a three-tap delay effect with independent control over left, center, 
 * and right delay lines. The center channel includes a feedback path with optional filtering.
 * Based on the Korg Triton delay effect design from Chapter 14.8, Figure 14.17.
 * 
 * Features:
 * - Three independent delay lines (Left, Center, Right)
 * - Configurable feedback path in the center channel only
 * - Optional high-pass and low-pass filtering in the feedback path
 * - Independent delay time control for each channel
 * - Filter bypass options for flexible sound shaping
 */

 #pragma once
 #include "include/AudioDelay.h"
 #include "include/AudioFilter.h"
 #include <IPlugAPIBase.h> // for namespace
 
 /**
  * @brief Enumeration of delay line channels
  */
 enum EDelays
 {
     kDelay_L = 0,  ///< Left channel delay
     kDelay_R,      ///< Right channel delay
     kDelay_C,      ///< Center channel delay (includes feedback path)
     kNumDelays     ///< Total number of delay channels
 };
 
 /**
  * @brief Enumeration of filter modes for the feedback path
  */
 enum EFilterMode
 {
     kBYPASS,    ///< No filtering in the feedback path
     kHPF,       ///< High-pass filter only (removes low frequencies)
     kLPF,       ///< Low-pass filter only (removes high frequencies)
     kALL,       ///< Both high-pass and low-pass filters in series
     kNumFilterModes  ///< Total number of filter modes
 };

namespace fxobjects
{
    struct LCRAudioDelayParameters
    {
        LCRAudioDelayParameters() {}
        /** all FXObjects parameter objects require overloaded= operator so remember to add new entries if you add new variables. */
        LCRAudioDelayParameters& operator=(const LCRAudioDelayParameters& params)	// need this override for collections to work
        {
            if (this == &params)
                return *this;

            algorithm = params.algorithm;
            wetLevel_dB_L = params.wetLevel_dB_L;
            dryLevel_dB_L = params.dryLevel_dB_L;
            wetLevel_dB_R = params.wetLevel_dB_R;
            dryLevel_dB_R = params.dryLevel_dB_R;
            wetLevel_dB_C = params.wetLevel_dB_C;
            centerFeedback_Pct = params.centerFeedback_Pct;

            updateType = params.updateType;
            leftDelay_mSec = params.leftDelay_mSec;
            rightDelay_mSec = params.rightDelay_mSec;
            centerDelay_mSec = params.centerDelay_mSec;
            delayRatio_Pct = params.delayRatio_Pct;

            return *this;
        }

        // --- individual parameters
        delayAlgorithm algorithm = delayAlgorithm::kNormal; ///< delay algorithm
        double wetLevel_dB_L = -3.0;	///< wet output level in dB
        double dryLevel_dB_L = -3.0;	///< dry output level in dB
        double wetLevel_dB_R = -3.0;	///< wet output level in dB
        double dryLevel_dB_R = -3.0;	///< dry output level in dB
        double wetLevel_dB_C = -3.0;	///< wet output level in dB
        double centerFeedback_Pct = 0.0;	///< center feedback as a % value

        delayUpdateType updateType = delayUpdateType::kLeftAndRight;///< update algorithm
        double leftDelay_mSec = 0.0;	///< left delay time
        double rightDelay_mSec = 0.0;	///< right delay time
        double centerDelay_mSec = 0.0; // center delay time
        double delayRatio_Pct = 100.0;	///< dela ratio: right length = (delayRatio)*(left length)
    };

    /**
     * @class LcrDelay
     * @brief Implements a Left-Center-Right delay effect with feedback on the center channel
     * 
     * This class provides a three-tap delay effect with independent control over left, center,
     * and right delay lines. The center channel includes a feedback path with optional filtering.
     * The effect supports both normal and ping-pong delay algorithms.
     * 
     * The delay is based on the Korg Triton delay design, featuring:
     * - Independent delay times for each channel
     * - Configurable feedback on the center channel
     * - Optional high-pass and/or low-pass filtering in the feedback path
     * - Sample-rate independent operation
     */
    class LcrDelay : public IAudioSignalProcessor
    {
    public:
        /**
         * @brief Constructor
         * 
         * Initializes the LCR delay with default filter settings.
         * Sets up the high-pass and low-pass filters with their default algorithms.
         */
        LcrDelay()
        {
            mAudioFilterParameters_HP.algorithm = filterAlgorithm::kHPF1;
            mAudioFilterParameters_LP.algorithm = filterAlgorithm::kLPF1;

            mAudioFilter_HP.setParameters(mAudioFilterParameters_HP);
            mAudioFilter_LP.setParameters(mAudioFilterParameters_LP);
        }

        /**
         * @brief Destructor
         */
        ~LcrDelay() {}

        /**
         * @brief Reset the delay effect to its initial state
         * 
         * Initializes or reinitializes the delay buffers and filters based on the current sample rate.
         * If the sample rate hasn't changed, it flushes the existing buffers. Otherwise, it recreates
         * the delay buffers and reinitializes the filters with the new sample rate.
         * 
         * @param _sampleRate The current audio sample rate in Hz
         * @return bool True if reset was successful, false otherwise
         */
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

            // --- create new buffer for Left, Right and Center, will store sample rate and length(mSec)
            createDelayBuffers(_sampleRate, bufferLength_mSec);

            // filter member settings
            mAudioFilter_LP.reset(_sampleRate);
            mAudioFilter_HP.reset(_sampleRate);

            mAudioFilterParameters_HP.algorithm = filterAlgorithm::kHPF1;
            mAudioFilterParameters_LP.algorithm = filterAlgorithm::kLPF1;

            mAudioFilter_HP.setParameters(mAudioFilterParameters_HP);
            mAudioFilter_LP.setParameters(mAudioFilterParameters_LP);

            return true;
        }

        /**
         * @brief Process a single audio sample through the delay effect (mono)
         * 
         * This method processes a single mono audio sample through the LCR delay effect.
         * It reads from both the left and center delay lines, applies filtering to the
         * center channel if configured, and writes the new samples back to the delay lines.
         * 
         * @param xn Input sample to process
         * @return double Processed output sample with dry/wet mix
         */
        virtual double processAudioSample(double xn)
        {
            // LCR - left for center delay line (TODO - check amplitude)
            double xnLL = xn;
            
            // Read from delay lines
            double ynL = delayBuffer_L.readBuffer(delayInSamples_L);  // Left delay
            double ynC = delayBuffer_C.readBuffer(delayInSamples_C);  // Center delay (with feedback)
            double yModC = 0.0;  // Will hold filtered center signal
            
            // Apply filtering to center channel if needed
            switch (mFilterMode) {
            case EFilterMode::kBYPASS:
                yModC = ynC;  // No filtering
                break;
            case EFilterMode::kHPF:
                yModC = mAudioFilter_HP.processAudioSample(ynC);  // High-pass only
                break;
            case EFilterMode::kLPF:
                yModC = mAudioFilter_LP.processAudioSample(ynC);  // Low-pass only
                break;
            case EFilterMode::kALL:
                // Apply both high-pass and low-pass in series
                yModC = mAudioFilter_HP.processAudioSample(ynC);
                yModC = mAudioFilter_LP.processAudioSample(yModC);
                break;
            default:
                yModC = ynC;  // Default to no filtering
                break;
            }
            
            // Create input for center delay buffer with feedback
            // Feedback amount is controlled by parameters.centerFeedback_Pct (0-100%)
            double dnC = xnLL + (parameters.centerFeedback_Pct / 100.0) * yModC;

            // Prepare input for left delay buffer (no feedback)
            double dn_L = xn;

            // Write new samples to delay buffers
            delayBuffer_L.writeBuffer(dn_L);  // Write to left delay
            delayBuffer_C.writeBuffer(dnC);   // Write to center delay (with feedback)

            // Mix dry signal with delayed signals
            // Output = (dry * input) + (wet * left_delay) + (wet * center_delay)
            double output = dryMix_L * xn + wetMix_L * ynL + wetMix_C * ynC;

            return output;
        }

        /** return true: this object can also process frames */
        virtual bool canProcessAudioFrame() { return true; }
        /*
        /**
         * @brief Process a stereo frame of audio through the delay effect
         * 
         * This method processes a single frame of stereo audio through the LCR delay effect.
         * It handles both stereo and mono-to-stereo processing, applying the delay effect
         * according to the current algorithm (normal or ping-pong) and filter settings.
         * 
         * @param inputFrame Pointer to input frame buffer (inputFrame[0] = left, inputFrame[1] = right)
         * @param outputFrame Pointer to output frame buffer
         * @param inputChannels Number of input channels (1 for mono, 2 for stereo)
         * @param outputChannels Number of output channels (1 for mono, 2 for stereo)
         * @return bool True if processing was successful, false otherwise
         */
        virtual bool processAudioFrame(const float* inputFrame, float* outputFrame,
            uint32_t inputChannels, uint32_t outputChannels)
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
                outputFrame[0] = (float)processAudioSample(inputFrame[0]);
                return true;
            }

            // --- we have 2 output channels
            double xnL = inputFrame[0];  // Left input
            double xnR = inputChannels > 1 ? inputFrame[1] : xnL;  // Right input (or duplicate left if mono)

            // Sum left and right for center delay line input
            double xnC = xnL + xnR;

            // --- read from all three delay lines
            double ynL = delayBuffer_L.readBuffer(delayInSamples_L);  // Left delay
            double ynR = delayBuffer_R.readBuffer(delayInSamples_R);  // Right delay
            double ynC = delayBuffer_C.readBuffer(delayInSamples_C);  // Center delay (with feedback)

            // Apply filtering to center channel if needed
            double yModC = 0.0;
            switch (mFilterMode) {
            case EFilterMode::kBYPASS:
                yModC = ynC;  // No filtering
                break;
            case EFilterMode::kHPF:
                yModC = mAudioFilter_HP.processAudioSample(ynC);  // High-pass only
                break;
            case EFilterMode::kLPF:
                yModC = mAudioFilter_LP.processAudioSample(ynC);  // Low-pass only
                break;
            case EFilterMode::kALL:
                // Apply both high-pass and low-pass in series
                yModC = mAudioFilter_HP.processAudioSample(ynC);
                yModC = mAudioFilter_LP.processAudioSample(yModC);
                break;
            default:
                yModC = ynC;  // Default to no filtering
                break;
            }
            
            // --- create input for center delay buffer (with feedback)
            // Feedback amount is controlled by parameters.centerFeedback_Pct (0-100%)
            double dnC = xnC + (parameters.centerFeedback_Pct / 100.0) * yModC;

            double dnL = xnL;  // Left delay input
            double dnR = xnR;  // Right delay input

            // --- Process based on delay algorithm
            if (parameters.algorithm == delayAlgorithm::kNormal)
            {
                // Normal mode: process each channel independently
                delayBuffer_L.writeBuffer(dnL);  // Left to left
                delayBuffer_R.writeBuffer(dnR);  // Right to right
                delayBuffer_C.writeBuffer(dnC);  // Center with feedback
            }
            else if (parameters.algorithm == delayAlgorithm::kPingPong)
            {
                // Ping-pong mode: cross the left and right channels
                delayBuffer_L.writeBuffer(dnR);  // Right to left
                delayBuffer_R.writeBuffer(dnL);  // Left to right
                delayBuffer_C.writeBuffer(dnC);  // Center with feedback
            }

            // --- Mix dry and wet signals for left and right channels
            double outputL = dryMix_L * xnL + wetMix_L * ynL;  // Left dry/wet mix
            double outputR = dryMix_R * xnR + wetMix_R * ynR;  // Right dry/wet mix
            double outputC = wetMix_C * ynC;                   // Center wet only

            // --- Combine with center channel and write to outputs
            outputFrame[0] = (float)(outputL + outputC);  // Left + center
            outputFrame[1] = (float)(outputR + outputC);  // Right + center

            return true;
        }

        /** get parameters: note use of custom structure for passing param data */
        /**
        \return AudioDelayParameters custom data structure
        */
        LCRAudioDelayParameters getParameters() { return parameters; }

        /** set parameters: note use of custom structure for passing param data */
        /**
        \param AudioDelayParameters custom data structure
        */
        void setParameters(LCRAudioDelayParameters _parameters)
        {
            // --- check mix in dB for calc
            // LEFT
            if (_parameters.dryLevel_dB_L != parameters.dryLevel_dB_L)
                dryMix_L = pow(10.0, _parameters.dryLevel_dB_L / 20.0);
            if (_parameters.wetLevel_dB_L != parameters.wetLevel_dB_L)
                wetMix_L = pow(10.0, _parameters.wetLevel_dB_L / 20.0);
            // RIGHT
            if (_parameters.dryLevel_dB_R != parameters.dryLevel_dB_R)
                dryMix_R = pow(10.0, _parameters.dryLevel_dB_R / 20.0);
            if (_parameters.wetLevel_dB_R != parameters.wetLevel_dB_R)
                wetMix_R = pow(10.0, _parameters.wetLevel_dB_R / 20.0);
            // CENTER
            if (_parameters.wetLevel_dB_C != parameters.wetLevel_dB_C)
                wetMix_C = pow(10.0, _parameters.wetLevel_dB_C / 20.0);

            // --- save; rest of updates are cheap on CPU
            parameters = _parameters;

            // --- check update type first:
            if (parameters.updateType == delayUpdateType::kLeftAndRight)
            {
                // --- set left and right delay times
                // --- calculate total delay time in samples + fraction
                double newDelayInSamples_L = parameters.leftDelay_mSec * (samplesPerMSec);
                double newDelayInSamples_R = parameters.rightDelay_mSec * (samplesPerMSec);
                double newDelayInSamples_C = parameters.centerDelay_mSec * (samplesPerMSec);

                // --- new delay time with fraction
                delayInSamples_L = newDelayInSamples_L;
                delayInSamples_R = newDelayInSamples_R;
                delayInSamples_C = newDelayInSamples_C;
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
            delayBuffer_C.createCircularBuffer(bufferLength);
        }

        /**
         * @brief Process a block of audio samples through the delay effect
         * 
         * This method processes a block of audio samples through the LCR delay effect,
         * handling both mono and stereo input/output configurations. It applies the delay
         * effect according to the current algorithm (normal or ping-pong) and filter settings.
         * 
         * @param inputs Array of input sample buffers (one per channel)
         * @param outputs Array of output sample buffers (one per channel)
         * @param numChannels Number of input/output channels
         * @param nFrames Number of frames to process
         */
        void processBlock(iplug::sample** inputs, iplug::sample** outputs, int numChannels, int nFrames)
        {
            // --- validate inputs and check for early exit
            if (numChannels == 0)
                return;

            // --- ensure we support the current delay algorithm
            if (parameters.algorithm != delayAlgorithm::kNormal &&
                parameters.algorithm != delayAlgorithm::kPingPong)
                return;

            // --- Mono processing (single output channel)
            if (numChannels == 1)
            {
                for (int s = 0; s < nFrames; s++)
                {
                    double xnL = inputs[0][s];  // Left input
                    double xnLL = xnL;  // Copy for center delay input

                    // --- read from delay lines
                    double ynL = delayBuffer_L.readBuffer(delayInSamples_L);  // Left delay
                    double ynC = delayBuffer_C.readBuffer(delayInSamples_C);  // Center delay (with feedback)
                    double yModC = 0.0;  // Will hold filtered center signal

                    // Apply filtering to center channel if needed
                    switch (mFilterMode) {
                    case EFilterMode::kBYPASS:
                        yModC = ynC;  // No filtering
                        break;
                    case EFilterMode::kHPF:
                        yModC = mAudioFilter_HP.processAudioSample(ynC);  // High-pass only
                        break;
                    case EFilterMode::kLPF:
                        yModC = mAudioFilter_LP.processAudioSample(ynC);  // Low-pass only
                        break;
                    case EFilterMode::kALL:
                        // Apply both high-pass and low-pass in series
                        yModC = mAudioFilter_HP.processAudioSample(ynC);
                        yModC = mAudioFilter_LP.processAudioSample(yModC);
                        break;
                    default:
                        yModC = ynC;  // Default to no filtering
                        break;
                    }

                    // --- create input for center delay buffer with feedback
                    // Feedback amount is controlled by parameters.centerFeedback_Pct (0-100%)
                    double dnC = xnLL + (parameters.centerFeedback_Pct / 100.0) * yModC;

                    // Prepare input for left delay buffer (no feedback)
                    double dn_L = xnL;

                    // Write new samples to delay buffers
                    delayBuffer_L.writeBuffer(dn_L);  // Write to left delay
                    delayBuffer_C.writeBuffer(dnC);   // Write to center delay (with feedback)

                    // Mix dry signal with delayed signals
                    // Output = (dry * input) + (wet * left_delay) + (wet * center_delay)
                    outputs[0][s] = dryMix_L * xnL + wetMix_L * ynL + wetMix_C * ynC;
                }
                return;
            }

            // --- Stereo processing (2 output channels)
            for (int s = 0; s < nFrames; s++)
            {
                double xnL = inputs[0][s];  // Left input
                double xnR = numChannels > 1 ? inputs[1][s] : xnL;  // Right input (or duplicate left if mono)

                // Sum left and right for center delay line input
                double xnC = xnL + xnR;

                // --- read from all three delay lines
                double ynL = delayBuffer_L.readBuffer(delayInSamples_L);  // Left delay
                double ynR = delayBuffer_R.readBuffer(delayInSamples_R);  // Right delay
                double ynC = delayBuffer_C.readBuffer(delayInSamples_C);  // Center delay (with feedback)

                // Apply filtering to center channel if needed
                double yModC = 0.0;
                switch (mFilterMode) {
                case EFilterMode::kBYPASS:
                    yModC = ynC;  // No filtering
                    break;
                case EFilterMode::kHPF:
                    yModC = mAudioFilter_HP.processAudioSample(ynC);  // High-pass only
                    break;
                case EFilterMode::kLPF:
                    yModC = mAudioFilter_LP.processAudioSample(ynC);  // Low-pass only
                    break;
                case EFilterMode::kALL:
                    // Apply both high-pass and low-pass in series
                    yModC = mAudioFilter_HP.processAudioSample(ynC);
                    yModC = mAudioFilter_LP.processAudioSample(yModC);
                    break;
                default:
                    yModC = ynC;  // Default to no filtering
                    break;
                }
                
                // --- create input for center delay buffer (with feedback)
                // Feedback amount is controlled by parameters.centerFeedback_Pct (0-100%)
                double dnC = xnC + (parameters.centerFeedback_Pct / 100.0) * yModC;

                double dnL = xnL;  // Left delay input
                double dnR = xnR;  // Right delay input

                // --- Process based on delay algorithm
                if (parameters.algorithm == delayAlgorithm::kNormal)
                {
                    // Normal mode: process each channel independently
                    delayBuffer_L.writeBuffer(dnL);  // Left to left
                    delayBuffer_R.writeBuffer(dnR);  // Right to right
                    delayBuffer_C.writeBuffer(dnC);  // Center with feedback
                }
                else if (parameters.algorithm == delayAlgorithm::kPingPong)
                {
                    // Ping-pong mode: cross the left and right channels
                    delayBuffer_L.writeBuffer(dnR);  // Right to left
                    delayBuffer_R.writeBuffer(dnL);  // Left to right
                    delayBuffer_C.writeBuffer(dnC);  // Center with feedback
                }

                // --- Mix dry and wet signals for left and right channels
                double outputL = dryMix_L * xnL + wetMix_L * ynL;  // Left dry/wet mix
                double outputR = dryMix_R * xnR + wetMix_R * ynR;  // Right dry/wet mix
                double outputC = wetMix_C * ynC;                   // Center wet only

                // --- Combine with center channel and write to outputs
                outputs[0][s] = (iplug::sample)(outputL + outputC);  // Left + center
                outputs[1][s] = (iplug::sample)(outputR + outputC);  // Right + center
            }
        }

        /**
         * @brief Set the cutoff frequency for the high-pass filter in the feedback path
         * 
         * This method updates the cutoff frequency of the high-pass filter applied to the
         * center channel feedback. The change is only applied if the new frequency differs
         * from the current setting to avoid unnecessary recalculations.
         * 
         * @param _fc The new cutoff frequency in Hz (must be positive)
         */
        void SetFrequencyHP(double _fc)
        {
            if (mAudioFilterParameters_HP.fc != _fc)
            {
                mAudioFilterParameters_HP.fc = _fc;
                mAudioFilter_HP.setParameters(mAudioFilterParameters_HP);
            }
        }

        /**
         * @brief Set the cutoff frequency for the low-pass filter in the feedback path
         * 
         * This method updates the cutoff frequency of the low-pass filter applied to the
         * center channel feedback. The change is only applied if the new frequency differs
         * from the current setting to avoid unnecessary recalculations.
         * 
         * @param _fc The new cutoff frequency in Hz (must be positive)
         */
        void SetFrequencyLP(double _fc)
        {
            if (mAudioFilterParameters_LP.fc != _fc)
            {
                mAudioFilterParameters_LP.fc = _fc;
                mAudioFilter_LP.setParameters(mAudioFilterParameters_LP);
            }
        }

        /**
         * @brief Set the filter mode for the center channel feedback path
         * 
         * This method configures the filtering applied to the center channel feedback.
         * It resets and reinitializes the filter components when the mode changes to ensure
         * clean transitions between filter states. The method supports four filter modes:
         * - kBYPASS: No filtering in the feedback path
         * - kHPF: Apply high-pass filtering only
         * - kLPF: Apply low-pass filtering only
         * - kALL: Apply both high-pass and low-pass filters in series
         * 
         * @param _mode The desired filter mode from the EFilterMode enum
         * @param _sampleRate The current audio sample rate in Hz (used for filter reset)
         */
        void SetFilterMode(EFilterMode _mode, double _sampleRate)
        {
            if (mFilterMode != _mode)
            {
                mFilterMode = _mode;

                // Reset filters to prevent artifacts when changing modes
                mAudioFilter_HP.reset(_sampleRate);
                mAudioFilter_LP.reset(_sampleRate);

                // Configure high-pass filter (1st order)
                mAudioFilterParameters_HP.algorithm = filterAlgorithm::kHPF1;
                mAudioFilterParameters_HP.fc = 1000.0;  // Default cutoff: 1kHz

                // Configure low-pass filter (1st order)
                mAudioFilterParameters_LP.algorithm = filterAlgorithm::kLPF1;
                mAudioFilterParameters_LP.fc = 1000.0;  // Default cutoff: 1kHz

                // Update filter parameters and recalculate coefficients
                mAudioFilter_HP.setParameters(mAudioFilterParameters_HP);
                mAudioFilter_LP.setParameters(mAudioFilterParameters_LP);
            }
        }

    private:
        LCRAudioDelayParameters parameters; ///< object parameters

        double sampleRate = 0.0;		///< current sample rate
        double samplesPerMSec = 0.0;	///< samples per millisecond, for easy access calculation
        double delayInSamples_L = 0.0;	///< double includes fractional part
        double delayInSamples_R = 0.0;	///< double includes fractional part
        double delayInSamples_C = 0.0;	///< double includes fractional part
        double bufferLength_mSec = 0.0;	///< buffer length in mSec
        unsigned int bufferLength = 0;	///< buffer length in samples
        double wetMix_L = 0.707; ///< wet output default = -3dB
        double dryMix_L = 0.707; ///< dry output default = -3dB
        double wetMix_R = 0.707; ///< wet output default = -3dB
        double dryMix_R = 0.707; ///< dry output default = -3dB
        double wetMix_C = 0.707; ///< wet output default = -3dB

        // --- delay buffer of doubles
        CircularBuffer<double> delayBuffer_L;	///< LEFT delay buffer of doubles
        CircularBuffer<double> delayBuffer_R;	///< RIGHT delay buffer of doubles
        CircularBuffer<double> delayBuffer_C;	///< RIGHT delay buffer of doubles

        // --- Low Pass and high Pass Filter for the Center Delay
        AudioFilter mAudioFilter_LP;              ///< Low-pass filter for center channel feedback
        AudioFilter mAudioFilter_HP;              ///< High-pass filter for center channel feedback
        AudioFilterParameters mAudioFilterParameters_HP;  ///< Parameters for high-pass filter
        AudioFilterParameters mAudioFilterParameters_LP;  ///< Parameters for low-pass filter
        EFilterMode mFilterMode = EFilterMode::kBYPASS;  ///< Current filter mode for feedback path
    };
} // namespace fxobjects