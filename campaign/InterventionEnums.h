/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include "EnumSupport.h"

namespace Kernel
{
    #define IDM_ENUMSPEC_TargetDemographicType                                           \
        ENUM_VALUE_SPEC(Everyone                         ,  1)                           \
        ENUM_VALUE_SPEC(ExplicitAgeRanges                ,  2)                           \
        ENUM_VALUE_SPEC(ExplicitAgeRangesAndGender       ,  3)                           \
        ENUM_VALUE_SPEC(ExplicitGender                   ,  4)                           \
        ENUM_VALUE_SPEC(PossibleMothers                  ,  5)                           \
        ENUM_VALUE_SPEC(ArrivingAirTravellers            ,  6)                           \
        ENUM_VALUE_SPEC(DepartingAirTravellers           ,  7)                           \
        ENUM_VALUE_SPEC(ArrivingRoadTravellers           ,  8)                           \
        ENUM_VALUE_SPEC(DepartingRoadTravellers          ,  9)                           \
        ENUM_VALUE_SPEC(AllArrivingTravellers            , 10)                           \
        ENUM_VALUE_SPEC(AllDepartingTravellers           , 11)                           \
        ENUM_VALUE_SPEC(ExplicitDiseaseState             , 12)                           \
        ENUM_VALUE_SPEC(ExplicitIDs                      , 13)
    ENUM_DECLARE(TargetDemographicType, IDM_ENUMSPEC_TargetDemographicType)


    #define IDM_ENUMSPEC_TargetGroupType                                                 \
        ENUM_VALUE_SPEC(Everyone                         ,  1)                           \
        ENUM_VALUE_SPEC(Infected                         ,  2)                           \
        ENUM_VALUE_SPEC(ActiveInfection                  ,  3)                           \
        ENUM_VALUE_SPEC(LatentInfection                  ,  4)                           \
        ENUM_VALUE_SPEC(MDR                              ,  5)                           \
        ENUM_VALUE_SPEC(TreatmentNaive                   ,  6)                           \
        ENUM_VALUE_SPEC(HasFailedTreatment               ,  7)                           \
        ENUM_VALUE_SPEC(HIVNegative                      ,  8)                           \
        ENUM_VALUE_SPEC(ActiveHadTreatment               ,  9)
    ENUM_DECLARE(TargetGroupType, IDM_ENUMSPEC_TargetGroupType)


    #define IDM_ENUMSPEC_TargetGender                                                    \
        ENUM_VALUE_SPEC(All                              ,  0)                           \
        ENUM_VALUE_SPEC(Male                             ,  1)                           \
        ENUM_VALUE_SPEC(Female                           ,  2)
    ENUM_DECLARE(TargetGender, IDM_ENUMSPEC_TargetGender)


    #define IDM_ENUMSPEC_EventOrConfig                                                   \
        ENUM_VALUE_SPEC(Config                           ,  1)                           \
        ENUM_VALUE_SPEC(Event                            ,  2)
    ENUM_DECLARE(EventOrConfig, IDM_ENUMSPEC_EventOrConfig)


    #define IDM_ENUMSPEC_TBHIVInfectionType                                              \
        ENUM_VALUE_SPEC(HIV                              ,  0)                           \
        ENUM_VALUE_SPEC(TB                               ,  1)
    ENUM_DECLARE(TBHIVInfectionType, IDM_ENUMSPEC_TBHIVInfectionType)


    // Adherent Drug
    #define IDM_ENUMSPEC_NonAdherenceOptionsType                                         \
        ENUM_VALUE_SPEC(NEXT_UPDATE                      ,  1)                           \
        ENUM_VALUE_SPEC(NEXT_DOSAGE_TIME                 ,  2)                           \
        ENUM_VALUE_SPEC(LOST_TAKE_NEXT                   ,  3)                           \
        ENUM_VALUE_SPEC(STOP                             ,  4)
    ENUM_DECLARE(NonAdherenceOptionsType, IDM_ENUMSPEC_NonAdherenceOptionsType)


    // AntiMalarialDrug
    #define IDM_ENUMSPEC_DrugUsageType                                                   \
        ENUM_VALUE_SPEC(SingleDose                       ,  1)                           \
        ENUM_VALUE_SPEC(FullTreatmentCourse              ,  2)                           \
        ENUM_VALUE_SPEC(Prophylaxis                      ,  3)                           \
        ENUM_VALUE_SPEC(SingleDoseWhenSymptom            ,  4)                           \
        ENUM_VALUE_SPEC(FullTreatmentWhenSymptom         ,  5)                           \
        ENUM_VALUE_SPEC(SingleDoseParasiteDetect         ,  6)                           \
        ENUM_VALUE_SPEC(FullTreatmentParasiteDetect      ,  7)                           \
        ENUM_VALUE_SPEC(SingleDoseNewDetectionTech       ,  8)                           \
        ENUM_VALUE_SPEC(FullTreatmentNewDetectionTech    ,  9)
    ENUM_DECLARE(DrugUsageType, IDM_ENUMSPEC_DrugUsageType)


    // BroadcastEventToOtherNodes
    #define IDM_ENUMSPEC_NodeSelectionType                                               \
        ENUM_VALUE_SPEC(DISTANCE_ONLY                    ,  0)                           \
        ENUM_VALUE_SPEC(MIGRATION_NODES_ONLY             ,  1)                           \
        ENUM_VALUE_SPEC(DISTANCE_AND_MIGRATION           ,  2)
    ENUM_DECLARE(NodeSelectionType, IDM_ENUMSPEC_NodeSelectionType)


    // InterventionForCurrentPartners
    // NO_PRIORITIZATION             - All partners are contacted
    // CHOSEN_AT_RANDOM              - Partners are randomly selected until Maximum_Partners have received the intervention
    // LONGER_TIME_IN_RELATIONSHIP   - Partners are sorted in descending order of the duration of the relationship. 
    //                                 Partners are contacted from the beginning of this list until Maximum_Partners have received the intervention.
    // SHORTER_TIME_IN_RELATIONSHIP  - Same thing but ascending order.
    // OLDER_AGE                     - Same thing but descending order based on age.
    // YOUNGER_AGE                   - Same thing but ascending order based on age.
    // RELATIONSHIP_TYPE             - In this case, the partners are sorted based on the order of the relationship types defined in the Relationship_Types parameter. 
    //                                 For example, "Relationship_Types":["MARITAL", "INFORMAL","TRANSITORY","COMMERCIAL"],
    //                                 will prioritize marital > informal > transitory > commercial, selecting at random between multiple partners of the same type.
    #define IDM_ENUMSPEC_PartnerPrioritizationType                                       \
        ENUM_VALUE_SPEC(NO_PRIORITIZATION                ,  0)                           \
        ENUM_VALUE_SPEC(CHOSEN_AT_RANDOM                 ,  1)                           \
        ENUM_VALUE_SPEC(LONGER_TIME_IN_RELATIONSHIP      ,  2)                           \
        ENUM_VALUE_SPEC(SHORTER_TIME_IN_RELATIONSHIP     ,  3)                           \
        ENUM_VALUE_SPEC(OLDER_AGE                        ,  4)                           \
        ENUM_VALUE_SPEC(YOUNGER_AGE                      ,  5)                           \
        ENUM_VALUE_SPEC(RELATIONSHIP_TYPE                ,  6)
    ENUM_DECLARE(PartnerPrioritizationType, IDM_ENUMSPEC_PartnerPrioritizationType)


    // MalariaChallenge
    #define IDM_ENUMSPEC_MalariaChallengeType                                            \
        ENUM_VALUE_SPEC(InfectiousBites                  ,  1)                           \
        ENUM_VALUE_SPEC(Sporozoites                      ,  2)
    ENUM_DECLARE(MalariaChallengeType, IDM_ENUMSPEC_MalariaChallengeType)


    // MalariaDiagnostic
    #define IDM_ENUMSPEC_MalariaDiagnosticType                                           \
        ENUM_VALUE_SPEC(Microscopy                       ,  1)                           \
        ENUM_VALUE_SPEC(NewDetectionTech                 ,  2)                           \
        ENUM_VALUE_SPEC(Other                            ,  3)
    ENUM_DECLARE(MalariaDiagnosticType, IDM_ENUMSPEC_MalariaDiagnosticType)


    // NodeLevelHealthTriggeredIVScaleUpSwitch
    #define IDM_ENUMSPEC_ScaleUpProfile                                                  \
        ENUM_VALUE_SPEC(Immediate                        ,  1)                           \
        ENUM_VALUE_SPEC(Linear                           ,  2)                           \
        ENUM_VALUE_SPEC(InterpolationMap                 ,  3)
    ENUM_DECLARE(ScaleUpProfile, IDM_ENUMSPEC_ScaleUpProfile)


    //TBHIVConfigurableTBdrug
    #define IDM_ENUMSPEC_TBHIVConfigurabeDrugState                                       \
        ENUM_VALUE_SPEC(ActiveMDR                        ,  0)                           \
        ENUM_VALUE_SPEC(LatentMDR                        ,  1)                           \
        ENUM_VALUE_SPEC(ActiveHIVPosOffART               ,  2)                           \
        ENUM_VALUE_SPEC(LatentHIVPosOffART               ,  3)                           \
        ENUM_VALUE_SPEC(ActiveHIVNegorPosOnART           ,  4)                           \
        ENUM_VALUE_SPEC(LatentHIVNegorPosOnART           ,  5)                           \
        ENUM_VALUE_SPEC(TBHIVCONFIGURABLEDRUGSTATECOUNT  ,  6)
    ENUM_DECLARE(TBHIVConfigurabeDrugState, IDM_ENUMSPEC_TBHIVConfigurabeDrugState)


    // TyphoidVaccine, TyphoidWASH
    #define IDM_ENUMSPEC_TyphoidVaccineMode                                              \
        ENUM_VALUE_SPEC(Shedding                         ,  1)                           \
        ENUM_VALUE_SPEC(Dose                             ,  2)                           \
        ENUM_VALUE_SPEC(Exposures                        ,  3)
    ENUM_DECLARE(TyphoidVaccineMode, IDM_ENUMSPEC_TyphoidVaccineMode)


    // Route aware intervention: Vaccine
    #define IDM_ENUMSPEC_IVRoute                                                         \
        ENUM_VALUE_SPEC(ALL                              ,  0)                           \
        ENUM_VALUE_SPEC(CONTACT                          ,  1)                           \
        ENUM_VALUE_SPEC(ENVIRONMENTAL                    ,  2)
    ENUM_DECLARE(IVRoute, IDM_ENUMSPEC_IVRoute)


    // VectorControlNodeTargeted
    #define IDM_ENUMSPEC_SpaceSprayTarget                                                \
        ENUM_VALUE_SPEC(SpaceSpray_FemalesOnly           , 11)                           \
        ENUM_VALUE_SPEC(SpaceSpray_MalesOnly             , 12)                           \
        ENUM_VALUE_SPEC(SpaceSpray_FemalesAndMales       , 13)                           \
        ENUM_VALUE_SPEC(SpaceSpray_Indoor                , 14)
    ENUM_DECLARE(SpaceSprayTarget, IDM_ENUMSPEC_SpaceSprayTarget)


    // VectorControlNodeTargeted
    #define IDM_ENUMSPEC_ArtificialDietTarget                                            \
        ENUM_VALUE_SPEC(AD_WithinVillage                 , 21)                           \
        ENUM_VALUE_SPEC(AD_OutsideVillage                , 22)
    ENUM_DECLARE(ArtificialDietTarget, IDM_ENUMSPEC_ArtificialDietTarget)
}
