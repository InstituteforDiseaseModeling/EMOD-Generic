/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"
#include "NodeLevelHealthTriggeredIVScaleUpSwitch.h"

#include "InterventionFactory.h"
#include "Log.h"
#include "Debug.h"
#include "NodeEventContext.h"  // for INodeEventContext (ICampaignCostObserver)
#include "ISimulationContext.h"

SETUP_LOGGING( "NodeLevelHealthTriggeredIVScaleUpSwitch" )

namespace Kernel
{
    IMPLEMENT_FACTORY_REGISTERED(NodeLevelHealthTriggeredIVScaleUpSwitch)

    NodeLevelHealthTriggeredIVScaleUpSwitch::NodeLevelHealthTriggeredIVScaleUpSwitch()
        : NodeLevelHealthTriggeredIV()
        , demographic_coverage_time_profile(ScaleUpProfile::Immediate)
        , initial_demographic_coverage(0)
        , primary_time_constant(0)
        , coverage_vs_time_map(0.0f, 999999.0f, 0.0f, 1.0f)
    {
    }

    NodeLevelHealthTriggeredIVScaleUpSwitch::~NodeLevelHealthTriggeredIVScaleUpSwitch() { }
 
    bool
    NodeLevelHealthTriggeredIVScaleUpSwitch::Configure(
        const Configuration * inputJson
    )
    {
        initConfig("Demographic_Coverage_Time_Profile",  demographic_coverage_time_profile, inputJson, MetadataDescriptor::Enum("Demographic_Coverage_Time_Profile", NodeHTI_Demographic_Coverage_Time_Profile_DESC_TEXT, MDD_ENUM_ARGS(ScaleUpProfile) ) );

        initConfigTypeMap("Initial_Demographic_Coverage", &initial_demographic_coverage, NodeHTI_Initial_Demographic_Coverage_DESC_TEXT,    0.0f,    1.0f,  0.0f, "Demographic_Coverage_Time_Profile", "Linear");
        initConfigTypeMap("Primary_Time_Constant",        &primary_time_constant,        NodeHTI_Primary_Time_Constant_DESC_TEXT,        FLT_MIN, FLT_MAX,  1.0f, "Demographic_Coverage_Time_Profile", "Linear");

        initConfigTypeMap("Coverage_vs_Time_Interpolation_Map", &coverage_vs_time_map, WEM_Durability_Map_End_DESC_TEXT, "Demographic_Coverage_Time_Profile", "InterpolationMap");

        //this has to be an array rather than a single intervention json so that it can be empty if you don't want to phase anything out
        initConfigComplexType("Not_Covered_IndividualIntervention_Configs", &not_covered_intervention_configs, NodeHTI_Not_Covered_IndividualIntervention_Configs_DESC_TEXT);

        bool ret= NodeLevelHealthTriggeredIV::Configure(inputJson);
        return ret;
    }

    float NodeLevelHealthTriggeredIVScaleUpSwitch::getDemographicCoverage( ) const
    {
        float demographic_coverage = demographic_restrictions.GetDemographicCoverage();  
        float current_demographic_coverage = demographic_coverage;  
        float dCover_dTime = demographic_coverage - initial_demographic_coverage;

        switch (demographic_coverage_time_profile)
        {
        case ScaleUpProfile::Immediate:
            LOG_DEBUG("ScaleUpProfile is Immediate, don't need to update demographic coverage by time \n");
            current_demographic_coverage = demographic_coverage;
            break;

        case ScaleUpProfile::Linear:
            if (duration <= primary_time_constant) 
            {
                dCover_dTime /= primary_time_constant;
                current_demographic_coverage = initial_demographic_coverage + dCover_dTime*duration;
            }
            else
            {
                current_demographic_coverage = demographic_coverage;
            }
            LOG_DEBUG_F("ScaleUpProfile is Linear, duration is %f, rate of %f, coverage is %f \n", duration, dCover_dTime, current_demographic_coverage); 
            break;

        case ScaleUpProfile::InterpolationMap:
            current_demographic_coverage = coverage_vs_time_map.getValueLinearInterpolation(duration, 0.0f);
            LOG_DEBUG_F("ScaleUpProfile is Interpolation Map, duration is %f, coverage is %f \n", duration, current_demographic_coverage);
            break;

        default:
            throw NotYetImplementedException( __FILE__, __LINE__, __FUNCTION__, "Only Immediate, Linear, and InterpolationMap supported. \n" );
            break;
        }
        return current_demographic_coverage;
    }

    void NodeLevelHealthTriggeredIVScaleUpSwitch::onDisqualifiedByCoverage(IIndividualHumanEventContext *pIndiv)
    {
        //if qualify by everything except demographic coverage, give the not_covered_individualintervention_config 
        // this intervention is the one phased out as the actual_individualintervention_config is phased in
        // if the not_covered_individualintervention_config is NULL, then there is no intervention to phase out
        LOG_DEBUG("The person qualified by everything except demographic coverage, give the not_covered_individualintervention_config \n");

        const json::Array & interventions_array = json::QuickInterpreter(not_covered_intervention_configs._json).As<json::Array>();
        LOG_DEBUG_F("not_covered_intervention_configs array size = %d\n", interventions_array.Size());
       
        if (interventions_array.Size() == 0 )
        {
            LOG_DEBUG("nothing to phase out \n"); 
        }
        else
        {
            for( int idx=0; idx<interventions_array.Size(); idx++ )
            {
                LOG_DEBUG_F("NodeHTIScaleUpSwitch will distribute notcoveredintervention #%d\n", idx);

                const json::Object& notcoveredIntervention = json_cast<const json::Object&>(interventions_array[idx]);

                std::stringstream param_name;
                param_name << "Not_Covered_IndividualIntervention_Configs[" << idx << "]";

                IDistributableIntervention *di = InterventionFactory::getInstance()->CreateIntervention( notcoveredIntervention,
                                                                                                         "campaign",
                                                                                                         param_name.str().c_str() ); 

                if( di )
                {
                    di->Distribute( pIndiv->GetInterventionsContext(), parent->GetCampaignCostObserver() );
                    LOG_DEBUG("A Node level health-triggered intervention was successfully distributed, gave the not_covered_intervention_configs\n");
                }
            }
        }
    }
}
