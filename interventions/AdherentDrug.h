/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include "AntiMalarialDrug.h"
#include "EventTrigger.h"
#include "InterpolatedValueMap.h"
#include "InterventionEnums.h"

namespace Kernel
{
    struct IWaningEffect;

    class AdherentDrug : public AntimalarialDrug
    {
        DECLARE_FACTORY_REGISTERED( InterventionFactory, AdherentDrug, IDistributableIntervention )

    public:
        AdherentDrug();
        AdherentDrug( const AdherentDrug& rOrig );
        virtual ~AdherentDrug();

        virtual bool Configure( const Configuration * ) override;

        // ISupports
        virtual QueryResult QueryInterface( iid_t iid, void **ppvObject ) override;

        // GenericDrug method
        virtual void Update( float dt ) override;
        virtual void SetContextTo( IIndividualHumanContext *context ) override;
        virtual bool IsTakingDose( float dt ) override;

    protected:
        virtual void  ConfigureDrugTreatment( IIndividualHumanInterventionsContext * ivc ) override;
        NonAdherenceOptionsType::Enum AdherentDrug::SelectNonAdherenceOption();

        IWaningEffect* m_pAdherenceEffect;
        InterpolatedValueMap m_DoseEffects;

        std::vector<NonAdherenceOptionsType::Enum> m_NonAdherenceOptions;
        std::vector<float>                         m_NonAdherenceCdf;

        EventTrigger::Enum m_TookDoseEvent;
        float m_MaxDuration;
        float m_CurrentDuration;
        int m_CurrentDose;
        int m_TotalDoses;

        DECLARE_SERIALIZABLE( AdherentDrug );
    };
}
