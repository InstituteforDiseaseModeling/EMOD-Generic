/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"
#include "TyphoidVaccine.h"
#include "InterventionsContainer.h" 
#include "NodeEventContext.h"
#include "Individual.h"
#include "IdmDateTime.h"
#include "INodeContext.h"

SETUP_LOGGING( "TyphoidVaccine" )

namespace Kernel
{
    BEGIN_QUERY_INTERFACE_BODY(TyphoidVaccine)
        HANDLE_INTERFACE(IConfigurable)
        HANDLE_INTERFACE(IDistributableIntervention)
        HANDLE_INTERFACE(IBaseIntervention)
        HANDLE_ISUPPORTS_VIA(IDistributableIntervention)
        HANDLE_INTERFACE(IBaseIntervention)
    END_QUERY_INTERFACE_BODY(TyphoidVaccine)

    IMPLEMENT_FACTORY_REGISTERED(TyphoidVaccine)

    TyphoidVaccine::TyphoidVaccine() 
        : changing_effect( nullptr )
        , effect( 1.0f )
        , vaccine_mode( TyphoidVaccineMode::Shedding )
    {
        initSimTypes( 1, "TYPHOID_SIM" );
    }

    TyphoidVaccine::TyphoidVaccine( const TyphoidVaccine& master )
        : changing_effect( nullptr )
        , effect( master.effect )
        , vaccine_mode( master.vaccine_mode )
    {
        if(master.changing_effect)
        {
            changing_effect = master.changing_effect->Clone();
        }
        else
        {
            LOG_INFO_F( "Looks like we're just going with fixed-value effect and not a variable-over-time effect structure.\n" );
        }
    }

    TyphoidVaccine::~TyphoidVaccine()
    {
        delete changing_effect;
        changing_effect = nullptr;
    }

    bool TyphoidVaccine::Configure(const Configuration* inputJson)
    {
        initConfig( "Mode", vaccine_mode, inputJson, MetadataDescriptor::Enum("Mode", TW_Mode_DESC_TEXT, MDD_ENUM_ARGS(TyphoidVaccineMode)) );

        initConfigTypeMap("Effect",           &effect,                             TW_Effect_DESC_TEXT, 0.0, 1.0, 1.0 );

        changing_effect = WaningEffectFactory::CreateInstance();
        initConfigTypeMap("Changing_Effect",  changing_effect->GetConfigurable(),  TW_CE_DESC_TEXT);

        bool configured = BaseIntervention::Configure( inputJson );

        LOG_DEBUG_F( "Vaccine configured with type %d and effect %f.\n", vaccine_mode, effect );
        return configured;
    }

    bool TyphoidVaccine::Distribute(IIndividualHumanInterventionsContext* context, ICampaignCostObserver* pCCO)
    {
        // Call base distribute first to check eligibility conditions and call SetContextTo
        bool distribute =  BaseIntervention::Distribute( context, pCCO );

        return distribute;
    }

    void TyphoidVaccine::SetContextTo(IIndividualHumanContext* context)
    {
        // Sets parent to context
        BaseIntervention::SetContextTo(context);

        if(changing_effect)
        {
            changing_effect->SetContextTo(context);
        }

        // store itvc for apply
        itvc = context->GetInterventionsContext()->GetContainerTyphoid();
    }

    void TyphoidVaccine::Update( float dt )
    {
        release_assert(itvc);
        auto _effect = effect; // this is bad and confusing. Don't do this. Talking to myself here...
        if( changing_effect )
        {
            changing_effect->Update( dt );
            if(changing_effect->Expired())
            {
                delete changing_effect;
                changing_effect = nullptr;
            }
            else
            {
                _effect = changing_effect->Current();
            }
        }

        auto multiplier = 1.0f-_effect;
        switch( vaccine_mode )
        {
            case TyphoidVaccineMode::Shedding:
            itvc->ApplyReducedSheddingEffect( multiplier );
            break;

            case TyphoidVaccineMode::Dose:
            itvc->ApplyReducedDoseEffect( multiplier );
            break;
        
            case TyphoidVaccineMode::Exposures:
            itvc->ApplyReducedNumberExposuresEffect( multiplier );
            break;

            default:
            throw BadEnumInSwitchStatementException( __FILE__, __LINE__, __FUNCTION__, "vaccine_mode", vaccine_mode, TyphoidVaccineMode::pairs::lookup_key( vaccine_mode ) );
        }
    }

    REGISTER_SERIALIZABLE(TyphoidVaccine);

    void TyphoidVaccine::serialize(IArchive& ar, TyphoidVaccine* obj)
    {
        BaseIntervention::serialize( ar, obj );
        TyphoidVaccine& vaccine = *obj;
    }
}
