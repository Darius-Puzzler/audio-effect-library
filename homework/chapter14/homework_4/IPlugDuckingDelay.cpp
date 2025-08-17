#include "IPlugDuckingDelay.h"
#include "IPlug_include_in_plug_src.h"
#include "IControls.h"

IPlugDuckingDelay::IPlugDuckingDelay(const InstanceInfo& info)
: iplug::Plugin(info, MakeConfig(kNumParams, kNumPresets))
{
  GetParam(kGain)->InitDouble("Gain", 0., 0., 100.0, 0.01, "%");
  GetParam(kGainSC)->InitDouble("Gain SC", 0., 0., 100.0, 0.01, "%");
  GetParam(kDelayTimeLeft)->InitDouble("Delay L", 250.0, 0.0, 2000.0, 0.01, "ms");
  GetParam(kDelayTimeRight)->InitDouble("Delay R", 250.0, 0.0, 2000.0, 0.01, "ms");
  GetParam(kDelayFeedbackLeft)->InitDouble("FB L", 0., 0., 100.0, 0.01, "%");
  GetParam(kDelayFeedbackRight)->InitDouble("FB R", 0., 0., 100.0, 0.01, "%");

  GetParam(kEnvAttack)->InitDouble("Attack", 5.0, 1.0, 250.0, 0.01, "ms");
  GetParam(kEnvRelease)->InitDouble("Release", 150.0, 1.0, 500.0, 0.01, "ms");

  GetParam(kThresh_dB)->InitDouble("Threshold", -24.0, -60.0, -3.0, 0.01, "dB");
  GetParam(kSensitivity)->InitDouble("Sensitivity", 1.0, 0.25, 5.0, 0.01, "");
  GetParam(kWetMin)->InitDouble("Wet Min", -12.0, -60.0, -3.0, 0.01, "dB");
  GetParam(kWetMax)->InitDouble("Wet Max", -3.0, -24.0, -3.0, 0.01, "dB");

#if IPLUG_EDITOR // http://bit.ly/2S64BDd
  mMakeGraphicsFunc = [&]() {
    return MakeGraphics(*this, PLUG_WIDTH, PLUG_HEIGHT, PLUG_FPS, GetScaleForScreen(PLUG_WIDTH, PLUG_HEIGHT));
  };
  
  mLayoutFunc = [&](IGraphics* pGraphics) {
    pGraphics->AttachCornerResizer(EUIResizerMode::Scale, false);
    pGraphics->AttachPanelBackground(COLOR_GRAY);
    pGraphics->LoadFont("Roboto-Regular", ROBOTO_FN);
    const IRECT b = pGraphics->GetBounds();
    pGraphics->AttachControl(new ITextControl(b.GetGridCell(1, 3, 8).SubRectVertical(2, 0), "Hello iPlug 2!", IText(15)));
    pGraphics->AttachControl(new ITextControl(b.GetGridCell(1, 3, 8).SubRectVertical(2, 1), "Ducking Delay!", IText(15)));
    pGraphics->AttachControl(new IVKnobControl(b.GetGridCell(7, 3, 8).GetCentredInside(100), kGain));

    pGraphics->AttachControl(new IVKnobControl(b.GetGridCell(10, 3, 8).GetCentredInside(80), kDelayTimeLeft));
    pGraphics->AttachControl(new IVKnobControl(b.GetGridCell(11, 3, 8).GetCentredInside(80), kDelayTimeRight));
    pGraphics->AttachControl(new IVKnobControl(b.GetGridCell(12, 3, 8).GetCentredInside(80), kDelayFeedbackLeft));
    pGraphics->AttachControl(new IVKnobControl(b.GetGridCell(13, 3, 8).GetCentredInside(80), kDelayFeedbackRight));
    pGraphics->AttachControl(new IVKnobControl(b.GetGridCell(15, 3, 8).GetCentredInside(80), kGainSC));

    pGraphics->AttachControl(new IVKnobControl(b.GetGridCell(17, 3, 8).GetCentredInside(80), kEnvAttack));
    pGraphics->AttachControl(new IVKnobControl(b.GetGridCell(18, 3, 8).GetCentredInside(80), kEnvRelease));

    pGraphics->AttachControl(new IVKnobControl(b.GetGridCell(19, 3, 8).GetCentredInside(80), kThresh_dB));
    pGraphics->AttachControl(new IVKnobControl(b.GetGridCell(20, 3, 8).GetCentredInside(80), kSensitivity));

    pGraphics->AttachControl(new IVKnobControl(b.GetGridCell(23, 3, 8).SubRectVertical(2, 0).GetCentredInside(80), kWetMin));
    pGraphics->AttachControl(new IVKnobControl(b.GetGridCell(23, 3, 8).SubRectVertical(2, 1).GetCentredInside(80), kWetMax));
  };
#endif
}

#if IPLUG_DSP
void IPlugDuckingDelay::ProcessBlock(sample** inputs, sample** outputs, int nFrames)
{
  const int nChans = NOutChansConnected();

  mDuckingDelay->processBlock(inputs, outputs, nFrames);
}
void IPlugDuckingDelay::OnReset()
{
    double sampleRate = GetSampleRate();
    int blockSize = GetBlockSize();
    mDuckingDelay->reset(sampleRate, blockSize);
}
void IPlugDuckingDelay::OnParamChange(int paramIdx)
{
    const double parameterValue = GetParam(paramIdx)->Value();
    mDuckingDelay->SetParameters(paramIdx, parameterValue);
}
#endif
