/**
\class PeakLimiter
\ingroup FX-Objects
\brief
The PeakLimiter object implements a simple peak limiter; it is really a simplified and hard-wired
versio of the DynamicsProcessor

Audio I/O:
- Processes mono input to mono output.

Control I/F:
- setThreshold_dB(double _threshold_dB) to adjust the limiter threshold
- setMakeUpGain_dB(double _makeUpGain_dB) to adjust the makeup gain

\author Will Pirkle http://www.willpirkle.com
\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/
#pragma once
#include "IAudioSignalProcessor.h"
#include "AudioDetector.h"
#include "EnumsAndStructs.h"
#include "helperfunctions.h"
#include "Constants.h"

namespace fxobjects {

    class PeakLimiter : public IAudioSignalProcessor
    {
    public:
        PeakLimiter() { setThreshold_dB(-3.0); }
        ~PeakLimiter() {}
    
        /** reset members to initialized state */
        virtual bool reset(double _sampleRate)
        {
            // --- init; true = analog time-constant
            //detector.setSampleRate(_sampleRate);
            detector.reset(_sampleRate);
    
            AudioDetectorParameters detectorParams = detector.getParameters();
            detectorParams.detect_dB = true;
            detectorParams.attackTime_mSec = 5.0;
            detectorParams.releaseTime_mSec = 25.0;
            detectorParams.clampToUnityMax = false;
            detectorParams.detectMode = ENVELOPE_DETECT_MODE_PEAK;
            detector.setParameters(detectorParams);
    
            return true;
        }
    
        /** return false: this object only processes samples */
        virtual bool canProcessAudioFrame() { return false; }
    
        /** process audio: implement hard limiter */
        /**
        \param xn input
        \return the processed sample
        */
        virtual double processAudioSample(double xn)
        {
            return dB2Raw(makeUpGain_dB)*xn*computeGain(detector.processAudioSample(xn));
        }
    
        /** compute the gain reduction value based on detected value in dB */
        double computeGain(double detect_dB)
        {
            double output_dB = 0.0;
    
            // --- defaults - you can change these here
            bool softknee = true;
            double kneeWidth_dB = 10.0;
    
            // --- hard knee
            if (!softknee)
            {
                // --- below threshold, unity
                if (detect_dB <= threshold_dB)
                    output_dB = detect_dB;
                // --- above threshold, compress
                else
                    output_dB = threshold_dB;
            }
            else
            {
                // --- calc gain with knee
                // --- left side of knee, outside of width, unity gain zone
                if (2.0*(detect_dB - threshold_dB) < -kneeWidth_dB)
                    output_dB = detect_dB;
                // --- inside the knee,
                else if (2.0*(fabs(detect_dB - threshold_dB)) <= kneeWidth_dB)
                    output_dB = detect_dB - pow((detect_dB - threshold_dB + (kneeWidth_dB / 2.0)), 2.0) / (2.0*kneeWidth_dB);
                // --- right of knee, compression zone
                else if (2.0*(detect_dB - threshold_dB) > kneeWidth_dB)
                    output_dB = threshold_dB;
            }
    
            // --- convert difference between threshold and detected to raw
            return  pow(10.0, (output_dB - detect_dB) / 20.0);
        }
    
        /** adjust threshold in dB */
        void setThreshold_dB(double _threshold_dB) { threshold_dB = _threshold_dB; }
    
        /** adjust makeup gain in dB*/
        void setMakeUpGain_dB(double _makeUpGain_dB) { makeUpGain_dB = _makeUpGain_dB; }
    
    protected:
        AudioDetector detector;		///< the detector object
        double threshold_dB = 0.0;	///< stored threshold (dB)
        double makeUpGain_dB = 0.0;	///< stored makeup gain (dB)
    };
} // namespace fxobjects
