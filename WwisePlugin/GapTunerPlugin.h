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

#pragma once

#include <AK/Wwise/Plugin.h>

// STL
#include <map>
#include <vector>

// RapidJSON
#include <rapidjson/document.h>

// See https://www.audiokinetic.com/library/edge/?source=SDK&id=plugin__dll.html
// for the documentation about Authoring plug-ins
class GapTunerPlugin final
  : public AK::Wwise::Plugin::AudioPlugin
  , public AK::Wwise::Plugin::RequestHost
{
public:
  // ----------------

  GapTunerPlugin();
  ~GapTunerPlugin();

  // ----------------

  // This function is called by Wwise to obtain parameters that will be written to a bank.
  // Because these can be changed at run-time, the parameter block should stay relatively small.
  // Larger data should be put in the Data Block.
  bool GetBankParameters(const GUID & in_guidPlatform, AK::Wwise::Plugin::DataWriter& in_dataWriter) const override;

  // Callback function for when a plugin property is changed -- in our case we
  // set the output pitch parameter ID when the output pitch parameter
  // reference gets updated.
  void NotifyPropertyChanged(const GUID& in_guidPlatform, const char* in_pszPropertyName) override;

private:

  // ----------------

  // Get the GUID string for the output pitch parameter reference
  bool GetOutputPitchParamRefGuidString(char* OutGuidString);

  // Get the short ID for the output pitch parameter
  bool GetOutputPitchParamShortId(const char* InGuidString, uint32_t& OutShortId);

  // Set the output pitch parameter ID
  bool SetOutputPitchParamId(const char* InGuidString, const uint32_t InShortId);

  // Get WAQL results for a set of query arguments and options
  const bool GetWaqlResults(const char* InQuery,
                            const char* InGuidString,
                            std::map<const char*, const char*> InArgStrings,
                            std::map<const char*, uint32_t> InArgUInts,
                            std::vector<char*> InReturnOptions,
                            bool bInConstructWaql,
                            rapidjson::Value& OutResults);

  // Convert a GUID to string
  static void ConvertGuidToString(const GUID& InGuid, char* OutString);

  // Create a WAQL query for a given GUID string
  static void ConstructWaqlQueryForGuidString(const char* InGuidString, char* OutQuery);

  // Create a WAQL query for a given GUID
  static void ConstructWaqlQueryForGuid(const GUID& InGuid, char* OutQuery);

};

DECLARE_AUDIOPLUGIN_CONTAINER(GapTuner);  // Exposes our PluginContainer structure that contains the info for our plugin
