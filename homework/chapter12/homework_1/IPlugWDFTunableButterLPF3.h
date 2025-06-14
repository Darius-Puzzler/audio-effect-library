#pragma once

#include <IPlug_include_in_plug_hdr.h>
#include "projects/WDFTunableButterLPF3.h"

const int kNumPresets = 1;

enum EParams
{
  kGain = 0,
  kFc,
  kNumParams
};

using namespace iplug;
using namespace igraphics;

class IPlugWDFTunableButterLPF3 final : public Plugin
{
public:
  IPlugWDFTunableButterLPF3(const InstanceInfo& info);

#if IPLUG_DSP // http://bit.ly/2S64BDd
  void ProcessBlock(sample** inputs, sample** outputs, int nFrames) override;
  void OnReset() override;
  void OnParamChange(int paramIdx) override;
public:
	WDFTunableButterLPF3 mWDFTunableButterLPF3;
#endif
};
