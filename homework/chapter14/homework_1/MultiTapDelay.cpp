#include "MultiTapDelay.h"

MultiTapDelay::MultiTapDelay() :
	mTapMultiplierSets {{
			// Custom Mode
			{ 1.0, 1.0, 1.0, 1.0 },
			// Golden Ration Mode
			{ 1.0, GOLDEN_RATIO, 1.5 * GOLDEN_RATIO, 2.0 * GOLDEN_RATIO },
			// Primes Mode
			{ 1.0, 2.0, 3.0, 5.0 }
		}
}
{
	static_assert(std::tuple_size<decltype(mTapMultiplierSets)>::value == static_cast<size_t>(ETapModes::kNumTapModes),
		"Number of multiplier sets must match number of tap modes");
}

/**
 * @brief Process a block of audio samples
 * 
 * This function processes a block of audio samples, applying the delay effect
 * and updating the output buffers.
 * 
 * @param inputs Input audio buffers (left and right channels)
 * @param outputs Output audio buffers (left and right channels)
 * @param nFrames Number of audio frames to process
 * @param tempo Current tempo in BPM
 */
void MultiTapDelay::ProcessBlock(iplug::sample** inputs, iplug::sample** outputs, int nFrames, double tempo)
{
	// check if host bpm changed and update delay times
	if (mCurrentBpm != tempo && mSyncEnabled)
	{
		SetAllDelayTimeSynced(tempo);
		mCurrentBpm = tempo;
	}

	// set outputs zero to avoid fragments
	memset(outputs[0], 0, nFrames * sizeof(iplug::sample));
	memset(outputs[1], 0, nFrames * sizeof(iplug::sample));

	// smooth the parameters
	mParameterSmoother.ProcessBlock(mParamsToSmooth, mModulations.GetList(), nFrames);

	// pointer to the smoothed master gain
	const iplug::sample* smoothedGainBuffer = mModulations.GetList()[kModGain];
	// pointer to the smoothed feedback L and R
	const double* feedbackbuffer_L = mModulations.GetList()[kModFeedback_L];
	const double* feedbackbuffer_R = mModulations.GetList()[kModFeedback_R];

	for (int s = 0; s < nFrames; s++)
	{
		float frame[2] = { static_cast<float>(inputs[0][s]), static_cast<float>(inputs[1][s]) };
		float outL = 0.0, outR = 0.0;

		// for each frame call the delay processAudioFrame and add it to the outputs
		for (int i = 0; i < kNumDelays; i++)
		{
			const double* delayBuffer_L = mModulations.GetList()[2 * i];
			const double* delayBuffer_R = mModulations.GetList()[2 * i + 1];

			// only update when parameter values changed
			if (mAudioDelayParameters[i].leftDelay_mSec != delayBuffer_L[0] ||
				mAudioDelayParameters[i].rightDelay_mSec != delayBuffer_R[0] ||
				mAudioDelayParameters[i].leftFeedback_Pct != feedbackbuffer_L[0] ||
				mAudioDelayParameters[i].rightFeedback_Pct != feedbackbuffer_R[0])
			{
				mAudioDelayParameters[i].leftDelay_mSec = delayBuffer_L[0];
				mAudioDelayParameters[i].rightDelay_mSec = delayBuffer_R[0];
				mAudioDelayParameters[i].leftFeedback_Pct = feedbackbuffer_L[0];
				mAudioDelayParameters[i].rightFeedback_Pct = feedbackbuffer_R[0];
				mAudioDelay[i].setParameters(mAudioDelayParameters[i]);
			}

			// 2 channel stereo processing, if 1 channel, feedback is taken from left channel
			float tapOut[2];
			mAudioDelay[i].processAudioFrame(frame, tapOut, 2, 2); // input frame will be output frame
			outL += tapOut[0] * smoothedGainBuffer[s];
			outR += tapOut[1] * smoothedGainBuffer[s];
		}
		// add the processed frame to the output
		outputs[0][s] = outL;
		outputs[1][s] = outR;
	}
}

/**
 * @brief Reset the delay effect
 * 
 * This function resets the delay effect, initializing the delay buffers and
 * setting the initial delay times.
 * 
 * @param sampleRate Current sample rate
 * @param blockSize Current block size
 * @param tempo Current tempo in BPM
 */
void MultiTapDelay::Reset(double sampleRate, int blockSize, double tempo) // only read the read only parameter pointer
{
	for (int i = 0; i < kNumDelays; i++)
	{
		if (i > 0)
		{
			// setting the dry level for delay1 - delay3 to -100dB
			mAudioDelayParameters[i].dryLevel_dB = -100.0f;
		}

		mAudioDelay[i].reset(sampleRate);
		mAudioDelay[i].createDelayBuffers(sampleRate, MAX_DELAY_TIME);
		mAudioDelay[i].setParameters(mAudioDelayParameters[i]);
	}

	// smoothing buffer
	mModulationsData.Resize(blockSize * kNumModulations);
	mModulations.Empty();

	for (int i = 0; i < kNumModulations; i++)
	{
		mModulations.Add(mModulationsData.Get() + (blockSize * i));
	}

	mCurrentBpm = tempo; // set current bpm from host
}

/**
 * @brief Set the delay parameters
 * 
 * This function sets the delay parameters, updating the internal state and
 * the UI controls.
 * 
 * @param pGraphics Pointer to the graphics interface
 * @param paramIdx Index of the parameter to set
 * @param value New value of the parameter
 * @param tempo Current tempo in BPM
 */
void MultiTapDelay::SetParameters(iplug::igraphics::IGraphics* pGraphics, int paramIdx, double value, double tempo)
{
	switch (paramIdx) {
	case kGain:
		mParamsToSmooth[kModGain] = value / 100.0;
		break;
	// Tap 0 - 3 Delay Times
	case kDelayTime_mSec_Tap0_L:
		if (mTapMode == ETapModes::kCustomDelay)
		{
			mParamsToSmooth[kModDelayTime0_L] = value;
		}
		else if (mTapMode == ETapModes::kGoldenRatio)
		{
			SetDelayTimeByMultipliers(pGraphics);
		}
		else if (mTapMode == ETapModes::kPrimes)
		{
			SetDelayTimeByMultipliers(pGraphics);
		}
		break;
	case kDelayTime_mSec_Tap0_R:
		if (mTapMode == ETapModes::kCustomDelay)
		{
			mParamsToSmooth[kModDelayTime0_R] = value;
		}
		else if (mTapMode == ETapModes::kGoldenRatio)
		{
			SetDelayTimeByMultipliers(pGraphics);
		}
		else if (mTapMode == ETapModes::kPrimes)
		{
			SetDelayTimeByMultipliers(pGraphics);
		}
		break;
	case kDelayTime_mSec_Tap1_L:
		mParamsToSmooth[kModDelayTime1_L] = value;
		break;
	case kDelayTime_mSec_Tap1_R:

		mParamsToSmooth[kModDelayTime1_R] = value;
		break;
	case kDelayTime_mSec_Tap2_L:
		mParamsToSmooth[kModDelayTime2_L] = value;
		break;
	case kDelayTime_mSec_Tap2_R:
		mParamsToSmooth[kModDelayTime2_R] = value;
		break;
	case kDelayTime_mSec_Tap3_L:
		mParamsToSmooth[kModDelayTime3_L] = value;
		break;
	case kDelayTime_mSec_Tap3_R:
		mParamsToSmooth[kModDelayTime3_R] = value;
		break;
	// Tap 0 Feedback
	case kDelayFeedback_Pct_Tap0_L:
		mParamsToSmooth[kModFeedback_L] = value;
		break;
	case kDelayFeedback_Pct_Tap0_R:
		mParamsToSmooth[kModFeedback_R] = value;
		break;
	// set the tap mode, the relationship between the 4 taps
	case kTapMode:
		switch (static_cast<ETapModes>(value)) {
		case ETapModes::kCustomDelay:
			mTapMode = ETapModes::kCustomDelay;
			break;
		case ETapModes::kGoldenRatio:
			mTapMode = ETapModes::kGoldenRatio;
			SetDelayTimeByMultipliers(pGraphics);
			break;
		case ETapModes::kPrimes:
			mTapMode = ETapModes::kPrimes;
			SetDelayTimeByMultipliers(pGraphics);
			break;
		default:
			mTapMode = ETapModes::kCustomDelay;
			break;
		}
		break;
	// sync to host bpm
	case kDelaySync:
		if (value)
		{
			mSyncEnabled = true;
			// get each delay division and set the times
			for (int i = 0; i < kNumDelays; i++)
			{
				auto pControl_L = pGraphics->GetControlWithParamIdx(static_cast<EParams>(kDelayDivision0_L + i));
				auto pControl_R = pGraphics->GetControlWithParamIdx(static_cast<EParams>(kDelayDivision0_R + i));
				if (pControl_L && pControl_R)
				{
					mCurrentDivision[i][0] = static_cast<ETempoDivison>(pControl_L->GetValue() * (kNumDivisions - 1));
					mCurrentDivision[i][1] = static_cast<ETempoDivison>(pControl_R->GetValue() * (kNumDivisions - 1));
				}
			}
			SetAllDelayTimeSynced(tempo);
		}
		else
		{
			mSyncEnabled = false;
			SetDelayTimesCustom(pGraphics);
		}
		break;
	// in sync mode, set the individual delay time divisions
	case kDelayDivision0_L:
		if (mSyncEnabled && mCurrentDivision[kDelay0][0] != static_cast<ETempoDivison>(value))
		{
			mCurrentDivision[kDelay0][0] = static_cast<ETempoDivison>(value);
			SetDelayTimeSynced(tempo, kDelay0, 0);
		}
		break;
	case kDelayDivision0_R:
		if (mSyncEnabled && mCurrentDivision[kDelay0][1] != static_cast<ETempoDivison>(value))
		{
			mCurrentDivision[kDelay0][1] = static_cast<ETempoDivison>(value);
			SetDelayTimeSynced(tempo, kDelay0, 1);
		}
		break;
	case kDelayDivision1_L:
		if (mSyncEnabled && mCurrentDivision[kDelay1][0] != static_cast<ETempoDivison>(value))
		{
			mCurrentDivision[kDelay1][0] = static_cast<ETempoDivison>(value);
			SetDelayTimeSynced(tempo, kDelay1, 0);
		}
		break;
	case kDelayDivision1_R:
		if (mSyncEnabled && mCurrentDivision[kDelay1][1] != static_cast<ETempoDivison>(value))
		{
			mCurrentDivision[kDelay1][1] = static_cast<ETempoDivison>(value);
			SetDelayTimeSynced(tempo, kDelay1, 1);
		}
		break;
	case kDelayDivision2_L:
		if (mSyncEnabled && mCurrentDivision[kDelay2][0] != static_cast<ETempoDivison>(value))
		{
			mCurrentDivision[kDelay2][0] = static_cast<ETempoDivison>(value);
			SetDelayTimeSynced(tempo, kDelay2, 0);
		}
		break;
	case kDelayDivision2_R:
		if (mSyncEnabled && mCurrentDivision[kDelay2][1] != static_cast<ETempoDivison>(value))
		{
			mCurrentDivision[kDelay2][1] = static_cast<ETempoDivison>(value);
			SetDelayTimeSynced(tempo, kDelay2, 1);
		}
		break;
	case kDelayDivision3_L:
		if (mSyncEnabled && mCurrentDivision[kDelay3][0] != static_cast<ETempoDivison>(value))
		{
			mCurrentDivision[kDelay3][0] = static_cast<ETempoDivison>(value);
			SetDelayTimeSynced(tempo, kDelay3, 0);
		}
		break;
	case kDelayDivision3_R:
		if (mSyncEnabled && mCurrentDivision[kDelay3][1] != static_cast<ETempoDivison>(value))
		{
			mCurrentDivision[kDelay3][1] = static_cast<ETempoDivison>(value);
			SetDelayTimeSynced(tempo, kDelay3, 1);
		}
		break;
	default:
		break;
	}
}

/**
 * @brief Get all delay times from the UI and set them for each delay
 * 
 * This function is called when the sync or tap mode changes.
 * 
 * @param pGraphics Pointer to the graphics interface
 */
void MultiTapDelay::SetDelayTimesCustom(iplug::igraphics::IGraphics* pGraphics)
{
	if (pGraphics)
	{
		for (int i = 0; i < kNumDelays; i++)
		{
			auto pControl_L = pGraphics->GetControlWithParamIdx(static_cast<EParams>(kDelayTime_mSec_Tap0_L + (2 * i)));
			auto pControl_R = pGraphics->GetControlWithParamIdx(static_cast<EParams>(kDelayTime_mSec_Tap0_R + (2 * i)));
			if (pControl_L && pControl_R)
			{
				mParamsToSmooth[2 * i] = pControl_L->GetValue() * MAX_DELAY_TIME;;
				mParamsToSmooth[2 * i + 1] = pControl_R->GetValue() * MAX_DELAY_TIME;;
			}
		}
	}
}

/**
 * @brief Update the delay time controls on the UI
 * 
 * This function updates the delay time controls on the UI with the new delay times.
 * 
 * @param pGraphics Pointer to the graphics interface
 * @param delayTimes New delay times for each tap and channel
 */
void MultiTapDelay::UpdateDelayTimecontrols(iplug::igraphics::IGraphics* pGraphics, const double delayTimes[][2])
{
	if (!pGraphics) return;

	// only update tap 1 - 3
	for (int i = 1; i < kNumDelays; i++)
	{
		auto pControl_L = pGraphics->GetControlWithParamIdx(static_cast<EParams>(kDelayTime_mSec_Tap0_L + (2 * i))); // channels increase in (2 * i) steps
		auto pControl_R = pGraphics->GetControlWithParamIdx(static_cast<EParams>(kDelayTime_mSec_Tap0_R + (2 * i)));
		if (pControl_L && pControl_R)
		{
			pControl_L->SetValueFromDelegate(delayTimes[i][0] / MAX_DELAY_TIME);
			pControl_L->SetDirty();
			pControl_R->SetValueFromDelegate(delayTimes[i][1] / MAX_DELAY_TIME);
			pControl_R->SetDirty();
		}
	}
}

/**
 * @brief Set the delay time for a specific tap when tempo sync is enabled
 * 
 * @param tempo Current tempo in BPM
 * @param delayIdx Index of the delay tap to update
 * @param channelIdx Channel index (0 = left, 1 = right)
 */
void MultiTapDelay::SetDelayTimeSynced(double tempo, EDelays delayIdx, int channelIdx)
{
	const double beatPeriod_ms = MSEC_PER_MIN / tempo;
	const double qnScalar = GetQNScalar(mCurrentDivision[delayIdx][channelIdx]);
	mParamsToSmooth[2 * delayIdx + channelIdx] = beatPeriod_ms * qnScalar;
}

/**
 * @brief Set all delay times using the current multiplier set
 * 
 * This function calculates delay times for all taps based on the base delay time
 * and the current tap mode's multiplier set. It updates both the internal
 * parameters and the UI controls.
 * 
 * @param pGraphics Pointer to the graphics interface for UI updates
 */
void MultiTapDelay::SetDelayTimeByMultipliers(iplug::igraphics::IGraphics* pGraphics)
{
	double delayTimes[kNumDelays][NCHANS] = { 0 }; // for holding the calculated delay times

	if (pGraphics)
	{
		// Get pointers to the base delay time controls
		iplug::igraphics::IControl* pControlBaseTime[NCHANS] = {
			pGraphics->GetControlWithParamIdx(kDelayTime_mSec_Tap0_L),
			pGraphics->GetControlWithParamIdx(kDelayTime_mSec_Tap0_R) };

		if (pControlBaseTime[0] && pControlBaseTime[1])
		{
			// Calculate base delay times, clamped to prevent exceeding max delay time
			double baseDelayTimes[NCHANS] = {
				std::clamp(pControlBaseTime[0]->GetValue() * MAX_DELAY_TIME, 0.0, mTapMultiplierSets[(int)mTapMode].maxBaseDelay),
				std::clamp(pControlBaseTime[1]->GetValue() * MAX_DELAY_TIME, 0.0, mTapMultiplierSets[(int)mTapMode].maxBaseDelay) };

			// Calculate delay times for all taps and channels
			for (int tap = 0; tap < kNumDelays; tap++)
			{
				for (int chan = 0; chan < NCHANS; chan++)
				{
					// Apply multiplier to get final delay time
					delayTimes[tap][chan] = baseDelayTimes[chan] * mTapMultiplierSets[(int)mTapMode].values[tap];
					
					// Update smoothed parameter value
					mParamsToSmooth[kModDelayTime0_L + 2 * tap + chan] = delayTimes[tap][chan];
				}
			}
			
			// Update the UI with new delay times
			UpdateDelayTimecontrols(pGraphics, delayTimes);
		}
	}
}

/**
 * @brief Update all delay times in sync mode
 * 
 * This function is called when the tempo changes and sync mode is enabled.
 * It updates all delay times based on the current tempo and division settings.
 * 
 * @param tempo Current tempo in BPM
 */
void MultiTapDelay::SetAllDelayTimeSynced(double tempo)
{
	for (int i = 0; i < kNumDelays; i++)
	{
		const double beatPeriod_ms = MSEC_PER_MIN / tempo;
		const double qnScalar_L = GetQNScalar(mCurrentDivision[i][0]);
		const double qnScalar_R = GetQNScalar(mCurrentDivision[i][1]);
		
		// Update left and right channels
		mParamsToSmooth[2 * i] = beatPeriod_ms * qnScalar_L;
		mParamsToSmooth[2 * i + 1] = beatPeriod_ms * qnScalar_R;
	}
}

/**
 * @brief Calculate the normalized value from ETapModes for updating the parameter on ui
 * 
 * @param value ETapModes value to normalize
 * @return Normalized value
 */
const double MultiTapDelay::NormalizeValueFromTapMode(ETapModes value)
{
	return static_cast<double>(value) / (((double)ETapModes::kNumTapModes - 1.0)); // x min is always 0
}
