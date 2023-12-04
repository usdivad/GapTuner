// ----------------------------------------------------------------
// GapTunerAnalysis.cpp

// ...

#include "GapTunerAnalysis.h"

// libc
#include <assert.h>

namespace GapTunerAnalysis
{
  
  void CalculateAcf(
    const CircularAudioBuffer<float>& InAnalysisWindow,
    std::vector<float>& OutAutocorrelations)
  {
    assert(
      InAnalysisWindow.GetCapacity() == OutAutocorrelations.size());
    
    const size_t WindowSize = InAnalysisWindow.GetCapacity();

    // Calculate ACF for each lag value
    for (uint32_t Lag = 0; Lag < WindowSize; ++Lag)
    {
      OutAutocorrelations[Lag] = CalculateAcfForLag(InAnalysisWindow,
                                                    Lag);
    }

    // Normalize
    const float FirstCorrelation = OutAutocorrelations[0];
    const float NormalizeMultiplier = FirstCorrelation != 0.f
                                      ? 1.f / FirstCorrelation
                                      : 1.f;
    for (uint32_t Lag = 0; Lag < WindowSize; ++Lag)
    {
      OutAutocorrelations[Lag] *= NormalizeMultiplier;
    }
  }

  float CalculateAcfForLag(
    const CircularAudioBuffer<float>& InSamples,
   const uint32_t InLag)
  {
    const size_t WindowSize = InSamples.GetCapacity();
    float Sum = 0.f;

    for (uint32_t SampleIdx = 0; SampleIdx < WindowSize; ++SampleIdx)
    {
      // All samples before lag amount are zeroed out, so we only
      // need to run calculations for lagged samples
      if (SampleIdx >= InLag)
      {
        const float SampleValue = InSamples.At(SampleIdx);
        
        const float LaggedSampleValue =
          InSamples.At(SampleIdx - InLag);

        Sum += SampleValue * LaggedSampleValue;
      }
    }

    return Sum;
  }

  void CalculateAcf_Fft(
    const CircularAudioBuffer<float>& InAnalysisWindow,
    std::vector<std::complex<double>>& OutFftInput,
    std::vector<std::complex<double>>& OutFftOutput,
    std::vector<float>& OutAutocorrelations)
  {
    // 1. Fill the input array with the contents of the analysis
    //    window, then zero-pad it so that it's twice the window size
    assert(
      InAnalysisWindow.GetCapacity() == OutAutocorrelations.size());

    const size_t AnalysisWindowSize = InAnalysisWindow.GetCapacity();

    const size_t FftWindowSize = AnalysisWindowSize * 2;

    for (uint32_t SampleIdx = 0;
         SampleIdx < FftWindowSize;
         ++SampleIdx)
    {
      if (SampleIdx < AnalysisWindowSize)
      {
        float SampleValue = 0;
        SampleValue = InAnalysisWindow.At(SampleIdx);
        OutFftInput[SampleIdx] = std::complex<double>(SampleValue);
      }
      else
      {
        OutFftInput[SampleIdx] = std::complex<double>(0.f);
      }
    }

    // 2. Take the FFT of the zero-padded input
    CalculateFft(OutFftInput,
                 OutFftOutput,
                 dj::fft_dir::DIR_FWD);

    // 3. Compute the squared magnitude of each coefficient in the
    //    FFT output, to get the power spectral density
    for (uint32_t CoeffIdx = 0; CoeffIdx < FftWindowSize; ++CoeffIdx)
    {
      const std::complex<double> Coefficient = OutFftOutput[CoeffIdx];
      const std::complex<double> Conjugate = std::conj(Coefficient);

      const std::complex<double> SquaredMagnitude =
        Coefficient * Conjugate;

      OutFftInput[CoeffIdx] = SquaredMagnitude;
    }

    // 4. Take the IFFT (inverse FFT) of the array of squared
    //    magnitudes
    CalculateFft(OutFftInput,
                 OutFftOutput,
                 dj::fft_dir::DIR_BWD);

    // 5. Take the real part of each value in the IFFT output and
    //    divide by the DC component (first element) -- the result
    //    gives the correlation coefficient between -1 and 1
    const auto IfftDcComponent =
      static_cast<float>(OutFftOutput[0].real());

    for (uint32_t CoeffIdx = 0;
         CoeffIdx < AnalysisWindowSize;
         ++CoeffIdx)
    {
      const auto CoefficientRealComponent =
        static_cast<float>(OutFftOutput[CoeffIdx].real());

      OutAutocorrelations[CoeffIdx] = CoefficientRealComponent /
                                       IfftDcComponent;
    }
  }

  void CalculateFft(
    const std::vector<std::complex<double>>& InFftSequence,
    std::vector<std::complex<double>>& OutFftSequence,
    const dj::fft_dir InFftDirection)
  {
    dj::fft1d(InFftSequence, OutFftSequence, InFftDirection);
  }

  uint32_t FindAcfPeakLag(
    const std::vector<float>& InAutocorrelations)
  {
    const size_t WindowSize = InAutocorrelations.size();
    uint32_t PeakLag = 0;
    float PeakCorr = 0.f;
    bool bReachedFirstZeroCrossing = false;

    // NOTE: Skip first correlation and only go up to half the window
    //       size, and only start counting after first zero crossing
    for (uint32_t Lag = 1; Lag < WindowSize / 2.f; ++Lag)
    {
      const float Corr = InAutocorrelations[Lag];

      const float PrevCorr = InAutocorrelations[Lag - 1];

      // We've reached first zero crossing when sign changes
      if (Corr * PrevCorr < 0)
      {
        bReachedFirstZeroCrossing = true;
      }

      // Update peak if zero crossing has been reached
      if (bReachedFirstZeroCrossing && Corr > PeakCorr)
      {
        PeakLag = Lag;
        PeakCorr = Corr;
      }
    }

    return PeakLag;
  }

  uint32_t FindKeyMaxima(std::vector<float>& OutKeyMaximaLags,
                         std::vector<float>& OutKeyMaximaCorrelations,
                         const std::vector<float>& InAutocorrelations,
                         const uint32_t InMaxNumMaxima)
  {
    const size_t WindowSize = InAutocorrelations.size();
    uint32_t MaximaIdx = 0;
    float MaximaLag = 0.f;
    float MaximaCorr = 0.f;
    bool bReachedNextPositiveZeroCrossing = false;

    // Again, skip first correlation and go up to half the window size
    for (uint32_t Lag = 1; Lag < WindowSize / 2.f; ++Lag)
    {
      const float PrevCorr = InAutocorrelations[Lag - 1];
      const float Corr = InAutocorrelations[Lag];
      const float NextCorr = InAutocorrelations[Lag + 1];

      // We've reached zero crossing when sign changes
      if (Corr * PrevCorr < 0)
      {
        if (PrevCorr < 0.f && NextCorr > 0.f) // Positive slope
        {
          bReachedNextPositiveZeroCrossing = true;

          // Add peak indices and values
          OutKeyMaximaLags[MaximaIdx] = MaximaLag;
          OutKeyMaximaCorrelations[MaximaIdx] = MaximaCorr;

          // Increment maxima index
          MaximaIdx++;
          if (MaximaIdx >= InMaxNumMaxima)
          {
            break;
          }

          // Reset
          MaximaLag = 0;
          MaximaCorr = 0.f;
        }
        else if (PrevCorr > 0.f && NextCorr < 0.f) // Negative slope
        {
          bReachedNextPositiveZeroCrossing = false;
        }
      }

      // Update peak if we're between positive and negative zero
      // crossings
      if (bReachedNextPositiveZeroCrossing && Corr > MaximaCorr)
      {
        // Without parabolic interpolation
        // MaximaLag = static_cast<float>(Lag);

        // With parabolic interpolation
        MaximaLag = FindInterpolatedMaximaLag(Lag,
                                              InAutocorrelations);

        // Either way the correlation is the same
        MaximaCorr = Corr;
      }
    }

    // This tells us how many maxima we ultimately found.
    // It will always be <= InMaxNumMaxima
    return MaximaIdx;
  }

  // Pick the best maxima from key maxima
  uint32_t PickBestMaxima(
    const std::vector<float>& InKeyMaximaLags,
    const std::vector<float>& InKeyMaximaCorrelations,
    const uint32_t InNumKeyMaxima,
    const float InThresholdMultiplier)
  {
    // This is the index in the array of maxima, not the actual lag
    uint32_t HighestMaximaIdx = 0;
    float HighestMaximaCorr = 0.f;

    // Find highest maxima index and correlation
    for (uint32_t MaximaIdx = 0;
         MaximaIdx < InNumKeyMaxima;
         ++MaximaIdx)
    {
      const float MaximaCorr = InKeyMaximaCorrelations[MaximaIdx];

      if (MaximaCorr > HighestMaximaCorr)
      {
        HighestMaximaIdx = MaximaIdx;
        HighestMaximaCorr = MaximaCorr;
      }
    }

    // Pick first one that's larger than threshold
    uint32_t BestMaximaIdx = HighestMaximaIdx;
    const float Threshold = HighestMaximaCorr * InThresholdMultiplier;

    for (uint32_t MaximaIdx = 0;
         MaximaIdx < InNumKeyMaxima;
         ++MaximaIdx)
    {
      const float MaximaCorr = InKeyMaximaCorrelations[MaximaIdx];

      if (MaximaCorr >= Threshold)
      {
        BestMaximaIdx = MaximaIdx;
        break;
      }
    }

    return BestMaximaIdx;
  }

  float FindInterpolatedMaximaLag(
    const uint32_t InMaximaLag,
    const std::vector<float>& InAutocorrelations)
  {
    const size_t WindowSize = InAutocorrelations.size();
    auto InterpolatedLag = static_cast<float>(InMaximaLag);

    // Can't interpolate first or last lag value
    if (InMaximaLag < 1 || InMaximaLag >= WindowSize - 1)
    {
      return InterpolatedLag;
    }

    // Get correlations for lags
    const uint32_t LeftNeighborLag = InMaximaLag - 1;
    const uint32_t RightNeighborLag = InMaximaLag + 1;

    const float MaximaCorrelation = InAutocorrelations[InMaximaLag];
    
    const float LeftNeighborCorrelation =
      InAutocorrelations[LeftNeighborLag];
    
    const float RightNeighborCorrelation =
      InAutocorrelations[RightNeighborLag];

    // ----
    // Perform interpolation calculation

    const float InterpolationAlpha =
      (MaximaCorrelation - LeftNeighborCorrelation) /
      (InMaximaLag - LeftNeighborLag);

    const float InterpolationBeta =
      (RightNeighborCorrelation - LeftNeighborCorrelation -
        (InterpolationAlpha * (RightNeighborLag - LeftNeighborLag))) /
      ((RightNeighborLag - LeftNeighborLag) *
        (RightNeighborLag - InMaximaLag));

    InterpolatedLag = ((LeftNeighborLag + InMaximaLag) / 2.f) -
                      (InterpolationAlpha / (2 * InterpolationBeta));

    return InterpolatedLag;
  }

  float ConvertSamplesToHz(const float InNumSamples,
                           const uint32_t InSampleRate)
  {
    if (InNumSamples == 0.f)
    {
      return 0.f;
    }

    return static_cast<float>(InSampleRate) / InNumSamples;
  }

  uint32_t FillAnalysisWindow(AkAudioBuffer* InBuffer,
                              CircularAudioBuffer<float>& InOutWindow,
                              const uint32_t InDownsamplingFactor)
  {
    const uint32_t NumChannels = InBuffer->NumChannels();
    const uint32_t NumSamples = InBuffer->uValidFrames;
    uint32_t NumSamplesPushed = 0;

    // Set analysis window read index to write index, so that we
    // always write as many samples as we have available
    InOutWindow.AlignReadWriteIndices();

    // Average all input channels so that we only analyze one single
    // buffer
    for (uint32_t SampleIdx = 0; SampleIdx < NumSamples; ++SampleIdx)
    {
      float SampleValue = 0.f;

      // Sum input sample values across channels
      for (uint32_t ChannelIdx = 0;
           ChannelIdx < NumChannels;
           ++ChannelIdx)
      {
        AkSampleType* pChannel = InBuffer->GetChannel(ChannelIdx);
        auto pBuf = static_cast<float*>(pChannel);

        SampleValue += pBuf[SampleIdx];
      }

      // Average summed sample values
      SampleValue /= NumSamples;

      // Add sample to analysis window
      if (SampleIdx % InDownsamplingFactor == 0)
      {
        NumSamplesPushed += InOutWindow.PushSingle(SampleValue);
      }
    }

    // Set analysis window read index to new write index (now that
    // we've pushed samples), so that we're always reading an entire
    // window's worth of samples
    InOutWindow.AlignReadWriteIndices();

    return NumSamplesPushed;
  }
}
