#include "IPlugMultiTapDelay.h"
#include "IPlug_include_in_plug_src.h"
#include "IControls.h"

IPlugMultiTapDelay::IPlugMultiTapDelay(const InstanceInfo& info)
: iplug::Plugin(info, MakeConfig(kNumParams, kNumPresets))
{
	GetParam(kGain)->InitDouble("Gain", 0., 0., 100.0, 0.01, "%");

	GetParam(kTapMode)->InitEnum("Tap Modes", 0, { "Custom", "GoldenRatio", "Primes" });

	// delay 0
	GetParam(kDelayTime_mSec_Tap0_L)->InitDouble("Delay L", 250.0, 0.0, MAX_DELAY_TIME, 0.01, "ms");
	GetParam(kDelayTime_mSec_Tap0_R)->InitDouble("Delay R", 250.0, 0.0, MAX_DELAY_TIME, 0.01, "ms");
	GetParam(kDelayFeedback_Pct_Tap0_L)->InitDouble("Feedback L", 0., 0., 100.0, 0.01, "%"); // 100.0% only for testing.
	GetParam(kDelayFeedback_Pct_Tap0_R)->InitDouble("Feedback R", 0., 0., 100.0, 0.01, "%"); // 100.0% only for testing.
	GetParam(kDelayDivision0_L)->InitEnum("Time/Division", 8, { DELAY_TEMPODIV_VALIST });
	GetParam(kDelayDivision0_R)->InitEnum("Time/Division", 8, { DELAY_TEMPODIV_VALIST });

	// delay 1
	GetParam(kDelayTime_mSec_Tap1_L)->InitDouble("Delay L", 250.0, 0.0, MAX_DELAY_TIME, 0.01, "ms");
	GetParam(kDelayTime_mSec_Tap1_R)->InitDouble("Delay R", 250.0, 0.0, MAX_DELAY_TIME, 0.01, "ms");
	GetParam(kDelayDivision1_L)->InitEnum("Time/Division", 8, { DELAY_TEMPODIV_VALIST });
	GetParam(kDelayDivision1_R)->InitEnum("Time/Division", 8, { DELAY_TEMPODIV_VALIST });
	
	// delay 2
	GetParam(kDelayTime_mSec_Tap2_L)->InitDouble("Delay L", 250.0, 0.0, MAX_DELAY_TIME, 0.01, "ms");
	GetParam(kDelayTime_mSec_Tap2_R)->InitDouble("Delay R", 250.0, 0.0, MAX_DELAY_TIME, 0.01, "ms");
	GetParam(kDelayDivision2_L)->InitEnum("Time/Division", 8, { DELAY_TEMPODIV_VALIST });
	GetParam(kDelayDivision2_R)->InitEnum("Time/Division", 8, { DELAY_TEMPODIV_VALIST });

	// delay 3
	GetParam(kDelayTime_mSec_Tap3_L)->InitDouble("Delay L", 250.0, 0.0, MAX_DELAY_TIME, 0.01, "ms");
	GetParam(kDelayTime_mSec_Tap3_R)->InitDouble("Delay R", 250.0, 0.0, MAX_DELAY_TIME, 0.01, "ms");
	GetParam(kDelayDivision3_L)->InitEnum("Time/Division", 8, { DELAY_TEMPODIV_VALIST });
	GetParam(kDelayDivision3_R)->InitEnum("Time/Division", 8, { DELAY_TEMPODIV_VALIST });

	//GetParam(kWetLevel_dB)->InitDouble("Wet Level", -3.0, -60.0, 12.0, 0.5, "dB");
	//GetParam(kDryLevel_dB)->InitDouble("Dry Level", -3.0, -60.0, 12.0, 0.5, "dB");
	GetParam(kDelaySync)->InitBool("Sync", 0);

#if IPLUG_EDITOR // http://bit.ly/2S64BDd
  mMakeGraphicsFunc = [&]() {
    return MakeGraphics(*this, PLUG_WIDTH, PLUG_HEIGHT, PLUG_FPS, GetScaleForScreen(PLUG_WIDTH, PLUG_HEIGHT));
  };
  
  mLayoutFunc = [&](IGraphics* pGraphics) {
    pGraphics->AttachCornerResizer(EUIResizerMode::Scale, false);
    pGraphics->AttachPanelBackground(COLOR_GRAY);
    pGraphics->LoadFont("Roboto-Regular", ROBOTO_FN);
    const IRECT b = pGraphics->GetBounds();

	// text size for delay controls
	const IText textStyle(16);
	IVStyle style;
	style.labelText = textStyle;

    pGraphics->AttachControl(new ITextControl(b.GetGridCell(0, 6, 4), "Hello iPlug 2 Multitab Delay!", IText(15)));
	pGraphics->AttachControl(new IVKnobControl(b.GetGridCell(1, 6, 4).GetCentredInside(80), kDelayFeedback_Pct_Tap0_L));
	pGraphics->AttachControl(new IVKnobControl(b.GetGridCell(2, 6, 4).GetCentredInside(80), kDelayFeedback_Pct_Tap0_R));
    pGraphics->AttachControl(new IVKnobControl(b.GetGridCell(3, 6, 4).GetCentredInside(80), kGain));
    
	pGraphics->AttachControl(new IVMenuButtonControl(b.GetGridCell(5, 6, 4).GetCentredInside(80), kTapMode));
	pGraphics->AttachControl(new IVSlideSwitchControl(b.GetGridCell(6, 6, 4).GetCentredInside(80), kDelaySync));

	// background panel to visually group each delay's controls
	if (pGraphics)
	{
		// Create a style for the group
		IVStyle groupStyle = style.WithColor(kBG, COLOR_GRAY.WithOpacity(0.2f))
			.WithFrameThickness(1.0f)
			.WithRoundness(5.0f);

		// Create a group background
		auto groupBounds0 = b.GetGridCell(2, 0, 6, 1).GetPadded(-5); // Adjust grid cell and padding as needed
		auto groupBounds1 = b.GetGridCell(3, 0, 6, 1).GetPadded(-5); // Adjust grid cell and padding as needed
		auto groupBounds2 = b.GetGridCell(4, 0, 6, 1).GetPadded(-5); // Adjust grid cell and padding as needed
		auto groupBounds3 = b.GetGridCell(5, 0, 6, 1).GetPadded(-5); // Adjust grid cell and padding as needed
		pGraphics->AttachControl(new IVPanelControl(groupBounds0, "", groupStyle), -1, "GROUP_BG0");
		pGraphics->AttachControl(new IVPanelControl(groupBounds1, "", groupStyle), -1, "GROUP_BG1");
		pGraphics->AttachControl(new IVPanelControl(groupBounds2, "", groupStyle), -1, "GROUP_BG2");
		pGraphics->AttachControl(new IVPanelControl(groupBounds3, "", groupStyle), -1, "GROUP_BG3");
	}

	pGraphics->AttachControl(new ITextControl(b.GetGridCell(8, 6, 4), "Delay Tap 1", IText(20)));
	pGraphics->AttachControl(new IVKnobControl(b.GetGridCell(9, 6, 4).GetCentredInside(80), kDelayTime_mSec_Tap0_L, "Delay 1", style), -1, "delaymaster");
	pGraphics->AttachControl(new IVKnobControl(b.GetGridCell(9, 6, 4).GetCentredInside(80), kDelayDivision0_L, "Time/Division", style));
	pGraphics->AttachControl(new IVKnobControl(b.GetGridCell(10, 6, 4).GetCentredInside(80), kDelayTime_mSec_Tap0_R, "Delay 1", style), -1, "delaymaster");
	pGraphics->AttachControl(new IVKnobControl(b.GetGridCell(10, 6, 4).GetCentredInside(80), kDelayDivision0_R, "Time/Division", style));

	pGraphics->AttachControl(new ITextControl(b.GetGridCell(12, 6, 4), "Delay Tap 2", IText(20)));
	pGraphics->AttachControl(new IVKnobControl(b.GetGridCell(13, 6, 4).GetCentredInside(80), kDelayTime_mSec_Tap1_L, "Delay 2", style), -1, "delayslave");
	pGraphics->AttachControl(new IVKnobControl(b.GetGridCell(13, 6, 4).GetCentredInside(80), kDelayDivision1_L, "Time/Division", style));
	pGraphics->AttachControl(new IVKnobControl(b.GetGridCell(14, 6, 4).GetCentredInside(80), kDelayTime_mSec_Tap1_R, "Delay 2", style), -1, "delayslave");
	pGraphics->AttachControl(new IVKnobControl(b.GetGridCell(14, 6, 4).GetCentredInside(80), kDelayDivision1_R, "Time/Division", style));

	pGraphics->AttachControl(new ITextControl(b.GetGridCell(16, 6, 4), "Delay Tap 3", IText(20)));
	pGraphics->AttachControl(new IVKnobControl(b.GetGridCell(17, 6, 4).GetCentredInside(80), kDelayTime_mSec_Tap2_L, "Delay 3", style), -1, "delayslave");
	pGraphics->AttachControl(new IVKnobControl(b.GetGridCell(17, 6, 4).GetCentredInside(80), kDelayDivision2_L, "Time/Division", style));
	pGraphics->AttachControl(new IVKnobControl(b.GetGridCell(18, 6, 4).GetCentredInside(80), kDelayTime_mSec_Tap2_R, "Delay 3", style), -1, "delayslave");
	pGraphics->AttachControl(new IVKnobControl(b.GetGridCell(18, 6, 4).GetCentredInside(80), kDelayDivision2_R, "Time/Division", style));

	pGraphics->AttachControl(new ITextControl(b.GetGridCell(20, 6, 4), "Delay Tap 4", IText(20)));
	pGraphics->AttachControl(new IVKnobControl(b.GetGridCell(21, 6, 4).GetCentredInside(80), kDelayTime_mSec_Tap3_L, "Delay 4", style), -1, "delayslave");
	pGraphics->AttachControl(new IVKnobControl(b.GetGridCell(21, 6, 4).GetCentredInside(80), kDelayDivision3_L, "Time/Division", style));
	pGraphics->AttachControl(new IVKnobControl(b.GetGridCell(22, 6, 4).GetCentredInside(80), kDelayTime_mSec_Tap3_R, "Delay 4", style), -1, "delayslave");
	pGraphics->AttachControl(new IVKnobControl(b.GetGridCell(22, 6, 4).GetCentredInside(80), kDelayDivision3_R, "Time/Division", style));
  };
#endif
}

#if IPLUG_DSP
void IPlugMultiTapDelay::ProcessBlock(sample** inputs, sample** outputs, int nFrames)
{
	const int nChans = NOutChansConnected();

	// all audio is processed in mMultiTapDelay
	mMultiTapDelay.ProcessBlock(inputs, outputs, nFrames, mTimeInfo.mTempo);
}

void IPlugMultiTapDelay::OnReset()
{
	const double sampleRate = GetSampleRate();
	const int blockSize = GetBlockSize();
	const double tempo = mTimeInfo.mTempo;
	mMultiTapDelay.Reset(sampleRate, blockSize,	tempo);
}

void IPlugMultiTapDelay::OnParamChange(int paramIdx)
{
	IGraphics* pGraphics = GetUI();
	mMultiTapDelay.SetParameters(pGraphics, paramIdx, GetParam(paramIdx)->Value(), mTimeInfo.mTempo);
}

void IPlugMultiTapDelay::OnParamChangeUI(int paramIdx, EParamSource source)
{
	switch (paramIdx) {
	case kDelaySync:
		if (auto pGraphics = GetUI())
		{
			const auto sync = GetParam(paramIdx)->Bool();
			// sync on - hide all all delay times, disable tap mode
			for (int i = 0; i < 2 * kNumDelays; i++)
			{
				pGraphics->HideControl(static_cast<EDelays>(i), sync);
			}

			pGraphics->DisableControl(kTapMode, sync);

			// sync off - hide all delay divisions
			for (int i = static_cast<int>(kDelayDivision0_L); i < static_cast<int>(kDelayDivision3_R) + 1; i++)
			{
				pGraphics->HideControl(static_cast<EParams>(i), !sync);
			}
		}
		break;
	case kTapMode:
		// enable all tap controls only in custom mode, else disable tap 1-3
		if (auto pGraphics = GetUI())
		{
			const ETapModes tapMode = static_cast<ETapModes>(GetParam(paramIdx)->Value());
			bool mode = true;

			switch (tapMode) {
			case ETapModes::kCustomDelay:
				mode = false;
				break;
			default:
				mode = true;
				break;
			}
			for (int i = 2; i < 2 * kNumDelays; i++)
			{
				pGraphics->DisableControl(static_cast<EParams>(i), mode);
			}
		}
		break;
	}
}
#endif
