/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include "stdafx.h"
#include "HIVEnums.h"

namespace Kernel
{
    ENUM_INITIALIZE(HIVInfectionStage, IDM_ENUMSPEC_HIVInfectionStage)
    ENUM_INITIALIZE(CD4_Model, IDM_ENUMSPEC_CD4_Model)
    ENUM_INITIALIZE(ARTStatus, IDM_ENUMSPEC_ARTStatus)
    ENUM_INITIALIZE(HIVDrugClass, IDM_ENUMSPEC_HIVDrugClass)
    ENUM_INITIALIZE(ReverseTranscriptaseNucleosideAnalog, IDM_ENUMSPEC_ReverseTranscriptaseNucleosideAnalog)
    ENUM_INITIALIZE(ReceivedTestResultsType, IDM_ENUMSPEC_ReceivedTestResultsType)
}
