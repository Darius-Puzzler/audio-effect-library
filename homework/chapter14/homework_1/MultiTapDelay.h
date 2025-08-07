/**
 * @file MultiTapDelay.h
 * @brief A multi-tap delay effect processor with various tap time relationships.
 * 
 * This class implements a multi-tap delay effect with the following features:
 * - Multiple delay tap modes (Custom, Golden Ratio, Primes)
 * - Tempo-synchronized delay times
 * - Stereo processing
 * - Parameter smoothing
 */

#pragma once

#include <IPlugConstants.h>
#include "include/AudioDelay.h"
#include <IControls.h>
#include <IGraphics.h>
#include <Smoothers.h>
#include <algorithm>

using namespace fxobjects;

/** @brief Enumeration of delay tap indices */
enum EDelays
{
    kDelay0 = 0,   ///< First delay tap
    kDelay1,       ///< Second delay tap
    kDelay2,       ///< Third delay tap
    kDelay3,       ///< Fourth delay tap
    kNumDelays     ///< Total number of delay taps
};

/** @brief Enumeration of plugin parameters */
enum EParams
{
    // Delay time parameters (in milliseconds)
    kDelayTime_mSec_Tap0_L = 0,  ///< Left channel delay time for tap 0
    kDelayTime_mSec_Tap0_R,      ///< Right channel delay time for tap 0
    kDelayTime_mSec_Tap1_L,      ///< Left channel delay time for tap 1
    kDelayTime_mSec_Tap1_R,      ///< Right channel delay time for tap 1
    kDelayTime_mSec_Tap2_L,      ///< Left channel delay time for tap 2
    kDelayTime_mSec_Tap2_R,      ///< Right channel delay time for tap 2
    kDelayTime_mSec_Tap3_L,      ///< Left channel delay time for tap 3
    kDelayTime_mSec_Tap3_R,      ///< Right channel delay time for tap 3

    // Tempo division parameters
    kDelayDivision0_L,  ///< Left channel tempo division for tap 0
    kDelayDivision0_R,  ///< Right channel tempo division for tap 0
    kDelayDivision1_L,  ///< Left channel tempo division for tap 1
    kDelayDivision1_R,  ///< Right channel tempo division for tap 1
    kDelayDivision2_L,  ///< Left channel tempo division for tap 2
    kDelayDivision2_R,  ///< Right channel tempo division for tap 2
    kDelayDivision3_L,  ///< Left channel tempo division for tap 3
    kDelayDivision3_R,  ///< Right channel tempo division for tap 3
    
    // Feedback parameters (only applied to the first/oldest delay tap)
    kDelayFeedback_Pct_Tap0_L,  ///< Left channel feedback amount for tap 0
    kDelayFeedback_Pct_Tap0_R,  ///< Right channel feedback amount for tap 0
    
    // Global parameters
    kGain,          ///< Output gain
    kDelaySync,     ///< Tempo sync enable/disable
    kTapMode,       ///< Current tap mode (Custom, Golden Ratio, Primes)
    kNumParams      ///< Total number of parameters
};

/** @brief Defines the relationship between delay tap times */
enum class ETapModes
{
    kCustomDelay = 0,  ///< Custom delay times (no fixed relationship)
    kGoldenRatio,      ///< Delay times follow the golden ratio (1.618...)
    kPrimes,           ///< Delay times follow prime number ratios
    kNumTapModes       ///< Total number of tap modes
};

/** @brief Enumeration of smoothed parameters */
enum EModulations
{
    kModDelayTime0_L,  ///< Smoothed left channel delay time for tap 0
    kModDelayTime0_R,  ///< Smoothed right channel delay time for tap 0
    kModDelayTime1_L,  ///< Smoothed left channel delay time for tap 1
    kModDelayTime1_R,  ///< Smoothed right channel delay time for tap 1
    kModDelayTime2_L,  ///< Smoothed left channel delay time for tap 2
    kModDelayTime2_R,  ///< Smoothed right channel delay time for tap 2
    kModDelayTime3_L,  ///< Smoothed left channel delay time for tap 3
    kModDelayTime3_R,  ///< Smoothed right channel delay time for tap 3
    kModFeedback_L,    ///< Smoothed left channel feedback
    kModFeedback_R,    ///< Smoothed right channel feedback
    kModGain,         ///< Smoothed output gain
    kNumModulations   ///< Total number of smoothed parameters
};

constexpr int NCHANS = 2;              ///< Number of audio channels (stereo)
constexpr double MAX_DELAY_TIME = 4000.0;  ///< Maximum delay time in milliseconds

/**
 * @brief Structure holding delay tap multipliers and their maximum base delay time
 */
struct TapMultipliers
{
    double maxBaseDelay;           ///< Maximum base delay time to prevent exceeding MAX_DELAY_TIME
    double values[kNumDelays];     ///< Array of multiplier values for each tap
    
    /**
     * @brief Construct a new Tap Multipliers object
     * @param m1 Multiplier for tap 0
     * @param m2 Multiplier for tap 1
     * @param m3 Multiplier for tap 2
     * @param m4 Multiplier for tap 3
     */
    TapMultipliers(double m1, double m2, double m3, double m4) :
        values { m1, m2, m3, m4 }
    {
        maxBaseDelay = MAX_DELAY_TIME / *std::max_element(std::begin(values), std::end(values));
    }
};

/**
 * @brief Multi-tap delay effect processor
 * 
 * This class implements a multi-tap delay effect with the following features:
 * - Multiple delay tap modes (Custom, Golden Ratio, Primes)
 * - Tempo-synchronized delay times
 * - Stereo processing with independent left/right control
 * - Parameter smoothing for click-free parameter changes
 */
class MultiTapDelay
{
public:
    /** @brief Constructor */
    MultiTapDelay();
    
    /** @brief Destructor */
    ~MultiTapDelay() {}

    /** @brief Tempo division values for sync mode */
    enum ETempoDivison
    {
        k64th = 0,   ///< 1 sixty fourth of a beat
        k32nd,       ///< 1 thirty second of a beat
        k16thT,      ///< 1 sixteenth note triplet
        k16th,       ///< 1 sixteenth note
        k16thD,      ///< 1 dotted sixteenth note
        k8thT,       ///< 1 eighth note triplet
        k8th,        ///< 1 eighth note
        k8thD,       ///< 1 dotted eighth note
        k4th,        ///< 1 quarter note (1 beat @ 4/4)
        k4thD,       ///< 1 dotted quarter note (1.5 beats @ 4/4)
        k2th,        ///< 2 beats @ 4/4
        kNumDivisions  ///< Total number of tempo divisions
    };

    /**
     * @brief Process a block of audio samples
     * @param inputs Input audio buffers
     * @param outputs Output audio buffers
     * @param nFrames Number of frames to process
     * @param tempo Current tempo in BPM (for sync mode)
     */
    void ProcessBlock(iplug::sample** inputs, iplug::sample** outputs, int nFrames, double tempo);
    
    /**
     * @brief Reset the delay line and internal state
     * @param sampleRate Current sample rate in Hz
     * @param blockSize Maximum expected block size
     * @param tempo Current tempo in BPM
     */
    void Reset(double sampleRate, int blockSize, double tempo);
    
    /**
     * @brief Set a parameter value
     * @param pGraphics Pointer to the graphics interface (for UI updates)
     * @param paramIdx Parameter index
     * @param value Normalized parameter value (0.0 to 1.0)
     * @param tempo Current tempo in BPM (for sync mode)
     */
    void SetParameters(iplug::igraphics::IGraphics* pGraphics, int paramIdx, double value, double tempo);
    
    /**
     * @brief Set all delay times from UI controls
     * @param pGraphics Pointer to the graphics interface
     */
    void SetDelayTimesCustom(iplug::igraphics::IGraphics* pGraphics);

    /**
     * @brief Get the current tap mode
     * @return Current tap mode
     */
    ETapModes getTapMode() { return mTapMode; }

    // Getters for individual delay times (for UI feedback)
    double GetDelayTime0() { return mAudioDelay[0].getParameters().leftDelay_mSec; }
    double GetDelayTime1() { return mAudioDelay[1].getParameters().leftDelay_mSec; }
    double GetDelayTime2() { return mAudioDelay[2].getParameters().leftDelay_mSec; }
    double GetDelayTime3() { return mAudioDelay[3].getParameters().leftDelay_mSec; }

private:
    /**
     * @brief Set delay times using the current multiplier set
     * @param pGraphics Pointer to the graphics interface
     */
    void SetDelayTimeByMultipliers(iplug::igraphics::IGraphics* pGraphics);
    
    /**
     * @brief Update UI controls with new delay times
     * @param pGraphics Pointer to the graphics interface
     * @param delayTimes Array of delay times [tap][channel]
     */
    void UpdateDelayTimecontrols(iplug::igraphics::IGraphics* pGraphics, const double delayTimes[][2]);
    
    /**
     * @brief Set delay time for a specific tap in sync mode
     * @param tempo Current tempo in BPM
     * @param delayIdx Delay tap index
     * @param channelIdx Channel index (0=left, 1=right)
     */
    void SetDelayTimeSynced(double tempo, EDelays delayIdx, int channelIdx);
    
    /**
     * @brief Set all delay times in sync mode
     * @param tempo Current tempo in BPM
     */
    void SetAllDelayTimeSynced(double tempo);
    
    /**
     * @brief Get the quarter note scalar for a tempo division
     * @param division Tempo division
     * @return Scalar value for the given division
     */
    static double GetQNScalar(ETempoDivison division)
    {
        static constexpr double scalars[kNumDivisions] = {
            0.0625,  // 1 sixty fourth of a beat
            0.125,   // 1 thirty second of a beat
            0.1667,  // 1 sixteenth note triplet
            0.25,    // 1 sixteenth note
            0.375,   // 1 dotted sixteenth note
            0.3333,  // 1 eighth note triplet
            0.5,     // 1 eighth note
            0.75,    // 1 dotted eighth note
            1.0,     // 1 quarter note (1 beat @ 4/4)
            1.5,     // 1 dotted quarter note (1.5 beats @ 4/4)
            2.0,     // 2 beats @ 4/4
        };

        return scalars[division];
    }

    /**
     * @brief Convert tap mode to normalized value for UI
     * @param value Tap mode to convert
     * @return Normalized value (0.0 to 1.0)
     */
    const double NormalizeValueFromTapMode(ETapModes value);

private:
    AudioDelay mAudioDelay[kNumDelays];                 ///< Audio delay lines
    AudioDelayParameters mAudioDelayParameters[kNumDelays];  ///< Parameters for each delay line
    ETapModes mTapMode = ETapModes::kCustomDelay;       ///< Current tap mode
    std::array<TapMultipliers, static_cast<size_t>(ETapModes::kNumTapModes)> mTapMultiplierSets;  ///< Multiplier sets for each tap mode
    
    // Helper variables
    ETempoDivison mCurrentDivision[kNumDelays][NCHANS];  ///< Current tempo division for each tap/channel
    static constexpr double GOLDEN_RATIO = 1.6180339887498948482;  ///< Golden ratio constant
    static constexpr double MSEC_PER_MIN = 60000.0;      ///< Milliseconds per minute (for BPM conversion)
    double mCurrentBpm = 1.0;                           ///< Current tempo in BPM
    bool mSyncEnabled = false;                          ///< Tempo sync enabled flag
    
    // Parameter smoothing
    WDL_TypedBuf<double> mModulationsData;              ///< Buffer for smoothed parameter values
    WDL_PtrList<double> mModulations;                   ///< List of parameter smoothers
    double mParamsToSmooth[kNumModulations];            ///< Target values for parameter smoothing
    iplug::LogParamSmooth<double, kNumModulations> mParameterSmoother;  ///< Parameter smoother
};