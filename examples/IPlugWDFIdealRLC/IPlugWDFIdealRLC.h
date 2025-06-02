#pragma once

#include <memory>
#include <IPlug_include_in_plug_hdr.h>
#include "projects/WDFIdealRLCLPF.h"
#include "projects/WDFIdealRLCHPF.h"
#include "projects/WDFIdealRLCBPF.h"
#include "projects/WDFIdealRLCBSF.h"

const int kNumPresets = 1;

enum EParams
{
  kGain = 0,
  kFilterType,
  kFc,
  kQ,
  kboostCut_dB,
  kNumParams
};

enum EFilterTypes
{
	kLPF,
	kHPF,
	kBPF,
	kBSF,
	kNumFilterTypes
};

using namespace iplug;
using namespace igraphics;

class IPlugWDFIdealRLC final : public Plugin
{
public:
  IPlugWDFIdealRLC(const InstanceInfo& info);

#if IPLUG_DSP // http://bit.ly/2S64BDd
  void ProcessBlock(sample** inputs, sample** outputs, int nFrames) override;
  void OnReset() override;
  void OnParamChange(int paramIdx) override;
private:
	WDFIdealRLCLPF mWDFIdealRLCLPF;
	WDFIdealRLCHPF mWDFIdealRLCHPF;
	WDFIdealRLCBPF mWDFIdealRLCBPF;
	WDFIdealRLCBSF mWDFIdealRLCBSF;
	IAudioSignalProcessor* mCurrentFilter = nullptr;
	WDFParameters mWDFParams;
#endif
};
