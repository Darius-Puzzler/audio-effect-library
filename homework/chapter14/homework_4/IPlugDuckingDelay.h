#pragma once

#include <IPlug_include_in_plug_hdr.h>
#include "projects/DuckingDelay.h"


const int kNumPresets = 1;

constexpr int nChannels = 2;

using namespace iplug;
using namespace igraphics;
using namespace fxobjects;

class IPlugDuckingDelay final : public Plugin
{
public:
  IPlugDuckingDelay(const InstanceInfo& info);

#if IPLUG_DSP // http://bit.ly/2S64BDd
  void ProcessBlock(sample** inputs, sample** outputs, int nFrames) override;
  void OnReset() override;
  void OnParamChange(int paramIdx) override;

private:
	DuckingDelay mDuckingDelay[nChannels];
#endif
};