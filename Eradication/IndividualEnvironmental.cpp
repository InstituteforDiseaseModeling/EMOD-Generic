/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"

#if defined(ENABLE_ENVIRONMENTAL)

#include "NodeEnvironmental.h"
#include "IndividualEnvironmental.h"
#include "InfectionEnvironmental.h"
#include "SusceptibilityEnvironmental.h"
#include "StrainIdentity.h"

#pragma warning(disable: 4244)

SETUP_LOGGING( "IndividualEnvironmental" )

namespace Kernel
{
    bool IndividualHumanEnvironmentalConfig::Configure( const Configuration* config ) // just called once!
    {
        LOG_DEBUG( "Configure\n" );

        bool ret = JsonConfigurable::Configure( config );
        return ret;
    }

    BEGIN_QUERY_INTERFACE_DERIVED(IndividualHumanEnvironmental, IndividualHuman)
    END_QUERY_INTERFACE_DERIVED(IndividualHumanEnvironmental, IndividualHuman)

    IndividualHumanEnvironmental::IndividualHumanEnvironmental( suids::suid _suid, float monte_carlo_weight, float initial_age, int gender) 
        : IndividualHuman( _suid, monte_carlo_weight, initial_age, gender)
        , exposureRoute( TransmissionRoute::ENVIRONMENTAL )
    {
    }

    IndividualHumanEnvironmental* IndividualHumanEnvironmental::CreateHuman(INodeContext *context, suids::suid id, float MCweight, float init_age, int gender)
    {
        IndividualHumanEnvironmental *newindividual = _new_ IndividualHumanEnvironmental( id, MCweight, init_age, gender);

        newindividual->SetContextTo(context);
        LOG_DEBUG_F( "Created human with age=%f\n", newindividual->m_age );

        return newindividual;
    }

    IndividualHumanEnvironmental::~IndividualHumanEnvironmental()
    {
    }

    void IndividualHumanEnvironmental::CreateSusceptibility(float imm_mod, float risk_mod)
    {
        susceptibility = SusceptibilityEnvironmental::CreateSusceptibility(this, imm_mod, risk_mod);
    }

    void IndividualHumanEnvironmental::ReportInfectionState()
    {
        m_new_infection_state = NewInfectionState::NewInfection; 
    }

    void IndividualHumanEnvironmental::UpdateInfectiousness(float dt)
    {
        infectiousness = 0;
        for (auto infection : infections)
        {
            LOG_DEBUG("Getting infectiousness by route.\n");
            float tmp_infectiousnessFecal = m_mc_weight * infection->GetInfectiousnessByRoute(TransmissionRoute::ENVIRONMENTAL);
            float tmp_infectiousnessOral  = m_mc_weight * infection->GetInfectiousnessByRoute(TransmissionRoute::CONTACT);

            StrainIdentity tmp_strainID;
            infection->GetInfectiousStrainID(&tmp_strainID);
            LOG_DEBUG_F("UpdateInfectiousness: InfectiousnessFecal = %f, InfectiousnessOral = %f.\n", tmp_infectiousnessFecal, tmp_infectiousnessOral);

            //deposit oral to 'contact', fecal to 'environmental' pool
            LOG_DEBUG("Getting routes.\n");

            for(auto& entry : transmissionGroupMembershipByRoute)
            {
                LOG_DEBUG_F("Found route:%s.\n", TransmissionRoute::pairs::lookup_key(entry.first));
                if (entry.first==TransmissionRoute::CONTACT)
                {
                    if (tmp_infectiousnessOral > 0.0f)
                    {
                        LOG_DEBUG_F("Depositing %f to route %s: (clade=%d, substain=%d)\n", tmp_infectiousnessOral, TransmissionRoute::pairs::lookup_key(entry.first), tmp_strainID.GetCladeID(), tmp_strainID.GetGeneticID());
                        parent->DepositFromIndividual( tmp_strainID, tmp_infectiousnessOral, entry.second, TransmissionRoute::CONTACT );
                        infectiousness += infection->GetInfectiousnessByRoute(TransmissionRoute::CONTACT);
                    }
                }
                else if (entry.first==TransmissionRoute::ENVIRONMENTAL)
                {
                    if (tmp_infectiousnessFecal > 0.0f)
                    {
                        LOG_DEBUG_F("Depositing %f to route %s: (clade=%d, substain=%d)\n", tmp_infectiousnessFecal, TransmissionRoute::pairs::lookup_key(entry.first), tmp_strainID.GetCladeID(), tmp_strainID.GetGeneticID());    
                        parent->DepositFromIndividual( tmp_strainID, tmp_infectiousnessFecal, entry.second, TransmissionRoute::ENVIRONMENTAL );
                        infectiousness += infection->GetInfectiousnessByRoute(TransmissionRoute::ENVIRONMENTAL);
                    }
                }
                else
                {
                    LOG_WARN_F("unknown route %s, do not deposit anything.\n", TransmissionRoute::pairs::lookup_key(entry.first));
                }
           }
        }
    }

    void IndividualHumanEnvironmental::UpdateGroupPopulation(float size_changes)
    {
        parent->UpdateTransmissionGroupPopulation(GetProperties()->GetOldVersion(), size_changes, this->GetMonteCarloWeight());
    }

    IInfection* IndividualHumanEnvironmental::createInfection( suids::suid _suid )
    {
        return InfectionEnvironmental::CreateInfection(this, _suid);
    }

    void IndividualHumanEnvironmental::Expose( const IContagionPopulation* cp, float dt, TransmissionRoute::Enum tx_route )
    {
        exposureRoute = tx_route;
        IndividualHuman::Expose( cp, dt, tx_route );
    }

    void IndividualHumanEnvironmental::AcquireNewInfection( const IStrainIdentity *infstrain, float incubation_period_override )
    {
        if ( infstrain )
        {
            StrainIdentity infectingStrain;
            infstrain->ResolveInfectingStrain( &infectingStrain );
            if ( incubation_period_override == 0.0f )
            {
                exposureRoute = TransmissionRoute::OUTBREAK;
            }
            infectingStrain.SetGeneticID(static_cast<int>(exposureRoute));
            IndividualHuman::AcquireNewInfection( &infectingStrain, incubation_period_override );
        }
        else
        {
            IndividualHuman::AcquireNewInfection( infstrain, incubation_period_override );
        }
    }

    REGISTER_SERIALIZABLE(IndividualHumanEnvironmental);

    void IndividualHumanEnvironmental::serialize(IArchive& ar, IndividualHumanEnvironmental* obj)
    {
        IndividualHuman::serialize(ar, obj);
    }
}

#endif // ENABLE_ENVIRONMENTAL
