#include "IPlugAnalogModelingDelay.h"
#include "IPlug_include_in_plug_src.h"
#include "IControls.h"

IPlugAnalogModelingDelay::IPlugAnalogModelingDelay(const InstanceInfo& info)
: iplug::Plugin(info, MakeConfig(kNumParams, kNumPresets))
{
  GetParam(kGain)->InitDouble("Gain", 0., 0., 100.0, 0.01, "%");
  GetParam(kDelayTime_mSec_L)->InitDouble("Delay L", 250.0, 0.0, 2000.0, 0.01, "ms");
  GetParam(kDelayTime_mSec_R)->InitDouble("Delay R", 250.0, 0.0, 2000.0, 0.01, "ms");
  GetParam(kDelayFeedback_Pct_L)->InitDouble("Feedback L", 0., 0., 100.0, 0.01, "%"); // 100.0% only for testing.
  GetParam(kDelayFeedback_Pct_R)->InitDouble("Feedback R", 0., 0., 100.0, 0.01, "%"); // 100.0% only for testing.

#if IPLUG_EDITOR // http://bit.ly/2S64BDd
  mMakeGraphicsFunc = [&]() {
    return MakeGraphics(*this, PLUG_WIDTH, PLUG_HEIGHT, PLUG_FPS, GetScaleForScreen(PLUG_WIDTH, PLUG_HEIGHT));
  };
  
  mLayoutFunc = [&](IGraphics* pGraphics) {
    pGraphics->AttachCornerResizer(EUIResizerMode::Scale, false);
    pGraphics->AttachPanelBackground(COLOR_GRAY);
    pGraphics->LoadFont("Roboto-Regular", ROBOTO_FN);
    const IRECT b = pGraphics->GetBounds();
    pGraphics->AttachControl(new ITextControl(b.GetGridCell(0, 5, 2), "Hello iPlug 2!", IText(50)));
    pGraphics->AttachControl(new ITextControl(b.GetGridCell(2, 5, 2), "Analog Modeling Delay!", IText(30)));
    pGraphics->AttachControl(new IVKnobControl(b.GetGridCell(4, 3, 5).GetCentredInside(80), kGain));
    pGraphics->AttachControl(new IVKnobControl(b.GetGridCell(6, 3, 5).GetCentredInside(80), kDelayTime_mSec_L));
    pGraphics->AttachControl(new IVKnobControl(b.GetGridCell(7, 3, 5).GetCentredInside(80), kDelayTime_mSec_R));
    pGraphics->AttachControl(new IVKnobControl(b.GetGridCell(8, 3, 5).GetCentredInside(80), kDelayFeedback_Pct_L));
    pGraphics->AttachControl(new IVKnobControl(b.GetGridCell(9, 3, 5).GetCentredInside(80), kDelayFeedback_Pct_R));
  };
#endif
}

#if IPLUG_DSP
void IPlugAnalogModelingDelay::ProcessBlock(sample** inputs, sample** outputs, int nFrames)
{
  const double gain = GetParam(kGain)->Value() / 100.;
  const int nChans = NOutChansConnected();

  mAnalogModelingDelay.processBlock(inputs, outputs, 2, nFrames);
  
  for (int s = 0; s < nFrames; s++) {
    for (int c = 0; c < nChans; c++) {
      outputs[c][s] = outputs[c][s] * gain;
    }
  }
}
void IPlugAnalogModelingDelay::OnReset()
{
    const double sampleRate = GetSampleRate();
    mAnalogModelingDelay.reset(sampleRate);
    mAnalogModelingDelay.createDelayBuffers(sampleRate, 2000.0);
}
void IPlugAnalogModelingDelay::OnParamChangeUI(int paramIdx, EParamSource source)
{
}
void IPlugAnalogModelingDelay::OnParamChange(int paramIdx)
{
    switch (paramIdx) {
    case kDelayTime_mSec_L:
        mAudioDelayParameters.leftDelay_mSec = GetParam(paramIdx)->Value();
        mAnalogModelingDelay.setParameters(mAudioDelayParameters);
        break;
    case kDelayTime_mSec_R:
        mAudioDelayParameters.rightDelay_mSec = GetParam(paramIdx)->Value();
        mAnalogModelingDelay.setParameters(mAudioDelayParameters);
        break;
    case kDelayFeedback_Pct_L:
        mAudioDelayParameters.leftFeedback_Pct = GetParam(paramIdx)->Value();
        mAnalogModelingDelay.setParameters(mAudioDelayParameters);
        break;
    case kDelayFeedback_Pct_R:
        mAudioDelayParameters.rightFeedback_Pct = GetParam(paramIdx)->Value();
        mAnalogModelingDelay.setParameters(mAudioDelayParameters);
        break;
    default:
        break;
    }
}
#endif
