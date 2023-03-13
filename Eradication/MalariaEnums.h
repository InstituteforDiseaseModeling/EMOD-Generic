/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include "EnumSupport.h"
#include "Malaria.h"

namespace Kernel
{
    // ENUM defs for MALARIA_MODEL
    #define IDM_ENUMSPEC_MalariaModel                                                    \
        ENUM_VALUE_SPEC(MALARIA_MECHANISTIC_MODEL                           ,  0)        \
        ENUM_VALUE_SPEC(MALARIA_REDUCEDSTATE_MODEL                          ,  1)        \
        ENUM_VALUE_SPEC(MALARIA_EXPONENTIAL_DURATION                        ,  2)        \
        ENUM_VALUE_SPEC(MALARIA_FIXED_DURATION                              ,  4)
    ENUM_DECLARE(MalariaModel, IDM_ENUMSPEC_MalariaModel)


    // ENUM defs for MALARIA_STRAINS
    #define IDM_ENUMSPEC_MalariaStrains                                                  \
        ENUM_VALUE_SPEC(FALCIPARUM_NONRANDOM_STRAIN                         , 11)        \
        ENUM_VALUE_SPEC(FALCIPARUM_RANDOM50_STRAIN                          , 12)        \
        ENUM_VALUE_SPEC(FALCIPARUM_RANDOM_STRAIN                            , 13)        \
        ENUM_VALUE_SPEC(FALCIPARUM_STRAIN_GENERATOR                         , 20)
    ENUM_DECLARE(MalariaStrains, IDM_ENUMSPEC_MalariaStrains)


    // ENUM defs for PARASITE_SWITCH_TYPE
    #define IDM_ENUMSPEC_ParasiteSwitchType                                              \
        ENUM_VALUE_SPEC(CONSTANT_SWITCH_RATE_2VARS                          ,  0)        \
        ENUM_VALUE_SPEC(RATE_PER_PARASITE_7VARS                             ,  1)        \
        ENUM_VALUE_SPEC(RATE_PER_PARASITE_5VARS_DECAYING                    ,  2)
    ENUM_DECLARE(ParasiteSwitchType, IDM_ENUMSPEC_ParasiteSwitchType)


    #define IDM_ENUMSPEC_ClinicalSymptomsEnum                                            \
        ENUM_VALUE_SPEC(CLINICAL_DISEASE                                    ,  0)        \
        ENUM_VALUE_SPEC(SEVERE_DISEASE                                      ,  1)        \
        ENUM_VALUE_SPEC(SEVERE_ANEMIA                                       ,  2)        \
        ENUM_VALUE_SPEC(CLINICAL_SYMPTOMS_COUNT                             ,  3)
    ENUM_DECLARE(ClinicalSymptomsEnum, IDM_ENUMSPEC_ClinicalSymptomsEnum)


    #define IDM_ENUMSPEC_SevereCaseTypesEnum                                             \
        ENUM_VALUE_SPEC(NONE                                                ,  0)        \
        ENUM_VALUE_SPEC(ANEMIA                                              ,  1)        \
        ENUM_VALUE_SPEC(PARASITES                                           ,  2)        \
        ENUM_VALUE_SPEC(FEVER                                               ,  3)
    ENUM_DECLARE(SevereCaseTypesEnum, IDM_ENUMSPEC_SevereCaseTypesEnum)


    #define IDM_ENUMSPEC_MalariaDrugType                                                 \
        ENUM_VALUE_SPEC(Artemisinin                                         ,  1)        \
        ENUM_VALUE_SPEC(Chloroquine                                         ,  2)        \
        ENUM_VALUE_SPEC(Quinine                                             ,  3)        \
        ENUM_VALUE_SPEC(SP                                                  ,  4)        \
        ENUM_VALUE_SPEC(Primaquine                                          ,  5)        \
        ENUM_VALUE_SPEC(Artemether_Lumefantrine                             ,  6)        \
        ENUM_VALUE_SPEC(GenTransBlocking                                    ,  7)        \
        ENUM_VALUE_SPEC(GenPreerythrocytic                                  ,  8)        \
        ENUM_VALUE_SPEC(Tafenoquine                                         ,  9)
    ENUM_DECLARE(MalariaDrugType, IDM_ENUMSPEC_MalariaDrugType)


    // ENUM defs for how maternal antibodies are modeled
    // SIMPLE_WANING draws a PfEMP1 antibody strength is initialized at a fraction of the mother's level and wanes exponentially
    // CONSTANT_INITIAL_IMMUNITY ignores the mother's level (or mean adult population level) of immunity and gives some constant initial immunity level to all children, which wanes exponentially.
    // TODO: EXPLICIT_ANTIBODIES for immune-initialization-style draw at capacity=0 and concentration=mother's level (?)
    #define IDM_ENUMSPEC_MaternalAntibodiesType                                          \
        ENUM_VALUE_SPEC(OFF                                                 ,  0)        \
        ENUM_VALUE_SPEC(SIMPLE_WANING                                       ,  1)        \
        ENUM_VALUE_SPEC(CONSTANT_INITIAL_IMMUNITY                           ,  2)
    ENUM_DECLARE(MaternalAntibodiesType, IDM_ENUMSPEC_MaternalAntibodiesType)


    #define IDM_ENUMSPEC_InnateImmuneVariationType                                       \
        ENUM_VALUE_SPEC(NONE                                                ,  0)        \
        ENUM_VALUE_SPEC(PYROGENIC_THRESHOLD                                 ,  1)        \
        ENUM_VALUE_SPEC(CYTOKINE_KILLING                                    ,  2)        \
        ENUM_VALUE_SPEC(PYROGENIC_THRESHOLD_VS_AGE                          ,  3)
    ENUM_DECLARE(InnateImmuneVariationType, IDM_ENUMSPEC_InnateImmuneVariationType)

    // CSP:    Circumsporozoite protein
    // MSP1:   Merozoite surface protein
    // PfEMP1: Plasmodium falciparum erythrocyte membrane protein (minor non-specific epitopes)
    //                                                            (major epitopes)
    #define IDM_ENUMSPEC_MalariaAntibodyType                                             \
        ENUM_VALUE_SPEC(CSP                                                 ,  0)        \
        ENUM_VALUE_SPEC(MSP1                                                ,  1)        \
        ENUM_VALUE_SPEC(PfEMP1_minor                                        ,  2)        \
        ENUM_VALUE_SPEC(PfEMP1_major                                        ,  3)        \
        ENUM_VALUE_SPEC(N_MALARIA_ANTIBODY_TYPES                            ,  4)
    ENUM_DECLARE(MalariaAntibodyType, IDM_ENUMSPEC_MalariaAntibodyType)


    #define IDM_ENUMSPEC_AsexualCycleStatus                                              \
        ENUM_VALUE_SPEC(NoAsexualCycle                                      ,  0)        \
        ENUM_VALUE_SPEC(AsexualCycle                                        ,  1)        \
        ENUM_VALUE_SPEC(HepatocyteRelease                                   ,  2)
    ENUM_DECLARE(AsexualCycleStatus, IDM_ENUMSPEC_AsexualCycleStatus)


    // 5 stages of development and mature gametocytes
    #define IDM_ENUMSPEC_GametocyteStages                                                \
        ENUM_VALUE_SPEC(Stage0                                              ,  0)        \
        ENUM_VALUE_SPEC(Stage1                                              ,  1)        \
        ENUM_VALUE_SPEC(Stage2                                              ,  2)        \
        ENUM_VALUE_SPEC(Stage3                                              ,  3)        \
        ENUM_VALUE_SPEC(Stage4                                              ,  4)        \
        ENUM_VALUE_SPEC(Mature                                              ,  5)        \
        ENUM_VALUE_SPEC(Count                                               ,  6)
    ENUM_DECLARE(GametocyteStages, IDM_ENUMSPEC_GametocyteStages)
}