/*
*   Homework Chapter 14.8 - 3. Implement the LCR delay in Figure 14.17.
* 
    The Left-Center-Right (LCR) delay shown in Figure 14.17 is based on the Korg Triton delay effect.
    There are three delay lines and only the center includes a feedback path. The delay time for each
    channel (L, C, and R) is independently adjustable. The filters in the feedback path have bypass
    switches so that they may be used alone or together in series.
*/

#pragma once
#include "include/AudioDelay.h"
#include "include/AudioFilter.h"
#include <IPlugAPIBase.h> // for namespace

enum EDelays
{
    kDelay_L = 0,
    kDelay_R,
    kDelay_C,
    kNumDelays
};

enum EFilterMode
{
    kBYPASS,
    kHPF,
    kLPF,
    kALL,
    kNumFilterModes
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
    class LcrDelay : public IAudioSignalProcessor
    {
    public:
        LcrDelay()
        {
            mAudioFilterParameters_HP.algorithm = filterAlgorithm::kHPF1;
            mAudioFilterParameters_LP.algorithm = filterAlgorithm::kLPF1;

            mAudioFilter_HP.setParameters(mAudioFilterParameters_HP);
            mAudioFilter_LP.setParameters(mAudioFilterParameters_LP);
        }		/* C-TOR */
        ~LcrDelay() {}	/* D-TOR */

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

        /** process MONO audio delay */
        /**
        \param xn input
        \return the processed sample
        */
        virtual double processAudioSample(double xn)
        {
            // LCR - left for center delay line (TODO - check amplitude)
            double xnLL = xn;
            // --- read delay
            double ynL = delayBuffer_L.readBuffer(delayInSamples_L);
            double ynC = delayBuffer_C.readBuffer(delayInSamples_C);
            double yModC = 0.0;
            switch (mFilterMode) {
            case EFilterMode::kBYPASS:
                break;
            case EFilterMode::kHPF:
                yModC = mAudioFilter_HP.processAudioSample(ynC);
                break;
            case EFilterMode::kLPF:
                yModC = mAudioFilter_LP.processAudioSample(ynC);
                break;
            case EFilterMode::kALL:
                yModC = mAudioFilter_HP.processAudioSample(ynC);
                yModC = mAudioFilter_LP.processAudioSample(yModC);
                break;
            default:
                break;
            }
            // --- create input for delay buffer with Center info
            double dnC = xnLL + (parameters.centerFeedback_Pct / 100.0) * yModC;

            // --- create input for delay buffer
            double dn_L = xn;

            // --- write to delay buffer
            delayBuffer_L.writeBuffer(dn_L);
            delayBuffer_C.writeBuffer(dnC);

            // --- form mixture out = dry*xn + wet*yn
            double output = dryMix_L * xn + wetMix_L * ynL + wetMix_C * ynC;

            return output;
        }

        /** return true: this object can also process frames */
        virtual bool canProcessAudioFrame() { return true; }
        /*
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
            // ---
            double xnL = inputFrame[0];

            // --- RIGHT channel (duplicate left input if mono-in)
            double xnR = inputChannels > 1 ? inputFrame[1] : xnL;

            // LCR - sum left and right for center delay line
            double xnC = xnL + xnR;

            // --- read delay LEFT
            double ynL = delayBuffer_L.readBuffer(delayInSamples_L);

            // --- read delay RIGHT
            double ynR = delayBuffer_R.readBuffer(delayInSamples_R);

            // --- read delay CENTER
            double ynC = delayBuffer_C.readBuffer(delayInSamples_C);

            // analog modeling - apply filter to center
            double yModC = 0.0;

            switch (mFilterMode) {
            case EFilterMode::kBYPASS:
                break;
            case EFilterMode::kHPF:
                yModC = mAudioFilter_HP.processAudioSample(ynC);
                break;
            case EFilterMode::kLPF:
                yModC = mAudioFilter_LP.processAudioSample(ynC);
                break;
            case EFilterMode::kALL:
                yModC = mAudioFilter_HP.processAudioSample(ynC);
                yModC = mAudioFilter_LP.processAudioSample(yModC);
                break;
            default:
                break;
            }
            // --- create input for delay buffer with Center info
            double dnC = xnC + (parameters.centerFeedback_Pct / 100.0) * yModC;

            // --- create input for delay buffer with LEFT channel info
            double dnL = xnL;

            // --- create input for delay buffer with RIGHT channel info
            double dnR = xnR;

            // --- decode
            if (parameters.algorithm == delayAlgorithm::kNormal)
            {
                // --- write to LEFT delay buffer with LEFT channel info
                delayBuffer_L.writeBuffer(dnL);

                // --- write to RIGHT delay buffer with RIGHT channel info
                delayBuffer_R.writeBuffer(dnR);

                // --- write to CENTER delay buffer with CENTER info
                delayBuffer_C.writeBuffer(dnC);
            }
            else if (parameters.algorithm == delayAlgorithm::kPingPong)
            {
                // --- write to LEFT delay buffer with RIGHT channel info
                delayBuffer_L.writeBuffer(dnR);

                // --- write to RIGHT delay buffer with LEFT channel info
                delayBuffer_R.writeBuffer(dnL);

                // --- write to CENTER delay buffer with CENTER info
                delayBuffer_C.writeBuffer(dnC);
            }

            // --- form mixture out = dry*xn + wet*yn
            double outputL = dryMix_L * xnL + wetMix_L * ynL;

            // --- form mixture out = dry*xn + wet*yn
            double outputR = dryMix_R * xnR + wetMix_R * ynR;

            // --- form mixture out = dry*xn + wet*yn
            double outputC = wetMix_C * ynC;

            // --- set left channel
            outputFrame[0] = (float)outputL + outputC;

            // --- set right channel
            outputFrame[1] = (float)outputR + outputC;

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
            // TODO - not used atm
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

        // process a block and call the Filter processblock after
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
                    double xnL = inputs[0][s];
                    // LCR - left for center delay line (TODO - check amplitude)
                    double xnLL = xnL;
                    // --- read delay
                    double ynL = delayBuffer_L.readBuffer(delayInSamples_L);
                    double ynC = delayBuffer_C.readBuffer(delayInSamples_C);
                    double yModC = 0.0;
                    switch (mFilterMode) {
                    case EFilterMode::kBYPASS:
                        break;
                    case EFilterMode::kHPF:
                        yModC = mAudioFilter_HP.processAudioSample(ynC);
                        break;
                    case EFilterMode::kLPF:
                        yModC = mAudioFilter_LP.processAudioSample(ynC);
                        break;
                    case EFilterMode::kALL:
                        yModC = mAudioFilter_HP.processAudioSample(ynC);
                        yModC = mAudioFilter_LP.processAudioSample(yModC);
                        break;
                    default:
                        break;
                    }
                    // --- create input for delay buffer with Center info
                    double dnC = xnLL + (parameters.centerFeedback_Pct / 100.0) * yModC;

                    // --- create input for delay buffer
                    double dn_L = xnL;

                    // --- write to delay buffer
                    delayBuffer_L.writeBuffer(dn_L);
                    delayBuffer_C.writeBuffer(dnC);

                    // --- form mixture out = dry*xn + wet*yn
                    outputs[0][s] = dryMix_L * xnL + wetMix_L * xnL + wetMix_C * ynC;
                }
                return;
            }

            // --- if we get here we know we have 2 output channels
            //
            // --- process n frames
            for (int s = 0; s < nFrames; s++)
            {
                double xnL = inputs[0][s];

                // --- RIGHT channel (duplicate left input if mono-in)
                double xnR = numChannels > 1 ? inputs[1][s] : xnL;

                // LCR - sum left and right for center delay line
                double xnC = xnL + xnR;

                // --- read delay LEFT
                double ynL = delayBuffer_L.readBuffer(delayInSamples_L);

                // --- read delay RIGHT
                double ynR = delayBuffer_R.readBuffer(delayInSamples_R);
                
                // --- read delay CENTER
                double ynC = delayBuffer_C.readBuffer(delayInSamples_C);

                // analog modeling - apply filter to center
                double yModC = 0.0;

                switch (mFilterMode) {
                case EFilterMode::kBYPASS:
                    yModC = ynC;
                    break;
                case EFilterMode::kHPF:
                    yModC = mAudioFilter_HP.processAudioSample(ynC);
                    break;
                case EFilterMode::kLPF:
                    yModC = mAudioFilter_LP.processAudioSample(ynC);
                    break;
                case EFilterMode::kALL:
                    yModC = mAudioFilter_HP.processAudioSample(ynC);
                    yModC = mAudioFilter_LP.processAudioSample(yModC);
                    break;
                default:
                    yModC = ynC;
                    break;
                }
                // --- create input for delay buffer with Center info
                double dnC = xnC + (parameters.centerFeedback_Pct / 100.0) * yModC;

                // --- create input for delay buffer with LEFT channel info
                double dnL = xnL;

                // --- create input for delay buffer with RIGHT channel info
                double dnR = xnR;

                // --- decode
                if (parameters.algorithm == delayAlgorithm::kNormal)
                {
                    // --- write to LEFT delay buffer with LEFT channel info
                    delayBuffer_L.writeBuffer(dnL);

                    // --- write to RIGHT delay buffer with RIGHT channel info
                    delayBuffer_R.writeBuffer(dnR);
                    
                    // --- write to CENTER delay buffer with CENTER info
                    delayBuffer_C.writeBuffer(dnC);
                }
                else if (parameters.algorithm == delayAlgorithm::kPingPong)
                {
                    // --- write to LEFT delay buffer with RIGHT channel info
                    delayBuffer_L.writeBuffer(dnR);

                    // --- write to RIGHT delay buffer with LEFT channel info
                    delayBuffer_R.writeBuffer(dnL);

                    // --- write to CENTER delay buffer with CENTER info
                    delayBuffer_C.writeBuffer(dnC);
                }

                // --- form mixture out = dry*xn + wet*yn
                double outputL = dryMix_L * xnL + wetMix_L * ynL;

                // --- form mixture out = dry*xn + wet*yn
                double outputR = dryMix_R * xnR + wetMix_R * ynR;
                
                // --- form mixture out = dry*xn + wet*yn
                double outputC = wetMix_C * ynC;

                // --- set left channel
                outputs[0][s] = (iplug::sample)outputL + outputC;

                // --- set right channel
                outputs[1][s] = (iplug::sample)outputR + outputC;
            }
        }

        // set the cutoff frequency for HP filter
        void SetFrequencyHP(double _fc)
        {
            if (mAudioFilterParameters_HP.fc != _fc)
            {
                mAudioFilterParameters_HP.fc = _fc;
                mAudioFilter_HP.setParameters(mAudioFilterParameters_HP);
            }
        }

        // set the cutoff frequency for LP filter
        void SetFrequencyLP(double _fc)
        {
            if (mAudioFilterParameters_LP.fc != _fc)
            {
                mAudioFilterParameters_LP.fc = _fc;
                mAudioFilter_LP.setParameters(mAudioFilterParameters_LP);
            }
        }

        // set the mode of filter
        // BYPASS(no filter), HP, LP and HP followed by LP
        // reset filters when mode changes
        void SetFilterMode(EFilterMode _mode, double _sampleRate)
        {
            if (mFilterMode != _mode)
            {
                mFilterMode = _mode;

                mAudioFilter_HP.reset(_sampleRate);
                mAudioFilter_LP.reset(_sampleRate);

                mAudioFilterParameters_HP.algorithm = filterAlgorithm::kHPF1;
                mAudioFilterParameters_HP.fc = 1000.0; // initial fc

                mAudioFilterParameters_LP.algorithm = filterAlgorithm::kLPF1;
                mAudioFilterParameters_LP.fc = 1000.0; // initial fc

                // setParameters to get AudioFilter::calculateFilterCoeffs() called
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

        // Low Pass and high Pass Filter for the Center Delay
        AudioFilter mAudioFilter_LP;
        AudioFilter mAudioFilter_HP;
        AudioFilterParameters mAudioFilterParameters_HP;
        AudioFilterParameters mAudioFilterParameters_LP;
        EFilterMode mFilterMode = EFilterMode::kBYPASS;
    };
} // namespace fxobjects