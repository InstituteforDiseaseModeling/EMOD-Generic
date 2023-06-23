/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"

#include "IndividualVector.h"
#include "SusceptibilityVector.h"
#include "InfectionVector.h"
#include "ITransmissionGroups.h"
#include "Common.h"
#include "Debug.h"
#include "IContagionPopulation.h"
#include "TransmissionGroupMembership.h"
#include "TransmissionGroupsBase.h"
#include "NodeVector.h"
#include "VectorInterventionsContainer.h"
#include "RANDOM.h"

#include "Log.h"

SETUP_LOGGING( "IndividualVector" )

namespace Kernel
{
    TransmissionGroupMembership_t IndividualHumanVector::human_indoor;
    TransmissionGroupMembership_t IndividualHumanVector::human_outdoor;
    TransmissionGroupMembership_t IndividualHumanVector::vector_indoor;
    TransmissionGroupMembership_t IndividualHumanVector::vector_outdoor;

    BEGIN_QUERY_INTERFACE_DERIVED(IndividualHumanVector, IndividualHuman)
        HANDLE_INTERFACE(IIndividualHumanVectorContext)
    END_QUERY_INTERFACE_DERIVED(IndividualHumanVector, IndividualHuman)

    IndividualHumanVector::IndividualHumanVector(suids::suid _suid, double monte_carlo_weight, double initial_age, int gender)
        : Kernel::IndividualHuman(_suid, float(monte_carlo_weight), float(initial_age), gender)
        , m_strain_exposure()
        , m_total_exposure(0.0f)
        , vector_susceptibility(nullptr)
        , vector_interventions(nullptr)
    {
    }

    IndividualHumanVector::IndividualHumanVector(INodeContext *context)
        : Kernel::IndividualHuman(context)
        , m_strain_exposure()
        , m_total_exposure(0.0f)
        , vector_susceptibility(nullptr)
        , vector_interventions(nullptr)
    {
    }

    IndividualHumanVector *IndividualHumanVector::CreateHuman(INodeContext *context, suids::suid id, double MCweight, double init_age, int gender)
    {
        Kernel::IndividualHumanVector *newhuman = _new_ Kernel::IndividualHumanVector(id, MCweight, init_age, gender);

        newhuman->SetContextTo(context);
        LOG_DEBUG_F( "Created human with age=%f\n", newhuman->m_age );

        return newhuman;
    }

    IndividualHumanVector::~IndividualHumanVector()
    {
        // deletion of individuals handled by parent destructor
        // deletion of susceptibility handled by parent destructor
    }

    void IndividualHumanVector::PropagateContextToDependents()
    {
        IndividualHuman::PropagateContextToDependents();

        if( vector_susceptibility == nullptr && susceptibility != nullptr)
        {
            if ( s_OK != susceptibility->QueryInterface(GET_IID(IVectorSusceptibilityContext), (void**)&vector_susceptibility) )
            {
                throw QueryInterfaceException( __FILE__, __LINE__, __FUNCTION__, "susceptibility", "IVectorSusceptibilityContext", "Susceptibility" );
            }
        }
    }

    void IndividualHumanVector::setupInterventionsContainer()
    {
        vector_interventions = _new_ VectorInterventionsContainer();
        interventions = vector_interventions; // initialize base class pointer to same object
    }

    void IndividualHumanVector::CreateSusceptibility(float imm_mod, float risk_mod)
    {
        SusceptibilityVector *newsusceptibility = SusceptibilityVector::CreateSusceptibility(this, imm_mod, risk_mod);
        vector_susceptibility = newsusceptibility;
        susceptibility = newsusceptibility; // initialize base class pointer to same object

        susceptibility->SetContextTo(this);
    }


    void IndividualHumanVector::ExposeToInfectivity(float dt)
    {
        // Reset counters
        m_strain_exposure.clear();
        m_total_exposure = 0;

        // Expose individual to all pools in weighted collection (i.e. indoor + outdoor)
        LOG_VALID("Exposure to contagion: vector to human.\n");
        parent->ExposeIndividual( this, transmissionGroupMembershipByRoute[TransmissionRoute::CONTACT], dt);

        // Decide based on total exposure to infectious bites
        // whether the individual becomes infected and with what strain
        if ( m_total_exposure > 0 )
        {
            ApplyTotalBitingExposure();
        }
    }

    void IndividualHumanVector::UpdateGroupPopulation(float size_changes)
    {
        // Update nodepool population for both human-vector and vector-human since we use the same normalization for both
        float host_vector_weight = float(GetMonteCarloWeight() * GetRelativeBitingRate());
        parent->UpdateTransmissionGroupPopulation({{"indoor","human"},{"outdoor","human"}}, size_changes, host_vector_weight);
        LOG_DEBUG_F("updated population for both human and vector, with size change %f and monte carlo weight %f.\n", size_changes, host_vector_weight);
    }

    void IndividualHumanVector::ApplyTotalBitingExposure()
    {
        // Make random draw whether to acquire new infection
        // dt incorporated already in ExposeIndividual function arguments
        float acquisition_probability = float(EXPCDF(-m_total_exposure));
        if ( GetRng()->e() >= acquisition_probability ) return;
            
        // Choose a strain based on a weighted draw over values from all vector-to-human pools
        float strain_cdf_draw = GetRng()->e() * m_total_exposure;
        std::vector<strain_exposure_t>::iterator it = std::lower_bound( m_strain_exposure.begin(), m_strain_exposure.end(), strain_cdf_draw, compare_strain_exposure_float_less());
        TransmissionGroupsBase::ContagionPopulationImpl contPop( &(it->first), (it->second) );
        LOG_DEBUG_F( "Mosquito->Human infection transmission based on total exposure %f. Existing infections = %d.\n", m_total_exposure, GetInfections().size() );
        IndividualHuman::AcquireNewInfection(&contPop, TransmissionRoute::CONTACT, -1.0f);
    }

    void IndividualHumanVector::Expose( const IContagionPopulation* cp, float dt, TransmissionRoute::Enum tx_route )
    {
        release_assert( cp );
        release_assert( susceptibility );
        release_assert( interventions );
#if 1
        // get rid of this. but seems to be needed for malaria garki. :( :( :(
        if( !vector_interventions )
        {
            vector_interventions = static_cast<VectorInterventionsContainer*>(interventions);
        }
#endif
        release_assert( vector_interventions );
        float acqmod = GetRelativeBitingRate() * susceptibility->getModAcquire() * interventions->GetInterventionReducedAcquire(tx_route);

        switch( tx_route )
        {
            case TransmissionRoute::VECTOR_TO_HUMAN_INDOOR:
                acqmod *= vector_interventions->GetblockIndoorVectorAcquire();
                break;

            case TransmissionRoute::VECTOR_TO_HUMAN_OUTDOOR:
                acqmod *= vector_interventions->GetblockOutdoorVectorAcquire();
                break;
        
            default:
                throw BadEnumInSwitchStatementException( __FILE__, __LINE__, __FUNCTION__, "transmission_route", tx_route, TransmissionRoute::pairs::lookup_key( tx_route ) );
        }

        // Accumulate vector of pairs of strain ids and cumulative infection probability
        float infection_probability = cp->GetTotalContagion() * acqmod * dt;
        if ( infection_probability > 0 )
        {
            // Increment total exposure
            m_total_exposure += infection_probability;

            // With a weighted random draw, pick a strain from the ContagionPopulation CDF
            StrainIdentity strain_id;
            cp->ResolveInfectingStrain(&strain_id);

            // Push this exposure and strain back to the storage array for all vector-to-human pools (e.g. indoor, outdoor)
            m_strain_exposure.push_back( std::make_pair(strain_id, m_total_exposure) );
        }
    }

    void IndividualHumanVector::UpdateInfectiousness(float dt)
    {
        infectiousness = 0;
        float inf_mod_trans = 0.0f;
        float tmp_inf, tmp_iv_mod;

        std::map<StrainIdentity, float> inf_mod_trans_by_strain;
        StrainIdentity tmp_strainIDs;

        // Loop once over all infections, caching strains and infectivity.
        // If total infectiousness exceeds unity, we will normalize all strains down accordingly.
        for (auto infection : infections)
        {
            release_assert( infection );
            release_assert( interventions );

            tmp_inf    = infection->GetInfectiousness();
            tmp_iv_mod = interventions->GetInterventionReducedTransmit(infection->GetSourceRoute());

            infectiousness += tmp_inf;
            inf_mod_trans  += tmp_inf * tmp_iv_mod;

            if ( tmp_inf > 0 )
            {
                infection->GetInfectiousStrainID(&tmp_strainIDs);
                inf_mod_trans_by_strain[tmp_strainIDs] += tmp_inf * tmp_iv_mod;
            }
        }

        // This optimization seems to make sense. Works for Dengue. Need to test for Vector/Malaria of course.
        if( infectiousness == 0 )
        {
            return;
        }

        // Maximum individual infectiousness is set here, capping the sum of unmodified infectivity at prob=1
        release_assert( susceptibility );
        float truncate_infectious_mod = (infectiousness > 1 ) ? 1.0f/infectiousness : 1.0f;
        float tmp_sus_mod = susceptibility->getModTransmit();
        infectiousness = inf_mod_trans * truncate_infectious_mod * tmp_sus_mod;

        // Host weight is the product of MC weighting and relative biting
        float host_vector_weight = float(GetMonteCarloWeight() * GetRelativeBitingRate());

        // Effects from vector intervention container
        IVectorInterventionsEffects* ivie = nullptr;
        if ( s_OK !=  interventions->QueryInterface(GET_IID(IVectorInterventionsEffects), (void**)&ivie) )
        {
            throw QueryInterfaceException( __FILE__, __LINE__, __FUNCTION__, "interventions", "IVectorInterventionsEffects", "IndividualHumanVector" );
        }

        // Loop again over infection strains, depositing (downscaled) infectivity modified by vector intervention effects etc. (indoor + outdoor)
        for (auto& infectivity : inf_mod_trans_by_strain)
        {
            const StrainIdentity *id = &(infectivity.first);
            LOG_DEBUG_F( "Depositing contagion from human to vector (indoor & outdoor) with biting-rate-driven weight of %f and combined modifiers of %f.\n",
                         host_vector_weight, infectivity.second * truncate_infectious_mod * susceptibility->getModTransmit() * ivie->GetblockIndoorVectorTransmit() );
            parent->DepositFromIndividual( *id, host_vector_weight * infectivity.second * truncate_infectious_mod * tmp_sus_mod * ivie->GetblockIndoorVectorTransmit(),  human_indoor,  TransmissionRoute::HUMAN_TO_VECTOR_INDOOR );
            parent->DepositFromIndividual( *id, host_vector_weight * infectivity.second * truncate_infectious_mod * tmp_sus_mod * ivie->GetblockOutdoorVectorTransmit(), human_outdoor, TransmissionRoute::HUMAN_TO_VECTOR_OUTDOOR );
        }
    }

    IInfection *IndividualHumanVector::createInfection(suids::suid _suid)
    {
        return InfectionVector::CreateInfection(this, _suid);
    }
    
    float 
    IndividualHumanVector::GetRelativeBitingRate(void) const
    {
        release_assert( vector_susceptibility );
        return vector_susceptibility->GetRelativeBitingRate();
    }

    void IndividualHumanVector::ReportInfectionState()
    {
        LOG_DEBUG( "ReportInfectionState\n" );
        // Is infection reported this turn?
        // Will only implement delayed reporting (for fever response) later
        // 50% reporting immediately
        if( GetRng()->e() < .5 )
        {
            m_new_infection_state = NewInfectionState::NewAndDetected;
        }
        else
        {
            release_assert( parent );
            m_new_infection_state = NewInfectionState::NewInfection;
        }
    }

    REGISTER_SERIALIZABLE(IndividualHumanVector);

    void IndividualHumanVector::serialize(IArchive& ar, IndividualHumanVector* obj)
    {
        IndividualHumanVector& individual = *obj;

        IndividualHuman::serialize(ar, obj);
        ar.labelElement("m_strain_exposure");
            Kernel::serialize(ar, individual.m_strain_exposure);
        ar.labelElement("m_total_exposure") & individual.m_total_exposure;
    }

    IArchive& serialize(IArchive& ar, std::vector<strain_exposure_t>& vec)
    {
        size_t count = ar.IsWriter() ? vec.size() : 0xDEADBEEF;
        ar.startArray(count);
        if (!ar.IsWriter())
        {
            vec.resize(count);
        }

        for (auto& entry : vec)
        {
            ar.startObject();
            StrainIdentity* strain = &entry.first;
            ar.labelElement("strain"); StrainIdentity::serialize(ar, strain);
            ar.labelElement("weight") & entry.second;
            ar.endObject();
        }
        ar.endArray();

        return ar;
    }
}
