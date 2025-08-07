#pragma once

#include <IPlug_include_in_plug_hdr.h>
#include "projects/MultiTapDelay.h"

const int kNumPresets = 1;

#define DELAY_TEMPODIV_VALIST "1/64", "1/32", "1/16T", "1/16", "1/16D", "1/8T", "1/8", "1/8D", "1/4", "1/4D", "1/2"

using namespace iplug;
using namespace igraphics;

class IPlugMultiTapDelay final : public Plugin
{
public:
  IPlugMultiTapDelay(const InstanceInfo& info);

#if IPLUG_DSP // http://bit.ly/2S64BDd
  void ProcessBlock(sample** inputs, sample** outputs, int nFrames) override;
  void OnReset() override;
  void OnParamChange(int paramIdx) override;
  void OnParamChangeUI(int paramIdx, EParamSource source) override;

public:
	MultiTapDelay mMultiTapDelay;

private:
	bool mSyncEnabled = false;
#endif
};
