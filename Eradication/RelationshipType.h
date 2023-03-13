/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include "SimulationEnums.h"

namespace Kernel
{
    std::set<std::string> GetAllowableRelationshipTypes();
    std::vector<RelationshipType::Enum> GetRelationshipTypes();
    std::vector<RelationshipType::Enum> ConvertStringsToRelationshipTypes( const std::string& rParamName,
                                                                           const std::vector<std::string>& rStrings );
}
