#include "IPlugWDFIdealRLC.h"
#include "IPlug_include_in_plug_src.h"
#include "IControls.h"

IPlugWDFIdealRLC::IPlugWDFIdealRLC(const InstanceInfo& info)
: iplug::Plugin(info, MakeConfig(kNumParams, kNumPresets))
{
  GetParam(kGain)->InitDouble("Gain", 0., 0., 100.0, 0.01, "%");
  GetParam(kFilterType)->InitEnum("Filter Type", 0, { "WDFIdealRLCLPF", "WDFIdealRLCHPF", "WDFIdealRLCBPF", "WDFIdealRLCBSF"});
  // fc needs to be initialized with the same values as the WDFParameters of the WDFIdealRLCLPF member
  // or there will be no audio output, only after "touching" the frequency knob in the gui which calls OnParamChange()
  GetParam(kFc)->InitDouble("fc", 100., 20., 20480.0, 0.01, "Hz", 0, "", IParam::ShapePowCurve(3.));
  GetParam(kQ)->InitDouble("Q", 0.707, 0.707, 20.0, 0.01, "");

#if IPLUG_EDITOR // http://bit.ly/2S64BDd
  mMakeGraphicsFunc = [&]() {
    return MakeGraphics(*this, PLUG_WIDTH, PLUG_HEIGHT, PLUG_FPS, GetScaleForScreen(PLUG_WIDTH, PLUG_HEIGHT));
  };
  
  mLayoutFunc = [&](IGraphics* pGraphics) {
    pGraphics->AttachCornerResizer(EUIResizerMode::Scale, false);
    pGraphics->AttachPanelBackground(COLOR_GRAY);
    pGraphics->LoadFont("Roboto-Regular", ROBOTO_FN);
    const IRECT b = pGraphics->GetBounds();
    pGraphics->AttachControl(new ITextControl(b.GetGridCell(0, 2, 3), "Hello iPlug 2 WDFIdealRLC!", IText(20)));
    pGraphics->AttachControl(new IVKnobControl(b.GetGridCell(2, 2, 3).GetCentredInside(100), kGain), kNoTag, "vcontrols");
    pGraphics->AttachControl(new IVKnobControl(b.GetGridCell(3, 2, 3).GetCentredInside(100), kFc, "fc"), kNoTag, "vcontrols");
    pGraphics->AttachControl(new IVKnobControl(b.GetGridCell(4, 2, 3).GetCentredInside(100), kQ, "Q"), kNoTag, "vcontrols");
    pGraphics->AttachControl(new IVMenuButtonControl(b.GetGridCell(5, 2, 3).GetCentredInside(125), kFilterType), kNoTag, "vcontrols");
  };
#endif
}

#if IPLUG_DSP
void IPlugWDFIdealRLC::ProcessBlock(sample** inputs, sample** outputs, int nFrames)
{
  const double gain = GetParam(kGain)->Value() / 100.;
  const int nChans = NOutChansConnected();
  
  for (int s = 0; s < nFrames; s++) {
    for (int c = 0; c < nChans; c++) {
      outputs[c][s] = mCurrentFilter->processAudioSample(inputs[c][s]) * gain;
    }
  }
}
void IPlugWDFIdealRLC::OnReset()
{
    const double sampleRate = GetSampleRate();
    mWDFIdealRLCLPF.reset(sampleRate);
    mWDFIdealRLCHPF.reset(sampleRate);
    mWDFIdealRLCBPF.reset(sampleRate);
    mWDFIdealRLCBSF.reset(sampleRate);

    // Apply parameters to WDF filter
    mWDFIdealRLCLPF.setParameters(mWDFParams);
    mWDFIdealRLCHPF.setParameters(mWDFParams);
    mWDFIdealRLCBPF.setParameters(mWDFParams);
    mWDFIdealRLCBSF.setParameters(mWDFParams);
}
void IPlugWDFIdealRLC::OnParamChange(int paramIdx)
{
    switch (paramIdx) {
    case kFc:
        mWDFParams.fc = GetParam(paramIdx)->Value();
        mCurrentFilter->setParameters(mWDFParams);
        break;
    case kQ:
        mWDFParams.Q = GetParam(paramIdx)->Value();
        mCurrentFilter->setParameters(mWDFParams);
        break;
    case kFilterType:
        const int value = GetParam(paramIdx)->Value();
        switch (value) {
        case kLPF:  mCurrentFilter = &mWDFIdealRLCLPF; break;
        case kHPF:  mCurrentFilter = &mWDFIdealRLCHPF; break;
        case kBPF:  mCurrentFilter = &mWDFIdealRLCBPF; break;
        case kBSF:  mCurrentFilter = &mWDFIdealRLCBSF; break;
        default:    mCurrentFilter = &mWDFIdealRLCLPF; break;
        }
    }
}
#endif
