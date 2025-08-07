#pragma once

#include <IPlug_include_in_plug_hdr.h>
#include "projects/AnalogModelingDelay.h"

const int kNumPresets = 1;

enum EParams
{
	kDelayTime_mSec_L = 0,
	kDelayTime_mSec_R,
	kDelayFeedback_Pct_L,
	kDelayFeedback_Pct_R,
	kGain,
	kNumParams
};

using namespace iplug;
using namespace igraphics;
using namespace fxobjects;

class IPlugAnalogModelingDelay final : public Plugin
{
public:
  IPlugAnalogModelingDelay(const InstanceInfo& info);

#if IPLUG_DSP // http://bit.ly/2S64BDd
  void ProcessBlock(sample** inputs, sample** outputs, int nFrames) override;
  void OnReset() override;
  void OnParamChangeUI(int paramIdx, EParamSource source) override;
  void OnParamChange(int paramIdx) override;

private:
	AnalogModelingDelay mAnalogModelingDelay;
	AudioDelayParameters mAudioDelayParameters;
#endif
};
