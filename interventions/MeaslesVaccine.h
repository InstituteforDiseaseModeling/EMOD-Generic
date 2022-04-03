/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include "MultiEffectVaccine.h"
#include "InterpolatedValueMap.h"

namespace Kernel
{ 
    class MeaslesVaccine : public MultiEffectVaccine
    {
        DECLARE_FACTORY_REGISTERED(InterventionFactory, MeaslesVaccine, IDistributableIntervention)

    public:
        MeaslesVaccine();
        MeaslesVaccine( const MeaslesVaccine& );
        virtual ~MeaslesVaccine();
        virtual int AddRef() override { return BaseIntervention::AddRef(); }
        virtual int Release() override { return BaseIntervention::Release(); }
        virtual bool Configure( const Configuration* pConfig ) override;

        // IDistributableIntervention
        bool ApplyVaccineTake(IIndividualHumanContext * pihc) override;
        virtual void SetContextTo( IIndividualHumanContext *context ) override;
        float TakeVsAge(float age);

        // ISupports
        virtual QueryResult QueryInterface(iid_t iid, void **ppvObject) override;


    protected:
        // context for this intervention--does not need to be reset upon migration, it is just for GiveVaccine()
        //Not sure whether I need to add all of the other vaccine stuff here or if they will be inherited.
        IIndividualHumanContext *parent;

        float existing_antibody_blocking_multiplier;
        InterpolatedValueMap m_take_vs_age_map;

        DECLARE_SERIALIZABLE(MeaslesVaccine);
    };
}
