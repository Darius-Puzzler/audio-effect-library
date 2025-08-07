#include "IPlugLcrDelay.h"
#include "IPlug_include_in_plug_src.h"
#include "IControls.h"
#include "IconsFontaudio.h"

IPlugLcrDelay::IPlugLcrDelay(const InstanceInfo& info)
: iplug::Plugin(info, MakeConfig(kNumParams, kNumPresets))
{
    GetParam(kGain)->InitDouble("Gain", 0., 0., 100.0, 0.01, "%");
    GetParam(kDelayTime_mSec_L)->InitDouble("Left", 250.0, 0.0, 2000.0, 0.01, "ms");
    GetParam(kDelayTime_mSec_R)->InitDouble("Right", 250.0, 0.0, 2000.0, 0.01, "ms");
    GetParam(kDelayTime_mSec_C)->InitDouble("Center", 250.0, 0.0, 2000.0, 0.01, "ms");
    GetParam(kDelayFeedback_Pct_C)->InitDouble("Feedback", 0., 0., 100.0, 0.01, "%"); // 100.0% only for testing.
    GetParam(kHP_fc)->InitDouble("HP", 1000.0, 0.0, 20480.0, 0.01, "Hz", IParam::kFlagsNone, "", IParam::ShapePowCurve(5.0));
    GetParam(kLP_fc)->InitDouble("LP", 1000.0, 0.0, 20480.0, 0.01, "Hz", IParam::kFlagsNone, "", IParam::ShapePowCurve(5.0));
    GetParam(kFilterMode)->InitEnum("Filter Mode", 0, { "BYPASS", "HPF", "LPF", "ALL" });

#if IPLUG_EDITOR // http://bit.ly/2S64BDd
  mMakeGraphicsFunc = [&]() {
    return MakeGraphics(*this, PLUG_WIDTH, PLUG_HEIGHT, PLUG_FPS, GetScaleForScreen(PLUG_WIDTH, PLUG_HEIGHT));
  };
  
  mLayoutFunc = [&](IGraphics* pGraphics) {
    pGraphics->AttachCornerResizer(EUIResizerMode::Scale, false);
    pGraphics->AttachPanelBackground(COLOR_GRAY);
    pGraphics->LoadFont("Roboto-Regular", ROBOTO_FN);
    pGraphics->LoadFont("Fontaudio", FONTAUDIO_FN);
    const IRECT b = pGraphics->GetBounds();

    const IVStyle style{
      true, // Show label
      true, // Show value
      {
        DEFAULT_BGCOLOR, // Background
        DEFAULT_FGCOLOR, // Foreground
        DEFAULT_PRCOLOR, // Pressed
        COLOR_BLACK, // Frame
        DEFAULT_HLCOLOR, // Highlight
        DEFAULT_SHCOLOR, // Shadow
        COLOR_BLACK, // Extra 1
        DEFAULT_X2COLOR, // Extra 2
        DEFAULT_X3COLOR  // Extra 3
      }, // Colors
      IText(12.f, EAlign::Center) // Label text
    };

    const IText fontaudioText{ 32.f, "Fontaudio" };

    pGraphics->AttachControl(new ITextControl(b.GetGridCell(0, 3, 2), "Hello iPlug 2!", IText(50)));
    pGraphics->AttachControl(new ITextControl(b.GetGridCell(2, 6, 2), "LCR Delay!", IText(30)));
    pGraphics->AttachControl(new IVTabSwitchControl(b.GetGridCell(3, 3, 5).GetCentredInside(150).SubRectVertical(3, 1),
        kFilterMode, { ICON_FAU_FILTER_BYPASS, ICON_FAU_FILTER_HIGHPASS, ICON_FAU_FILTER_LOWPASS, ICON_FAU_FILTER_BANDPASS }, "Mode", style.WithValueText(fontaudioText), EVShape::EndsRounded));
    pGraphics->AttachControl(new IVKnobControl(b.GetGridCell(4, 3, 5).GetCentredInside(100), kGain));
    pGraphics->AttachControl(new IVKnobControl(b.GetGridCell(6, 3, 5).GetCentredInside(80), kDelayTime_mSec_L));
    pGraphics->AttachControl(new IVKnobControl(b.GetGridCell(7, 3, 5).GetCentredInside(80), kDelayTime_mSec_R));
    pGraphics->AttachControl(new IVKnobControl(b.GetGridCell(8, 3, 5).GetCentredInside(80), kDelayTime_mSec_C));
    pGraphics->AttachControl(new IVKnobControl(b.GetGridCell(9, 3, 5).GetCentredInside(80), kDelayFeedback_Pct_C));
    pGraphics->AttachControl(new IVKnobControl(b.GetGridCell(13, 3, 5).GetCentredInside(80), kHP_fc));
    pGraphics->AttachControl(new IVKnobControl(b.GetGridCell(14, 3, 5).GetCentredInside(80), kLP_fc));
  };
#endif
}

#if IPLUG_DSP
void IPlugLcrDelay::ProcessBlock(sample** inputs, sample** outputs, int nFrames)
{
  const double gain = GetParam(kGain)->Value() / 100.;
  const int nChans = NOutChansConnected();

  // LCR processBlock
  // no parameter smoothing implemented
  mLcrDelay.processBlock(inputs, outputs, nChans, nFrames);
  
  for (int s = 0; s < nFrames; s++) {
    for (int c = 0; c < nChans; c++) {
      outputs[c][s] = outputs[c][s] * gain; // apply gain here
    }
  }
}
void IPlugLcrDelay::OnReset()
{
    const double sampleRate = GetSampleRate();
    mLcrDelay.reset(sampleRate);
    mLcrDelay.createDelayBuffers(sampleRate, 2000.0);
}

// no parameter smoothing implemented
void IPlugLcrDelay::OnParamChange(int paramIdx)
{
    switch (paramIdx) {
    case kDelayFeedback_Pct_C:
        mLCRAudioDelayParameters.centerFeedback_Pct = GetParam(paramIdx)->Value(); // feedback_Pct is being calculated to [0.0, 1.0] in processBlock
        mLcrDelay.setParameters(mLCRAudioDelayParameters);
        break;
    case kDelayTime_mSec_C:
        mLCRAudioDelayParameters.centerDelay_mSec = GetParam(paramIdx)->Value();
        mLcrDelay.setParameters(mLCRAudioDelayParameters);
        break;
    case kDelayTime_mSec_L:
        mLCRAudioDelayParameters.leftDelay_mSec = GetParam(paramIdx)->Value();
        mLcrDelay.setParameters(mLCRAudioDelayParameters);
        break;
    case kDelayTime_mSec_R:
        mLCRAudioDelayParameters.rightDelay_mSec = GetParam(paramIdx)->Value();
        mLcrDelay.setParameters(mLCRAudioDelayParameters);
        break;
    case kHP_fc:
        mLcrDelay.SetFrequencyHP(GetParam(paramIdx)->Value());
        break;
    case kLP_fc:
        mLcrDelay.SetFrequencyLP(GetParam(paramIdx)->Value());
        break;
    case kFilterMode:
        const EFilterMode mode = static_cast<EFilterMode>(GetParam(paramIdx)->Value());
        switch (mode) {
        case EFilterMode::kBYPASS:
            mLcrDelay.SetFilterMode(mode, GetSampleRate());
            iplug::DBGMSG("Filter Mode = %d\n", (int)mode);
            break;
        case EFilterMode::kHPF:
            mLcrDelay.SetFilterMode(mode, GetSampleRate());
            iplug::DBGMSG("Filter Mode = %d\n", (int)mode);
            break;
        case EFilterMode::kLPF:
            mLcrDelay.SetFilterMode(mode, GetSampleRate());
            iplug::DBGMSG("Filter Mode = %d\n", (int)mode);
            break;
        case EFilterMode::kALL:
            mLcrDelay.SetFilterMode(mode, GetSampleRate());
            iplug::DBGMSG("Filter Mode = %d\n", (int)mode);
            break;
        default:
            break;
        }
        break;
    }
}
#endif
