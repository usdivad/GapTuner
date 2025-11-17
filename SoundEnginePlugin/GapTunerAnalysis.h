// ----------------------------------------------------------------
// GapTunerAnalysis.h

// ...

#pragma once

#undef min
#undef max
#define NOMINMAX

// STL
#include <algorithm>
#include <complex>

// AK
#include <AK/SoundEngine/Common/IAkPlugin.h>
#include <AK/Tools/Common/AkArray.h>

// CircularAudioBuffer
#include "CircularAudioBuffer/CircularAudioBuffer.h"

// dj_fft
#include "dj_fft/dj_fft.h"

namespace GapTunerAnalysis
{
  // ----------------
  // Autocorrelation -- naive method from Chapter 9

  // Calculate normalized autocorrelation function for a window
  void CalculateAcf(
    const CircularAudioBuffer<float>& InAnalysisWindow,
    AkArray<float, float, AkPluginArrayAllocator>& OutAutocorrelations);

  // Calculate autocorrelation (using dot product) for a given lag
  float CalculateAcfForLag(
    const CircularAudioBuffer<float>& InSamples,
    const uint32_t InLag);

  // ----------------
  // Autocorrelation -- improved method from Chapter 10

  // Calculate autocorrelation using the FFT
  void CalculateAcf_Fft(
    const CircularAudioBuffer<float>& InAnalysisWindow,
    AkArray<std::complex<double>, std::complex<double>, AkPluginArrayAllocator>& OutFftInput,
    AkArray<std::complex<double>, std::complex<double>, AkPluginArrayAllocator>& OutFftOutput,
    AkArray<float, float, AkPluginArrayAllocator>& OutAutocorrelations);

  // Calculate the FFT (forwards or backwards) given an input
  // sequence
  void CalculateFft(
    const AkArray<std::complex<double>, std::complex<double>, AkPluginArrayAllocator>& InFftSequence,
    AkArray<std::complex<double>, std::complex<double>, AkPluginArrayAllocator>& OutFftSequence,
    const dj::fft_dir InFftDirection);

  // ----------------
  // Peak-picking -- Naive
  
  // Pick the peak lag given the autocorrelation coefficients for a
  // series of time lags
  uint32_t FindAcfPeakLag(
    const AkArray<float, float, AkPluginArrayAllocator>& InAutocorrelations);

  // ----------------
  // Peak-picking -- MPM

  // Gather a list of key maxima using the MPM's peak-picking process
  uint32_t FindKeyMaxima(AkArray<float, float, AkPluginArrayAllocator>& OutKeyMaximaLags,
                         AkArray<float, float, AkPluginArrayAllocator>& OutKeyMaximaCorrelations,
                         const AkArray<float, float, AkPluginArrayAllocator>& InAutocorrelations,
                         const uint32_t InMaxNumMaxima);

  // Pick the best maxima from a list of key maxima
  uint32_t PickBestMaxima(
    const AkArray<float, float, AkPluginArrayAllocator>& InKeyMaximaLags,
    const AkArray<float, float, AkPluginArrayAllocator>& InKeyMaximaCorrelations,
    const uint32_t InNumKeyMaxima,
    const float InThresholdMultiplier);

  // Find the interpolated maxima for a given lag
  float FindInterpolatedMaximaLag(
    const uint32_t InMaximaLag,
    const AkArray<float, float, AkPluginArrayAllocator>& InAutocorrelations);

  // ----------------
  // Utilities

  // Convert from num samples to Hz, based on a given sample rate
  float ConvertSamplesToHz(const float InNumSamples,
                           const uint32_t InSampleRate);

  // Fill an analysis window with samples from an input audio buffer
  uint32_t FillAnalysisWindow(AkAudioBuffer* InBuffer,
                              CircularAudioBuffer<float>& InOutWindow,
                              const uint32_t InDownsamplingFactor);
}
