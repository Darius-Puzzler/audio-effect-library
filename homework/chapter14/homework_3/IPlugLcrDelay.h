#pragma once

#include <IPlug_include_in_plug_hdr.h>
#include "projects/LcrDelay.h"

const int kNumPresets = 1;

enum EParams
{
	kDelayTime_mSec_L = 0,
	kDelayTime_mSec_R,
	kDelayTime_mSec_C,
	kDelayFeedback_Pct_C, // only center feedback
	kDelayDivision_L, // not used
	kDelayDivision_R, // not used
	kDelayDivision_C, // not used
	kFilterMode,
	kHP_fc,
	kLP_fc,
	kWetLevel_dB, // not used
	kDryLevel_dB, // not used
	kDelaySync, // not used
	kGain,
	kNumParams
};

using namespace iplug;
using namespace igraphics;
using namespace fxobjects;

class IPlugLcrDelay final : public Plugin
{
public:
  IPlugLcrDelay(const InstanceInfo& info);

#if IPLUG_DSP // http://bit.ly/2S64BDd
  void ProcessBlock(sample** inputs, sample** outputs, int nFrames) override;
  void OnReset() override;
  void OnParamChange(int paramIdx) override;

private:
	LcrDelay mLcrDelay;
	LCRAudioDelayParameters mLCRAudioDelayParameters;
#endif
};
