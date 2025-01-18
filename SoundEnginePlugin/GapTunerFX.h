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
// GapTunerFX.h

// The audio effect portion of our pitch detection plugin.

#pragma once

// STL
#include <complex>

// AK
#include <AK/SoundEngine/Common/IAkPlugin.h>

// CircularAudioBuffer
#include "CircularAudioBuffer/CircularAudioBuffer.h"

// GapTuner
#include "GapTunerFXParams.h"

class GapTunerFX : public AK::IAkInPlaceEffectPlugin
{

public:

  // ----------------

  GapTunerFX() = default;

  // ----------------
  // IAkEffectPlugin::

  // Initialize plugin
  AKRESULT Init(AK::IAkPluginMemAlloc* InAllocator,
                AK::IAkEffectPluginContext* InContext,
                AK::IAkPluginParam* InParams,
                AkAudioFormat& InFormat) override;

  // Terminate plugin
  AKRESULT Term(AK::IAkPluginMemAlloc* InAllocator) override;

  // Execute DSP for a block of audio
  void Execute(AkAudioBuffer* InOutBuffer) override;

  // ----------------
  // IAkInPlaceEffectPlugin::

  // The reset action should perform any actions required to reinitialize the
  // state of the plug-in to its original state (e.g. after Init() or on effect
  // bypass).
  AKRESULT Reset() override;

  // Plug-in information query mechanism used when the sound engine requires
  // information about the plug-in to determine its behavior.
  AKRESULT GetPluginInfo(AkPluginInfo& out_rPluginInfo) override;

  // Skips execution of some frames, when the voice is virtual playing from
  // elapsed time.
  //
  // This can be used to simulate processing that would have taken place (e.g.
  // update internal state).
  //
  // Return AK_DataReady or AK_NoMoreData, depending if there would be audio
  // output or not at that point.
  AKRESULT TimeSkip(AkUInt32 in_uFrames) override;

private:

  // ----------------

  // Get our actual window size, taking downsampling into account
  uint32_t GetWindowSize() const;

  // Set the value of our output pitch RTPC
  AKRESULT SetOutputPitchParameterValue(
    AkRtpcValue InOutputPitchParamValue);

  // ----------------

  // Wwise-plugin-specific members, set in Init()
  GapTunerFXParams* m_PluginParams { nullptr };
  AK::IAkPluginMemAlloc* m_PluginMemoryAllocator { nullptr };
  AK::IAkEffectPluginContext* m_PluginContext { nullptr };

  // Sample rate, also set in Init()
  uint32_t m_SampleRate { 48000 };

  // How long (in ms) we've been unable to make a valid pitch
  // prediction for
  uint32_t m_UnpitchedTimeElapsedMs { 0 };

  // ----------------
  // Analysis members

  // Analysis window backed by circular buffer class
  CircularAudioBuffer<float> m_AnalysisWindow { };

  // How many samples we've written to the analysis window so far
  uint32_t m_AnalysisWindowSamplesWritten { 0 };

  // Calculated autocorrelation coefficients
  std::vector<float> m_AutocorrelationCoefficients { };

  // Key maxima lags and correlations, for MPM-based peak-picking
  std::vector<float> m_KeyMaximaLags { };
  std::vector<float> m_KeyMaximaCorrelations { };

  // FFT
  std::vector<std::complex<double>> m_FftIn { };
  std::vector<std::complex<double>> m_FftOut { };
};
