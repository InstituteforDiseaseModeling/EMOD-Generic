/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"
#include "MeaslesVaccine.h"
#include "IndividualEventContext.h"  // IIndividualHumanEventContext
#include "InterventionsContainer.h"  // for IVaccineConsumer methods
#include "IIndividualHuman.h"
#include "IIndividualHumanContext.h"
//#include "Contexts.h"                // for IIndividualHumanContext, IIndividualHumanInterventionsContext, IIndividualHumanEventContext
#include "RANDOM.h"


SETUP_LOGGING( "MeaslesVaccine" )

namespace Kernel
{
    BEGIN_QUERY_INTERFACE_BODY(MeaslesVaccine)
        HANDLE_INTERFACE(IConfigurable)
        HANDLE_INTERFACE(IDistributableIntervention)
        HANDLE_INTERFACE(IVaccine)
        HANDLE_INTERFACE(IBaseIntervention)
        HANDLE_ISUPPORTS_VIA(IDistributableIntervention)
        HANDLE_INTERFACE(IBaseIntervention)
    END_QUERY_INTERFACE_BODY(MeaslesVaccine)

    IMPLEMENT_FACTORY_REGISTERED(MeaslesVaccine)

    bool MeaslesVaccine::Configure(
        const Configuration * inputJson
    )
    {

        initConfigTypeMap("Existing_Antibody_Blocking_Multiplier", &existing_antibody_blocking_multiplier, SV_Vaccine_Take_DESC_TEXT, 0.0, 1.0, 0.0);  //Not in love with this name, it's not super informative.  
        //If a child has existing antibodies, maternal or otherwise, measles vaccine will less likely to "take", at least for now (Long-term, it's not clear whether
        //maternal antibodies should be handled separately from non-maternal immunity, will return to this later.  Does not matter if non-mAb immunity is binary and does not wane).  
        //This float scales the take rate by a user-specified value to account for this.
        initConfigComplexType("Vaccine_Take_Vs_Age_Map", &m_take_vs_age_map, WEM_Durability_Map_End_DESC_TEXT);
        //Define a piecewise linear vaccine take vs. recipient age in days.  Important checks - the two vectors must be of equal length.    

        //MeaslesVaccine need only be an anti-acquisition vaccine, since if the individual can't acquire, they can't transmit or have mortality .  I suppose it could also have effects on M

        WaningConfig acquire_config;
        initConfigComplexType("Acquire_Config",  &acquire_config, MEV_Acquire_Config_DESC_TEXT ); // not really reusing multi-effect base class
        bool configured = BaseIntervention::Configure( inputJson );
        if( configured && !JsonConfigurable::_dryrun )
        {
            acquire_effect   = WaningEffectFactory::CreateInstance( acquire_config );
        }
        return configured;
    }

    MeaslesVaccine::MeaslesVaccine() 
    : MultiEffectVaccine()
    , m_take_vs_age_map( 0.0f, 99999.0f, 0.0f, 1.0f )  
    , existing_antibody_blocking_multiplier( 0.0f )
    {
    }

    MeaslesVaccine::MeaslesVaccine( const MeaslesVaccine& master )
    : MultiEffectVaccine( master )
    , m_take_vs_age_map(0.0f, 99999.0f, 0.0f, 1.0f)
    , existing_antibody_blocking_multiplier(0.0f)
    {
        existing_antibody_blocking_multiplier = master.existing_antibody_blocking_multiplier;
        m_take_vs_age_map = master.m_take_vs_age_map;
        cost_per_unit = master.cost_per_unit;

        /*acquire_effect = master.acquire_effect->Clone();
        transmit_effect = master.transmit_effect->Clone();
        mortality_effect = master.mortality_effect->Clone();*/
    }

    MeaslesVaccine::~MeaslesVaccine()
    {
        /*delete acquire_effect;
        delete transmit_effect;
        delete mortality_effect;*/
    }


    bool MeaslesVaccine::ApplyVaccineTake(IIndividualHumanContext* p_ihc)
    {
        release_assert(p_ihc);

        bool did_vaccine_take = true;

        //Here, we are going to get an age-dependent take rate 
        float age = p_ihc->GetEventContext()->GetAge(); 
        vaccine_take = m_take_vs_age_map.getValueLinearInterpolation(age, 1.0f);

        //Here, existing antibodies will potentially block the take rate even further.
        IIndividualHuman* p_iih = nullptr;
        if( p_ihc->QueryInterface( GET_IID( IIndividualHuman ), (void**)&p_iih ) != s_OK )
        {
            throw QueryInterfaceException( __FILE__, __LINE__, __FUNCTION__, "parent", "IIndividualHuman", "IIndividualHumanContext" );
        }

        float mAcquire = p_iih->GetAcquisitionImmunity(); 
        float take_mod = existing_antibody_blocking_multiplier + mAcquire*(1-existing_antibody_blocking_multiplier);  
        //if mod_acquire is 0 (full immunity), we get the user-specified "blocking multiplier".  This goes to 1 if mod_acquire is 1, 
        vaccine_take = vaccine_take * take_mod;

        return SimpleVaccine::ApplyVaccineTake( p_ihc );
    }

    void MeaslesVaccine::SetContextTo( IIndividualHumanContext *context )
    {
        MultiEffectVaccine::SetContextTo( context );
    }

    REGISTER_SERIALIZABLE(MeaslesVaccine);

    void MeaslesVaccine::serialize(IArchive& ar, MeaslesVaccine* obj)
    {
        MultiEffectVaccine::serialize( ar, obj );
        MeaslesVaccine& vaccine = *obj;
        ar.labelElement("m_take_vs_age_map")                       & vaccine.m_take_vs_age_map;
        ar.labelElement("existing_antibody_blocking_multiplier")   & vaccine.existing_antibody_blocking_multiplier;
    }
}
