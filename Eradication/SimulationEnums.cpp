/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include "stdafx.h"
#include "SimulationEnums.h"

namespace Kernel
{
    ENUM_INITIALIZE(Gender, IDM_ENUMSPEC_Gender)
    ENUM_INITIALIZE(ClimateStructure, IDM_ENUMSPEC_ClimateStructure)
    ENUM_INITIALIZE(ClimateUpdateResolution, IDM_ENUMSPEC_ClimateUpdateResolution)
    ENUM_INITIALIZE(DistributionType, IDM_ENUMSPEC_DistributionType)
    ENUM_INITIALIZE(IndSamplingType, IDM_ENUMSPEC_IndSamplingType)
    ENUM_INITIALIZE(MigrationPattern, IDM_ENUMSPEC_MigrationPattern)
    ENUM_INITIALIZE(MigrationStructure, IDM_ENUMSPEC_MigrationStructure)
    ENUM_INITIALIZE(MigrationType, IDM_ENUMSPEC_MigrationType)
    ENUM_INITIALIZE(GenderDataType, IDM_ENUMSPEC_GenderDataType)
    ENUM_INITIALIZE(InterpolationType, IDM_ENUMSPEC_InterpolationType)
    ENUM_INITIALIZE(ModiferEquationType, IDM_ENUMSPEC_ModiferEquationType)
    ENUM_INITIALIZE(MortalityTimeCourse, IDM_ENUMSPEC_MortalityTimeCourse)
    ENUM_INITIALIZE(PKPDModel, IDM_ENUMSPEC_PKPDModel)
    ENUM_INITIALIZE(RandomNumberGeneratorType, IDM_ENUMSPEC_RandomNumberGeneratorType)
    ENUM_INITIALIZE(RandomNumberGeneratorPolicy, IDM_ENUMSPEC_RandomNumberGeneratorPolicy)
    ENUM_INITIALIZE(MaternalProtectionType, IDM_ENUMSPEC_MaternalProtectionType)
    ENUM_INITIALIZE(SusceptibilityType, IDM_ENUMSPEC_SusceptibilityType)
    ENUM_INITIALIZE(SimType, IDM_ENUMSPEC_SimType)
    ENUM_INITIALIZE(TransmissionRoute, IDM_ENUMSPEC_TransmissionRoute)
    ENUM_INITIALIZE(TransmissionGroupType, IDM_ENUMSPEC_TransmissionGroupType)
    ENUM_INITIALIZE(VitalBirthDependence, IDM_ENUMSPEC_VitalBirthDependence)
    ENUM_INITIALIZE(VitalDeathDependence, IDM_ENUMSPEC_VitalDeathDependence)
    ENUM_INITIALIZE(DistributionFunction, IDM_ENUMSPEC_DistributionFunction)
    ENUM_INITIALIZE(WEBoxDist, IDM_ENUMSPEC_WEBoxDist)
    ENUM_INITIALIZE(ThresholdType, IDM_ENUMSPEC_ThresholdType)
    ENUM_INITIALIZE(SerializationType, IDM_ENUMSPEC_SerializationType)
    ENUM_INITIALIZE(SerializationTypeRead, IDM_ENUMSPEC_SerializationTypeRead)
    ENUM_INITIALIZE(SerializationTypeWrite, IDM_ENUMSPEC_SerializationTypeWrite)
    ENUM_INITIALIZE(SerializationPrecision, IDM_ENUMSPEC_SerializationPrecision)
    ENUM_INITIALIZE(PolioSerotypes, IDM_ENUMSPEC_PolioSerotypes)
    ENUM_INITIALIZE(PolioVirusTypes, IDM_ENUMSPEC_PolioVirusTypes)
    ENUM_INITIALIZE(PolioVaccines, IDM_ENUMSPEC_PolioVaccines)
    ENUM_INITIALIZE(PolioVaccineType, IDM_ENUMSPEC_PolioVaccineType)
    ENUM_INITIALIZE(EvolutionPolioClockType, IDM_ENUMSPEC_EvolutionPolioClockType)
    ENUM_INITIALIZE(VDPVVirulenceModelType, IDM_ENUMSPEC_VDPVVirulenceModelType)
    ENUM_INITIALIZE(RiskGroup, IDM_ENUMSPEC_RiskGroup)
    ENUM_INITIALIZE(AssortivityGroup, IDM_ENUMSPEC_AssortivityGroup)
    ENUM_INITIALIZE(FormationRateType, IDM_ENUMSPEC_FormationRateType)
    ENUM_INITIALIZE(RelationshipType, IDM_ENUMSPEC_RelationshipType)
    ENUM_INITIALIZE(RelationshipState, IDM_ENUMSPEC_RelationshipState)
    ENUM_INITIALIZE(RelationshipMigrationAction, IDM_ENUMSPEC_RelationshipMigrationAction)
    ENUM_INITIALIZE(RelationshipTerminationReason, IDM_ENUMSPEC_RelationshipTerminationReason)
    ENUM_INITIALIZE(ExtraRelationalFlagType, IDM_ENUMSPEC_ExtraRelationalFlagType)
    ENUM_INITIALIZE(TBDrugType, IDM_ENUMSPEC_TBDrugType)
    ENUM_INITIALIZE(TBFastProgressorType, IDM_ENUMSPEC_TBFastProgressorType)
    ENUM_INITIALIZE(TBInfectionState, IDM_ENUMSPEC_TBInfectionState)
    ENUM_INITIALIZE(TargetedDiseaseState, IDM_ENUMSPEC_TargetedDiseaseState)
    ENUM_INITIALIZE(TargetDiseaseStateType, IDM_ENUMSPEC_TargetDiseaseStateType)
}
