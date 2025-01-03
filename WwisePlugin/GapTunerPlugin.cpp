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

#include "GapTunerPlugin.h"
#include "../SoundEnginePlugin/GapTunerFXFactory.h"

#include <AK/Tools/Common/AkAllocator.h>
#include <AK/SoundEngine/Common/AkSoundEngine.h>

#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

namespace GapTunerPluginConstants
{
  // GUID string size (e.g. `{1514A4D8-1DA6-412A-A17E-75CA0C2149F3}`)
  constexpr size_t kGuidStringSize = 39;

  // WAQL query string size (e.g. `$ "{1514A4D8-1DA6-412A-A17E-75CA0C2149F3}"`)
  constexpr size_t kGuidWaqlQueryStringSize = kGuidStringSize + 4;

  // Short ID string size (e.g. `1908158473`)
  constexpr size_t kShortIdStringSize = 11;
}

GapTunerPlugin::GapTunerPlugin()
{
}

GapTunerPlugin::~GapTunerPlugin()
{
}

bool GapTunerPlugin::GetBankParameters(
  const GUID & in_guidPlatform,
  AK::Wwise::Plugin::DataWriter& in_dataWriter) const
{
  in_dataWriter.WriteUInt32(m_propertySet.GetUInt32(
    in_guidPlatform, "OutputPitchParameterID"));

  in_dataWriter.WriteUInt32(m_propertySet.GetUInt32(
    in_guidPlatform, "WindowSize"));

  in_dataWriter.WriteUInt32(m_propertySet.GetUInt32(
    in_guidPlatform, "MaxNumKeyMaxima"));

  in_dataWriter.WriteReal32(m_propertySet.GetReal32(
    in_guidPlatform, "KeyMaximaThresholdMultiplier"));

  in_dataWriter.WriteReal32(m_propertySet.GetReal32(
    in_guidPlatform, "ClarityThreshold"));

  in_dataWriter.WriteUInt32(m_propertySet.GetUInt32(
    in_guidPlatform, "DownsamplingFactor"));

  in_dataWriter.WriteUInt32(m_propertySet.GetUInt32(
    in_guidPlatform, "SmoothingRateMs"));

  in_dataWriter.WriteUInt32(m_propertySet.GetUInt32(
    in_guidPlatform, "SmoothingCurve"));

  in_dataWriter.WriteBool(m_propertySet.GetBool(
      in_guidPlatform, "ZeroOutUnpitched"));

  in_dataWriter.WriteUInt32(m_propertySet.GetUInt32(
      in_guidPlatform, "UnpitchedCooldownMs"));

  return true;
}

void GapTunerPlugin::NotifyPropertyChanged(const GUID& in_guidPlatform, 
                                           const char* in_pszPropertyName)
{
  // Only update for output pitch parameter reference
  if (strcmp(in_pszPropertyName, "OutputPitchParameterReference") != 0)
  {
    return;
  }

  // --------
  // 1. Get output pitch parameter reference

  char GuidString[GapTunerPluginConstants::kGuidStringSize];
  bool bSuccess = false;

  bSuccess = GetOutputPitchParamRefGuidString(GuidString);

  // ----
  // 2. Get output pitch parameter short ID

  uint32_t ShortId = 0;
  bSuccess = GetOutputPitchParamShortId(GuidString, ShortId);

  // ----
  // 4. Set output pitch parameter ID (as a parameter)

  bSuccess = SetOutputPitchParamId(GuidString, ShortId);
}

bool GapTunerPlugin::GetOutputPitchParamRefGuidString(char* OutGuidString)
{
  const GUID InGuid = *(m_propertySet.GetID());
  char InGuidString[GapTunerPluginConstants::kGuidStringSize];
  ConvertGuidToString(InGuid, InGuidString);

  std::map<const char*, const char*> ArgStrings;
  std::map<const char*, uint32_t> ArgUInts;
  std::vector<char*> ReturnOptions;
  rapidjson::Value Results;

  ReturnOptions.push_back("@OutputPitchParameterReference");

  // ----

  bool bSuccess = GetWaqlResults("ak.wwise.core.object.get",
                                 InGuidString,
                                 ArgStrings,
                                 ArgUInts,
                                 ReturnOptions,
                                 true,
                                 Results);
  if (!bSuccess)
  {
    return false;
  }

  const auto& OutputRefObject = Results["@OutputPitchParameterReference"];
  const auto OutputRefGuidString = OutputRefObject["id"].GetString();
  strcpy(OutGuidString, OutputRefGuidString);

    return true;
}

bool GapTunerPlugin::GetOutputPitchParamShortId(const char* InGuidString,
                                                uint32_t& OutShortId)
{
  std::map<const char*, const char*> ArgStrings;
  std::map<const char*, uint32_t> ArgUInts;
  std::vector<char*> ReturnOptions;
  rapidjson::Value Results;

  ReturnOptions.push_back("shortid");

  // ----

  bool bSuccess = GetWaqlResults("ak.wwise.core.object.get",
                                 InGuidString,
                                 ArgStrings,
                                 ArgUInts,
                                 ReturnOptions,
                                 true,
                                 Results);
  if (!bSuccess)
  {
    return false;
  }

  OutShortId = Results["shortid"].GetUint();

    return true;
}

bool GapTunerPlugin::SetOutputPitchParamId(const char* InGuidString,
                                           const uint32_t InShortId)
{
  const GUID ObjectGuid = *(m_propertySet.GetID());
  char ObjectGuidString[GapTunerPluginConstants::kGuidStringSize];
  ConvertGuidToString(ObjectGuid, ObjectGuidString);

  std::map<const char*, const char*> ArgStrings;
  std::map<const char*, uint32_t> ArgUInts;
  std::vector<char*> ReturnOptions;
  rapidjson::Value Results;

  char ObjectArgString[] = "object";
  char PropertyArgString[] = "property";
  char ValueArgString[] = "value";

  ArgStrings.insert(
    std::pair<const char*, const char*>("object", ObjectGuidString));
  ArgStrings.insert(
    std::pair<const char*, const char*>("property", "OutputPitchParameterID"));

  ArgUInts.insert(std::pair<const char*, uint32_t>("value", InShortId));

  // ----

  bool bSuccess = GetWaqlResults("ak.wwise.core.object.setProperty",
                                 InGuidString,
                                 ArgStrings,
                                 ArgUInts,
                                 ReturnOptions,
                                 false,
                                 Results);

  return bSuccess;
}

const bool GapTunerPlugin::GetWaqlResults(
  const char* InQuery,
  const char* InGuidString,
  std::map<const char*, const char*> InArgStrings,
  std::map<const char*, uint32_t> InArgUInts,
  std::vector<char*> InReturnOptions,
  bool bInConstructWaql,
  rapidjson::Value& OutResults)
{
  // ----
  // Setup

  // AK
  AK::Wwise::Mallocator Alloc;
  AK::Wwise::SafeAllocator<char> SzResults(&Alloc);
  AK::Wwise::SafeAllocator<char> szError(&Alloc);

  // JSON
  rapidjson::Document Args;
  rapidjson::Document Options;
  rapidjson::Value ReturnOptions;
  rapidjson::Value returnObject;

  Args.SetObject();
  Options.SetObject();
  ReturnOptions.SetArray();

  // ----
  // WAQL query using GUID

  if (bInConstructWaql)
  {
    char WaqlQueryString[GapTunerPluginConstants::kGuidWaqlQueryStringSize];
    ConstructWaqlQueryForGuidString(InGuidString, WaqlQueryString);
    rapidjson::Value WaqlQueryValue(WaqlQueryString, Args.GetAllocator());

    Args.AddMember("waql", WaqlQueryValue, Args.GetAllocator());
  }

  // ----
  // Args

  for (const auto [ArgKey, ArgVal] : InArgStrings)
  {
    rapidjson::Value ArgKeyJson(ArgKey, Args.GetAllocator());
    rapidjson::Value ArgValJson(ArgVal, Args.GetAllocator());

    Args.AddMember(ArgKeyJson, ArgValJson, Args.GetAllocator());
  }

  for (const auto [ArgKey, ArgValUInt] : InArgUInts)
  {
    rapidjson::Value ArgKeyJson(ArgKey, Args.GetAllocator());

    Args.AddMember(ArgKeyJson, ArgValUInt, Args.GetAllocator());
  }

  // ----
  // Return options

  const auto ReturnOptionsSize = InReturnOptions.size();

  if (ReturnOptionsSize > 0)
  { 
    for (size_t i = 0; i < InReturnOptions.size(); ++i)
    {
      const auto Option = InReturnOptions[i];
      rapidjson::Value OptionJson(Option, Options.GetAllocator());
      ReturnOptions.PushBack(OptionJson, Options.GetAllocator());
    }

    Options.AddMember("return", ReturnOptions, Options.GetAllocator());
  }

  // ----
  // JSON string buffers

  rapidjson::StringBuffer ArgsBuffer;
  rapidjson::Writer<rapidjson::StringBuffer> ArgsWriter(ArgsBuffer);
  Args.Accept(ArgsWriter);

  rapidjson::StringBuffer OptionsBuffer;
  rapidjson::Writer<rapidjson::StringBuffer> OptionsWriter(OptionsBuffer);
  Options.Accept(OptionsWriter);

  // ----
  // WAAPI call + results parsing

  m_host.WaapiCall(InQuery,
                   ArgsBuffer.GetString(),
                   OptionsBuffer.GetString(),
                   Alloc,
                   SzResults,
                   szError);

  if (!szError)
  {
    rapidjson::Document FullResults;
    FullResults.Parse(SzResults);

    if (FullResults.HasMember("return"))
    {
      const auto ReturnResults = FullResults["return"].GetArray();

      if (ReturnResults.Capacity() > 0)
      {
        OutResults = ReturnResults[0];
        
        return true;
      }
    }
  }

  return false;
}

void GapTunerPlugin::ConvertGuidToString(const GUID& InGuid, char* OutString)
{
  snprintf(OutString, GapTunerPluginConstants::kGuidStringSize,
           "{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
           InGuid.Data1,
           InGuid.Data2,
           InGuid.Data3,
           InGuid.Data4[0], InGuid.Data4[1], InGuid.Data4[2], InGuid.Data4[3],
           InGuid.Data4[4], InGuid.Data4[5], InGuid.Data4[6], InGuid.Data4[7]);
}

void GapTunerPlugin::ConstructWaqlQueryForGuidString(const char* InGuidString,
                                                     char* OutQuery)
{
  snprintf(OutQuery,
           GapTunerPluginConstants::kGuidWaqlQueryStringSize,
           "$ \"%s\"",
           InGuidString);
}

void GapTunerPlugin::ConstructWaqlQueryForGuid(const GUID& InGuid,
                                               char* OutQuery)
{
  char GuidString[GapTunerPluginConstants::kGuidStringSize];
  ConvertGuidToString(InGuid, GuidString);
  ConstructWaqlQueryForGuidString(GuidString, OutQuery);
}

// Create a PluginContainer structure that contains the info for our plugin
DEFINE_AUDIOPLUGIN_CONTAINER(GapTuner);

// This is a DLL, we want to have a standardized name
EXPORT_AUDIOPLUGIN_CONTAINER(GapTuner);

// Add our CLI class to the PluginContainer
ADD_AUDIOPLUGIN_CLASS_TO_CONTAINER(
    GapTuner,        // Name of the plug-in container for this shared library
    GapTunerPlugin,  // Authoring plug-in class to add to the plug-in container
    GapTunerFX       // Corresponding Sound Engine plug-in class
);
DEFINE_PLUGIN_REGISTER_HOOK

// Placeholder assert hook for Wwise plug-ins using AKASSERT
// (cassert used by default)
DEFINEDUMMYASSERTHOOK;
