/*******************************************************************************
The content of this file includes portions of the AUDIOKINETIC Wwise Technology
released in source code form as part of the SDK installer package.

Commercial License Usage

Licensees holding valid commercial licenses to the AUDIOKINETIC Wwise Technology
may use this file in accordance with the end user license agreement provided
with the software or, alternatively, in accordance with the terms contained in a
written agreement between you and Audiokinetic Inc.

Apache License Usage

Alternatively, this file may be used under the Apache License, Version 2.0 (the
"Apache License"); you may not use this file except in compliance with the
Apache License. You may obtain a copy of the Apache License at
http://www.apache.org/licenses/LICENSE-2.0.

Unless required by applicable law or agreed to in writing, software distributed
under the Apache License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES
OR CONDITIONS OF ANY KIND, either express or implied. See the Apache License for
the specific language governing permissions and limitations under the License.

Copyright (c) 2022 Audiokinetic Inc.
*******************************************************************************/

#ifndef GapTunerFXParams_H
#define GapTunerFXParams_H

#include <AK/SoundEngine/Common/IAkPlugin.h>
#include <AK/Plugin/PluginServices/AkFXParameterChangeHandler.h>

// Add parameters IDs here, those IDs should map to the AudioEnginePropertyID
// attributes in the xml property definition.
static const AkPluginParamID PARAM_OUTPUT_PITCH_REFERENCE_ID = 0;
static const AkPluginParamID PARAM_OUTPUT_PITCH_PARAMETER_ID_ID = 1;

static const AkPluginParamID PARAM_WINDOW_SIZE_ID = 2;
static const AkPluginParamID PARAM_MAX_NUM_KEY_MAXIMA_ID = 3;
static const AkPluginParamID PARAM_KEY_MAXIMA_THRESHOLD_MULTIPLIER_ID = 4;
static const AkPluginParamID PARAM_CLARITY_THRESHOLD_ID = 5;
static const AkPluginParamID PARAM_DOWNSAMPLING_FACTOR = 6;
static const AkPluginParamID PARAM_SMOOTHING_RATE_MS_ID = 7;
static const AkPluginParamID PARAM_SMOOTHING_CURVE_ID = 8;
static const AkPluginParamID PARAM_ZERO_OUT_UNPITCHED_ID = 9;
static const AkPluginParamID PARAM_UNPITCHED_COOLDOWN_MS_ID = 10;

static const AkUInt32 NUM_PARAMS = 11;

struct GapTunerRTPCParams
{
};

struct GapTunerNonRTPCParams
{
  AkUInt32 OutputPitchParameterId;
  AkUInt32 WindowSize;
  AkUInt32 MaxNumKeyMaxima;
  AkReal32 KeyMaximaThresholdMultiplier;
  AkReal32 ClarityThreshold;
  AkUInt32 DownsamplingFactor;
  AkUInt32 SmoothingRateMs;
  AkUInt32 SmoothingCurve; // As enum
  bool     ZeroOutUnpitched;
  AkUInt32 UnpitchedCooldownMs;
};

struct GapTunerFXParams
  : public AK::IAkPluginParam
{
  GapTunerFXParams();
  GapTunerFXParams(const GapTunerFXParams& in_rParams);

  ~GapTunerFXParams();

  // Create a duplicate of the parameter node instance in its current state.
  IAkPluginParam* Clone(AK::IAkPluginMemAlloc* in_pAllocator) override;

  // Initialize the plug-in parameter node interface.
  // Initializes the internal parameter structure to default values or with the
  // provided parameter block if it is valid.
  AKRESULT Init(AK::IAkPluginMemAlloc* in_pAllocator,
                const void* in_pParamsBlock,
                AkUInt32 in_ulBlockSize) override;

  // Called by the sound engine when a parameter node is terminated.
  AKRESULT Term(AK::IAkPluginMemAlloc* in_pAllocator) override;

  // Set all plug-in parameters at once using a parameter block.
  AKRESULT SetParamsBlock(const void* in_pParamsBlock,
                          AkUInt32 in_ulBlockSize) override;

  // Update a single parameter at a time and perform the necessary actions on
  // the parameter changes.
  AKRESULT SetParam(AkPluginParamID in_paramID,
                    const void* in_pValue,
                    AkUInt32 in_ulParamSize) override;

  AK::AkFXParameterChangeHandler<NUM_PARAMS> m_paramChangeHandler;

  GapTunerRTPCParams RTPC;
  GapTunerNonRTPCParams NonRTPC;
};

#endif // GapTunerFXParams_H
