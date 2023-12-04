// Circular audio buffer class.
// Adapted from Game Audio Programming 3 Chapter 7 by Ethan Geller.
// Edits by David Su prefaced with "DS"

#pragma once

#undef min
#undef max

#include <algorithm>
#include <atomic>
#include <vector>


// Circular audio buffer class.
template <typename SampleType>
class CircularAudioBuffer
{

  // See Section 7.5.1 of Game Audio Programming Vol 3 for full
  // implementation.
private:
  std::vector<SampleType> InternalBuffer;
  uint32_t Capacity;
  std::atomic<uint32_t> ReadCounter;
  std::atomic<uint32_t> WriteCounter;

public:
  CircularAudioBuffer()
  {
    SetCapacity(0);
  }

  CircularAudioBuffer(uint32_t InCapacity)
  {
    SetCapacity(InCapacity);
  }

  void SetCapacity(uint32_t InCapacity)
  {
    Capacity = InCapacity + 1;
    ReadCounter.store(0);
    WriteCounter.store(0);
    InternalBuffer.resize(Capacity);
  }

  // Pushes some amount of samples into this circular buffer.
  // Returns the amount of samples read
  uint32_t Push(const SampleType* InBuffer, uint32_t NumSamples)
  {
    SampleType* DestBuffer = InternalBuffer.data();
    const uint32_t ReadIndex = ReadCounter.load();
    const uint32_t WriteIndex = WriteCounter.load();

    uint32_t NumToCopy = std::min(NumSamples, Remainder());
    const uint32_t NumToWrite =
      std::min(NumToCopy, Capacity - WriteIndex);
    memcpy(
      &DestBuffer[WriteIndex],
      InBuffer,
      NumToWrite * sizeof(SampleType));

    memcpy(
      &DestBuffer[0],
      &InBuffer[NumToWrite],
      (NumToCopy - NumToWrite) * sizeof(SampleType));

    WriteCounter.store((WriteIndex + NumToCopy) % Capacity);

    return NumToCopy;   
  }

  // Same as Pop() but does not increment the read counter.
  uint32_t Peek(SampleType* OutBuffer, uint32_t NumSamples) const
  {
    const SampleType* SrcBuffer = static_cast<const SampleType*>(InternalBuffer.data()); // DS: Added const SampleType* cast
    const uint32_t ReadIndex = ReadCounter.load(); // DS: Replaced uint32 with uint32_t
    const uint32_t WriteIndex = WriteCounter.load(); // DS: Replaced uint32 with uint32_t

    uint32_t NumToCopy = std::min(NumSamples, Num());

    const uint32_t NumRead = std::min(NumToCopy, Capacity - ReadIndex); // DS: Replaced int32 with uint32_t
    memcpy(
      OutBuffer,
      &SrcBuffer[ReadIndex],
      NumRead * sizeof(SampleType));

    memcpy(
      &OutBuffer[NumRead],
      &SrcBuffer[0],
      (NumToCopy - NumRead) * sizeof(SampleType));

    return NumToCopy;
  }

  // Pops some amount of samples into this circular buffer.
  // Returns the amount of samples read.
  uint32_t Pop(SampleType* OutBuffer, uint32_t NumSamples)
  {
    uint32_t NumSamplesRead = Peek(OutBuffer, NumSamples);
    
    ReadCounter.store(
      (ReadCounter.load() + NumSamplesRead) % Capacity);

    return NumSamplesRead;
  }

  // When called, seeks the read or write cursor to only
  // retain either the NumSamples latest data (if
  // bRetainOldestSamples is false) or the NumSamples oldest data
  // (if bRetainOldestSamples is true) in the buffer. Cannot be
  // used to increase the capacity of this buffer.
  void SetNum(uint32_t NumSamples, bool bRetainOldestSamples = false)
  {
    if (bRetainOldestSamples)
    {
      WriteCounter.store(
        (ReadCounter.load() + NumSamples) % Capacity); // DS: Replace GetValue() with load()
    }
    else
    {
      int64_t ReadCounterNum =
        ((uint32_t)WriteCounter.load()) - ((uint32_t)NumSamples); // DS: Replace int32 with uint32_t
      if (ReadCounterNum < 0)
      {
        ReadCounterNum = Capacity + ReadCounterNum;
      }

      ReadCounter.store(static_cast<uint32_t>(ReadCounterNum)); // DS: Add explicit cast to uint32_t
    }
  }

  // Get the number of samples that can be popped off of the buffer.
  uint32_t Num() const
  {
    const uint32_t ReadIndex = ReadCounter.load();
    const uint32_t WriteIndex = WriteCounter.load();

    if (WriteIndex >= ReadIndex)
    {
      return WriteIndex - ReadIndex;
    }
    else
    {
      return Capacity - ReadIndex + WriteIndex;
    }
  }

  // Get the current capacity of the buffer
  uint32_t GetCapacity() const
  {
    return Capacity;
  }

  // Get the number of samples that can be pushed onto the
  // buffer before it is full.
  uint32_t Remainder() const
  {
    const uint32_t ReadIndex = ReadCounter.load();
    const uint32_t WriteIndex = WriteCounter.load();

    return (Capacity - 1 - WriteIndex + ReadIndex) % Capacity;
  }

  // ----------------------------------------------------------------
  // Additional convenience methods
  // (not in original implementation)

  // Get the sample at a specific index offset from the read index
  SampleType At(uint32_t InIndex) const
  {
    const uint32_t ReadIndex = ReadCounter.load();
    const uint32_t SampleIndex = (ReadIndex + InIndex) % Capacity;
    const SampleType SampleValue = InternalBuffer[SampleIndex];
    return SampleValue;
  }

  // Push a single sample to the buffer
  uint32_t PushSingle(const SampleType& InSample)
  {
    return Push(&InSample, 1);
  }

  // Set read index to write index
  void AlignReadWriteIndices()
  {
    SetNum(0, false);
  }

};
