#include "IPlugWDFTunableButterLPF3.h"
#include "IPlug_include_in_plug_src.h"
#include "IControls.h"

IPlugWDFTunableButterLPF3::IPlugWDFTunableButterLPF3(const InstanceInfo& info)
: iplug::Plugin(info, MakeConfig(kNumParams, kNumPresets))
{
  GetParam(kGain)->InitDouble("Gain", 0., 0., 100.0, 0.01, "%");
  GetParam(kFc)->InitDouble("fc", 1000., 60., 20480.0, 10, "Hz", 0, "", IParam::ShapePowCurve(3.));

#if IPLUG_EDITOR // http://bit.ly/2S64BDd
  mMakeGraphicsFunc = [&]() {
    return MakeGraphics(*this, PLUG_WIDTH, PLUG_HEIGHT, PLUG_FPS, GetScaleForScreen(PLUG_WIDTH, PLUG_HEIGHT));
  };
  
  mLayoutFunc = [&](IGraphics* pGraphics) {
    pGraphics->AttachCornerResizer(EUIResizerMode::Scale, false);
    pGraphics->AttachPanelBackground(COLOR_GRAY);
    pGraphics->LoadFont("Roboto-Regular", ROBOTO_FN);
    const IRECT b = pGraphics->GetBounds();
    pGraphics->AttachControl(new ITextControl(b.GetGridCell(0, 2, 2).GetMidVPadded(50), "Hello iPlug 2 WDFTunableButterLPF3!", IText(20)));
    pGraphics->AttachControl(new IVKnobControl(b.GetGridCell(1, 2, 2).GetCentredInside(100), kGain));
    pGraphics->AttachControl(new IVKnobControl(b.GetGridCell(3, 2, 2).GetCentredInside(100).GetVShifted(-100), kFc));
  };
#endif
}

#if IPLUG_DSP
void IPlugWDFTunableButterLPF3::ProcessBlock(sample** inputs, sample** outputs, int nFrames)
{
  const double gain = GetParam(kGain)->Value() / 100.;
  const int nChans = NOutChansConnected();
  
  for (int s = 0; s < nFrames; s++) {
    for (int c = 0; c < nChans; c++) {
      outputs[c][s] = mWDFTunableButterLPF3.processAudioSample(inputs[c][s]) * gain;
    }
  }
}
void IPlugWDFTunableButterLPF3::OnReset()
{
    mWDFTunableButterLPF3.reset(GetSampleRate());
}
void IPlugWDFTunableButterLPF3::OnParamChange(int paramIdx)
{
    switch (paramIdx) {
    case kFc:
        mWDFTunableButterLPF3.calculateNewComponentValues(GetParam(paramIdx)->Value());
        break;
    }
}
#endif
