/***************************************************************************************************

Copyright (c) 2016 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"

#include "IIndividualHuman.h"
#include "Node.h"
#include "PropertyReportEnvironmental.h"
#include "TransmissionGroupsBase.h"
#include "SimulationEnvironmental.h"
#include "SimulationEventContext.h"
#include "StrainIdentity.h"
#include "NodeEnvironmental.h"

using namespace std;
using namespace json;

SETUP_LOGGING( "PropertyReportEnvironmental" )

static const std::string _report_name = "PropertyReportEnvironmental.json";

// nasty copy-paste from ReportEnvironmental.
static const std::string _num_enviro_infections_label    = "New Infections By Route (ENVIRONMENT)";
static const std::string _num_contact_infections_label   = "New Infections By Route (CONTACT)";

namespace Kernel
{

GET_SCHEMA_STATIC_WRAPPER_IMPL( PropertyReportEnvironmental, PropertyReportEnvironmental )

        /////////////////////////
// Initialization methods
/////////////////////////
IReport*
PropertyReportEnvironmental::CreateReport()
{
    return _new_ PropertyReportEnvironmental();
}

PropertyReportEnvironmental::PropertyReportEnvironmental()
    : PropertyReport( _report_name )
{
}

PropertyReportEnvironmental::PropertyReportEnvironmental( const std::string& _report_name )
    : PropertyReport( _report_name )
{
}

void
PropertyReportEnvironmental::LogIndividualData(
    Kernel::IIndividualHuman* individual
)
{
    PropertyReport::LogIndividualData( individual );

    // Try an optimized solution that constructs a reporting bucket string based entirely
    // on the properties of the individual. But we need some rules. Let's start with simple
    // alphabetical ordering of category names
    std::string reportingBucket = individual->GetPropertyReportString();

    float mc_weight = (float)individual->GetMonteCarloWeight();
    const Kernel::IndividualHumanEnvironmental* individual_ty = static_cast<const Kernel::IndividualHumanEnvironmental*>(individual);

    if( individual->IsInfected() )
    {
        NewInfectionState::_enum nis = individual->GetNewInfectionState(); 
        if( nis == NewInfectionState::NewAndDetected ||
            nis == NewInfectionState::NewInfection )
        {
            auto inf = individual->GetInfections().back();
            if( inf->GetSourceRoute() == TransmissionRoute::ENVIRONMENTAL )
            {
                new_enviro_infections[ reportingBucket ] += mc_weight;
            }
            else if( inf->GetSourceRoute() == TransmissionRoute::CONTACT )
            {
                new_contact_infections[ reportingBucket ] += mc_weight;
            }
        }
    }
}

void
PropertyReportEnvironmental::LogNodeData( INodeContext* pNC )
{
    PropertyReport::LogNodeData(pNC);

    LOG_DEBUG( "LogNodeData in PropertyReportEnvironmental\n" );
    for (const auto& entry : permutationsSet)
    {
        std::string reportingBucket = PropertiesToString( entry );

        Accumulate( _num_enviro_infections_label + ":" + reportingBucket, new_enviro_infections[ reportingBucket ] );
        Accumulate( _num_contact_infections_label + ":" + reportingBucket, new_contact_infections[ reportingBucket ] );
        new_enviro_infections[ reportingBucket ] = 0;
        new_contact_infections[ reportingBucket ] = 0;
    }

    {
        // Grovel through the demographics to see which properties are being used for HINT.

        if ( IPFactory::GetInstance() && IPFactory::GetInstance()->HasIPs() )
        {
            for ( auto property : IPFactory::GetInstance()->GetIPList() )
            {
                auto nodeId = pNC->GetExternalID();
                auto hint = property->GetIntraNodeTransmission( nodeId );
                auto matrix = hint.GetMatrix();

                if ( matrix.size() > 0 )
                {
                    reportContagionForRoute( hint.GetRouteName(), property, pNC );
                }
                else if ( hint.GetRouteToMatrixMap().size() > 0 )
                {
                    for (auto entry : hint.GetRouteToMatrixMap())
                    {
                        reportContagionForRoute( entry.first, property, pNC );
                    }
                }
                else //HINT is enabled, but no transmission matrix is detected
                {
                    LOG_INFO_F( "IndividualProperty '%s' isn't part of HINT configuration.\n", property->GetKeyAsString().c_str() );
                }
            }
        }
        else
        {
            LOG_INFO( "PropertyReportEnvironmental didn't find any IndividualProperties.\n" );
        }
    }
}

void PropertyReportEnvironmental::reportContagionForRoute( TransmissionRoute::Enum route, IndividualProperty* property, INodeContext* pNC )
{
    if ( (route != TransmissionRoute::CONTACT) && (route != TransmissionRoute::ENVIRONMENTAL) )
    {
        LOG_WARN_F( "Unknown route '%s' in IndividualProperties for node %d.\n", TransmissionRoute::pairs::lookup_key(route), pNC->GetExternalID() );
        return;
    }

    std::string prefix = (route == TransmissionRoute::CONTACT) ? "Contagion (Contact):" : "Contagion (Environment):";
    for (auto& value : property->GetValues<IPKeyValueContainer>())
    {
        const string& label = value.ToString();
        auto contagion = pNC->GetContagionByRouteAndProperty( route, value );
        Accumulate( (prefix + label).c_str(), contagion );
    }
}

}
