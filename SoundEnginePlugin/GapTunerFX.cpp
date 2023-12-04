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


// ----------------------------------------------------------------
// GapTunerFX.cpp

#include "GapTunerFX.h"

// STL
#include <string>

// AK
#include <AK/AkWwiseSDKVersion.h>

// GapTuner
#include "GapTunerAnalysis.h"
#include "../GapTunerConfig.h"

// ----------------------------------------------------------------------------

// Initialize plugin
AKRESULT GapTunerFX::Init(AK::IAkPluginMemAlloc* InAllocator,
                          AK::IAkEffectPluginContext* InContext,
                          AK::IAkPluginParam* InParams,
                          AkAudioFormat& InFormat)
{
  // Set Wwise-plugin-specific members
  m_PluginParams = static_cast<GapTunerFXParams*>(InParams);
  m_PluginMemoryAllocator = InAllocator;
  m_PluginContext = InContext;

  // Keep track of sample rate
  m_SampleRate = static_cast<uint32_t>(InFormat.uSampleRate);

  // ----
  // Allocate memory for analysis window
  const uint32_t WindowSize = GetWindowSize();

  m_AutocorrelationCoefficients.resize(WindowSize);

  // The circular buffer sets its internal capacity to
  // InCapacity + 1; if we pass in WindowSize - 1, then
  // m_AnalysisWindow.GetCapacity() == WindowSize, which is
  // the result we want
  m_AnalysisWindow.SetCapacity(WindowSize - 1); 

  // ----
  // Allocate memory for key maxima
  const uint32_t MaxNumKeyMaxima =
    m_PluginParams->NonRTPC.MaxNumKeyMaxima;

  m_KeyMaximaLags.resize(MaxNumKeyMaxima);
  m_KeyMaximaCorrelations.resize(MaxNumKeyMaxima);

  // ----
  // Allocate memory for FFT
  const uint32_t FftWindowSize = WindowSize * 2;

  m_FftIn.resize(FftWindowSize);
  m_FftOut.resize(FftWindowSize);

  return AK_Success;
}

AKRESULT GapTunerFX::Term(AK::IAkPluginMemAlloc* InAllocator)
{
  AK_PLUGIN_DELETE(InAllocator, this);
  return AK_Success;
}

void GapTunerFX::Execute(AkAudioBuffer* InOutBuffer)
{
  // ----
  // Fill analysis window
  const uint32_t WindowSize = GetWindowSize();
  const uint32_t DownsamplingFactor =
    m_PluginParams->NonRTPC.DownsamplingFactor;

  const uint32_t NumSamplesPushed =
    GapTunerAnalysis::FillAnalysisWindow(InOutBuffer,
                                         m_AnalysisWindow,
                                         DownsamplingFactor);

  m_AnalysisWindowSamplesWritten += NumSamplesPushed;

  // Skip analysis if we haven't yet filled a full window
  if (m_AnalysisWindowSamplesWritten < WindowSize)
  {
    return;
  }

  // ----
  // Perform analysis

  // Naive method from Chapter 9
  /*
  GapTunerAnalysis::CalculateAcf(m_AnalysisWindow,
                                 m_AutocorrelationCoefficients);
  */

  // Improved method from Chapter 10
  GapTunerAnalysis::CalculateAcf_Fft(m_AnalysisWindow,
                                     m_FftIn,
                                     m_FftOut,
                                     m_AutocorrelationCoefficients);

  // ----
  // Peak picking
  const uint32_t MaxNumKeyMaxima =
    m_PluginParams->NonRTPC.MaxNumKeyMaxima;

  const uint32_t NumKeyMaxima =
    GapTunerAnalysis::FindKeyMaxima(m_KeyMaximaLags,
                                    m_KeyMaximaCorrelations,
                                    m_AutocorrelationCoefficients,
                                    MaxNumKeyMaxima);
  
  const float KeyMaximaThresholdMultiplier =
    m_PluginParams->NonRTPC.KeyMaximaThresholdMultiplier;

  const uint32_t BestMaximaLagIndex =
      GapTunerAnalysis::PickBestMaxima(m_KeyMaximaLags,
                                       m_KeyMaximaCorrelations,
                                       NumKeyMaxima,
                                       KeyMaximaThresholdMultiplier);

  // Best maxima lag and correlation
  const float BestMaximaLag = m_KeyMaximaLags[BestMaximaLagIndex];
  
  const float BestMaximaCorrelation =
    m_KeyMaximaCorrelations[BestMaximaLagIndex];

  // ----
  // Conversion
  const uint32_t AnalysisSampleRate =
    m_SampleRate / m_PluginParams->NonRTPC.DownsamplingFactor;

  const float BestMaximaFrequency =
      GapTunerAnalysis::ConvertSamplesToHz(BestMaximaLag,
                                           AnalysisSampleRate);

  // ----
  // Set output parameters

  // Only set parameter if clarity exceeds threshold
  const float ClarityThreshold =
    m_PluginParams->NonRTPC.ClarityThreshold;
  
  bool bSetRtpc = BestMaximaCorrelation > ClarityThreshold;
  if (bSetRtpc)
  {
    AkRtpcID OutputPitchParamId =
      m_PluginParams->NonRTPC.OutputPitchParameterId;
    
    AkRtpcValue OutputPitchParamValue = 0.f;

    // Set to best maxima frequency
    OutputPitchParamValue = BestMaximaFrequency;

    // Set RTPC with interpolation
    AK::IAkGlobalPluginContext* GlobalContext =
      m_PluginContext->GlobalContext();

    const auto SmoothingCurve = 
      static_cast<AkCurveInterpolation>(
        m_PluginParams->NonRTPC.SmoothingCurve);

    const uint32_t SmoothingRateMs =
      m_PluginParams->NonRTPC.SmoothingRateMs;

    AKRESULT Result =
      GlobalContext->SetRTPCValue(OutputPitchParamId,
                                  OutputPitchParamValue,
                                  AK_INVALID_GAME_OBJECT,
                                  SmoothingRateMs,
                                  SmoothingCurve,
                                  false);
  }
}

// -----------------------------------------------------------------------------

uint32_t GapTunerFX::GetWindowSize() const
{
  return m_PluginParams->NonRTPC.WindowSize /
         m_PluginParams->NonRTPC.DownsamplingFactor;
}

// -----------------------------------------------------------------------------

AK::IAkPlugin* CreateGapTunerFX(AK::IAkPluginMemAlloc* in_pAllocator)
{
    return AK_PLUGIN_NEW(in_pAllocator, GapTunerFX());
}

AK::IAkPluginParam* CreateGapTunerFXParams(AK::IAkPluginMemAlloc* in_pAllocator)
{
    return AK_PLUGIN_NEW(in_pAllocator, GapTunerFXParams());
}

AK_IMPLEMENT_PLUGIN_FACTORY(GapTunerFX,
                            AkPluginTypeEffect,
                            GapTunerConfig::CompanyID,
                            GapTunerConfig::PluginID)

AKRESULT GapTunerFX::Reset()
{
    return AK_Success;
}

AKRESULT GapTunerFX::GetPluginInfo(AkPluginInfo& out_rPluginInfo)
{
    out_rPluginInfo.eType = AkPluginTypeEffect;
    out_rPluginInfo.bIsInPlace = true;
	  out_rPluginInfo.bCanProcessObjects = false;
    out_rPluginInfo.uBuildVersion = AK_WWISESDK_VERSION_COMBINED;
    return AK_Success;
}


AKRESULT GapTunerFX::TimeSkip(AkUInt32 in_uFrames)
{
    return AK_DataReady;
}
