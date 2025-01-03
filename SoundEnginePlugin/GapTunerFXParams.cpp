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

#include "GapTunerFXParams.h"

#include <AK/Tools/Common/AkBankReadHelpers.h>

GapTunerFXParams::GapTunerFXParams()
{
}

GapTunerFXParams::~GapTunerFXParams()
{
}

GapTunerFXParams::GapTunerFXParams(const GapTunerFXParams& in_rParams)
{
  RTPC = in_rParams.RTPC;
  NonRTPC = in_rParams.NonRTPC;
  m_paramChangeHandler.SetAllParamChanges();
}

AK::IAkPluginParam* GapTunerFXParams::Clone(
  AK::IAkPluginMemAlloc* in_pAllocator)
{
  return AK_PLUGIN_NEW(in_pAllocator, GapTunerFXParams(*this));
}

AKRESULT GapTunerFXParams::Init(AK::IAkPluginMemAlloc* in_pAllocator,
                                const void* in_pParamsBlock,
                                AkUInt32 in_ulBlockSize)
{
  if (in_ulBlockSize == 0)
  {
      // Initialize default parameters here
      NonRTPC.OutputPitchParameterId = 0;
      NonRTPC.WindowSize = 2048;
      NonRTPC.MaxNumKeyMaxima = 8;
      NonRTPC.KeyMaximaThresholdMultiplier = 0.9f;
      NonRTPC.ClarityThreshold = 0.8f;
      NonRTPC.DownsamplingFactor = 2;
      NonRTPC.SmoothingRateMs = 0;
      NonRTPC.SmoothingCurve = 0;
      NonRTPC.ZeroOutUnpitched = false;
      NonRTPC.UnpitchedCooldownMs = 50;
      
      m_paramChangeHandler.SetAllParamChanges();
      return AK_Success;
  }

  return SetParamsBlock(in_pParamsBlock, in_ulBlockSize);
}

AKRESULT GapTunerFXParams::Term(AK::IAkPluginMemAlloc* in_pAllocator)
{
  AK_PLUGIN_DELETE(in_pAllocator, this);
  return AK_Success;
}

AKRESULT GapTunerFXParams::SetParamsBlock(const void* in_pParamsBlock,
                                          AkUInt32 in_ulBlockSize)
{
  AKRESULT eResult = AK_Success;
  AkUInt8* pParamsBlock = (AkUInt8*)in_pParamsBlock;

  // Read bank data here
  NonRTPC.OutputPitchParameterId =         READBANKDATA(AkUInt32,
                                                        pParamsBlock,
                                                        in_ulBlockSize);
  NonRTPC.WindowSize =                     READBANKDATA(AkUInt32,
                                                        pParamsBlock,
                                                        in_ulBlockSize);
  NonRTPC.MaxNumKeyMaxima =                READBANKDATA(AkUInt32,
                                                        pParamsBlock,
                                                        in_ulBlockSize);
  NonRTPC.KeyMaximaThresholdMultiplier =   READBANKDATA(AkReal32,
                                                        pParamsBlock,
                                                        in_ulBlockSize);
  NonRTPC.ClarityThreshold =               READBANKDATA(AkReal32,
                                                       pParamsBlock,
                                                       in_ulBlockSize);
  NonRTPC.DownsamplingFactor =             READBANKDATA(AkUInt32,
                                                        pParamsBlock,
                                                        in_ulBlockSize);
  NonRTPC.SmoothingRateMs =                READBANKDATA(AkUInt32,
                                                        pParamsBlock,
                                                        in_ulBlockSize);
  NonRTPC.SmoothingCurve =                 READBANKDATA(AkUInt32,
                                                        pParamsBlock,
                                                        in_ulBlockSize);
  NonRTPC.ZeroOutUnpitched =               READBANKDATA(bool,
                                                        pParamsBlock,
                                                        in_ulBlockSize);
  NonRTPC.UnpitchedCooldownMs =            READBANKDATA(AkUInt32,
                                                        pParamsBlock,
                                                        in_ulBlockSize);

  CHECKBANKDATASIZE(in_ulBlockSize, eResult);
  m_paramChangeHandler.SetAllParamChanges();

  return eResult;
}

AKRESULT GapTunerFXParams::SetParam(AkPluginParamID in_paramID,
                                    const void* in_pValue,
                                    AkUInt32 in_ulParamSize)
{
  AKRESULT eResult = AK_Success;

  // Handle parameter change here
  switch (in_paramID)
  {
    case PARAM_OUTPUT_PITCH_PARAMETER_ID_ID:
      NonRTPC.OutputPitchParameterId = *((AkUInt32*)in_pValue);
      m_paramChangeHandler.SetParamChange(PARAM_OUTPUT_PITCH_PARAMETER_ID_ID);
      break;
    case PARAM_WINDOW_SIZE_ID:
      NonRTPC.WindowSize = *((AkUInt32*)in_pValue);
      m_paramChangeHandler.SetParamChange(PARAM_WINDOW_SIZE_ID);
      break;
    case PARAM_MAX_NUM_KEY_MAXIMA_ID:
      NonRTPC.MaxNumKeyMaxima = *((AkUInt32*)in_pValue);
      m_paramChangeHandler.SetParamChange(PARAM_MAX_NUM_KEY_MAXIMA_ID);
      break;
    case PARAM_KEY_MAXIMA_THRESHOLD_MULTIPLIER_ID:
      NonRTPC.KeyMaximaThresholdMultiplier = *((AkReal32*)in_pValue);
      m_paramChangeHandler.SetParamChange(
        PARAM_KEY_MAXIMA_THRESHOLD_MULTIPLIER_ID);
      break;
    case PARAM_CLARITY_THRESHOLD_ID:
      NonRTPC.ClarityThreshold = *((AkReal32*)in_pValue);
      m_paramChangeHandler.SetParamChange(PARAM_CLARITY_THRESHOLD_ID);
      break;
    case PARAM_DOWNSAMPLING_FACTOR:
      NonRTPC.DownsamplingFactor = *((AkUInt32*)in_pValue);
      m_paramChangeHandler.SetParamChange(PARAM_DOWNSAMPLING_FACTOR);
      break;
    case PARAM_SMOOTHING_RATE_MS_ID:
      NonRTPC.SmoothingRateMs = *((AkUInt32*)in_pValue);
      m_paramChangeHandler.SetParamChange(PARAM_SMOOTHING_RATE_MS_ID);
      break;
    case PARAM_SMOOTHING_CURVE_ID:
      NonRTPC.SmoothingCurve = *((AkUInt32*)in_pValue);
      m_paramChangeHandler.SetParamChange(PARAM_SMOOTHING_CURVE_ID);
      break;
    case PARAM_ZERO_OUT_UNPITCHED_ID:
        NonRTPC.ZeroOutUnpitched = *((bool*)in_pValue);
        m_paramChangeHandler.SetParamChange(PARAM_ZERO_OUT_UNPITCHED_ID);
        break;
    case PARAM_UNPITCHED_COOLDOWN_MS_ID:
        NonRTPC.UnpitchedCooldownMs = *((AkUInt32*)in_pValue);
        m_paramChangeHandler.SetParamChange(PARAM_UNPITCHED_COOLDOWN_MS_ID);
        break;
    default:
      eResult = AK_InvalidParameter;
      break;
  }

  return eResult;
}
