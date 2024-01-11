/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"

#include "IdmString.h"
#include "ConfigParams.h"
#include "IIndividualHuman.h"
#include "Node.h"
#include "PropertyReport.h"


using namespace std;
using namespace json;

SETUP_LOGGING( "PropertyReport" )


namespace Kernel {


/////////////////////////
// Initialization methods
/////////////////////////
IReport* PropertyReport::CreateReport()
{
    return new PropertyReport( "PropertyReport.json" );
}

PropertyReport::PropertyReport( const std::string& rReportName )
    : Report( rReportName )
    , permutationsSet()
    , disease_deaths()
    , infected()
    , new_contact_infections()
    , new_enviro_infections()
    , new_infections()
    , new_reported_infections()
    , statPop()
{
}

void PropertyReport::Initialize( unsigned int nrmSize )
{
    BaseChannelReport::Initialize( nrmSize );

    if(IPFactory::GetInstance())
    {
        permutationsSet = IPFactory::GetInstance()->GetAllPossibleKeyValueCombinations<IPKeyValueContainer>();
    }

    return;
}

void PropertyReport::LogIndividualData( Kernel::IIndividualHuman* individual )
{
    std::string reportingBucket = individual->GetPropertyReportString();

    auto mcw = individual->GetMonteCarloWeight();
    auto nis = individual->GetNewInfectionState();

    if(nis == NewInfectionState::NewAndDetected || nis == NewInfectionState::NewInfection)
    {
        new_infections[ reportingBucket ] += mcw;

        // Check is necessarry; new infection may have already cleared
        if(individual->GetInfections().size() > 0)
        {
            auto inf = individual->GetInfections().back();
            if( inf->GetSourceRoute() == TransmissionRoute::ENVIRONMENTAL )
            {
                new_enviro_infections[ reportingBucket ] += mcw;
            }
            else if( inf->GetSourceRoute() == TransmissionRoute::CONTACT )
            {
                new_contact_infections[ reportingBucket ] += mcw;
            }
        }
    }

    if(nis == NewInfectionState::NewAndDetected || nis == NewInfectionState::NewlyDetected)
    {
        new_reported_infections[ reportingBucket ] += mcw;
    }

    if(individual->GetStateChange() == HumanStateChange::KilledByInfection)
    {
        disease_deaths[ reportingBucket ] += mcw;
    }

    if (individual->IsInfected())
    {
        infected[ reportingBucket ] += mcw;
    }

    statPop[ reportingBucket ] += mcw;
}

void PropertyReport::LogNodeData( Kernel::INodeContext * pNC) 
{
    LOG_DEBUG( "LogNodeData\n" );
    for (auto& reportingBucket : permutationsSet)
    {
        Accumulate("New Infections:" + reportingBucket, new_infections[reportingBucket]);
        new_infections[reportingBucket] = 0.0f;

        if(pNC->GetParams()->enable_environmental_route)
        {
            Accumulate( "New Infections By Route (ENVIRONMENT):" + reportingBucket, new_enviro_infections[ reportingBucket ] );
            new_enviro_infections[ reportingBucket ] = 0;

            Accumulate( "New Infections By Route (CONTACT):" + reportingBucket, new_contact_infections[ reportingBucket ] );
            new_contact_infections[ reportingBucket ] = 0;
        }

        Accumulate("Disease Deaths:" + reportingBucket, disease_deaths[reportingBucket]);

        Accumulate("Statistical Population:" + reportingBucket, statPop[ reportingBucket ] );
        statPop[reportingBucket] = 0.0f;

        Accumulate("Infected:" + reportingBucket, infected[ reportingBucket ]);
        infected[ reportingBucket ] = 0.0f;
    }

    if ( IPFactory::GetInstance() && IPFactory::GetInstance()->HasIPs() && pNC->GetParams()->enable_environmental_route )
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
        }
    }
}

// normalize by time step and create derived channels
void PropertyReport::postProcessAccumulatedData()
{
    LOG_DEBUG( "postProcessAccumulatedData in PropertyReport\n" );
}

// This is just to avoid Disease Deaths not broken down by props which is in base class. Why is it there?
void PropertyReport::EndTimestep( float currentTime, float dt )
{
    LOG_DEBUG( "EndTimestep\n" );
}

void PropertyReport::reportContagionForRoute( TransmissionRoute::Enum tx_route, IndividualProperty* property, INodeContext* pNC )
{
    std::string prefix;
    switch(tx_route)
    {
        case TransmissionRoute::CONTACT:
            prefix = "Contagion (Contact):";
            break;

        case TransmissionRoute::ENVIRONMENTAL:
            prefix = "Contagion (Environment):";
            break;

        default:
            throw BadEnumInSwitchStatementException(__FILE__, __LINE__, __FUNCTION__, "route", uint32_t(tx_route), TransmissionRoute::pairs::lookup_key(tx_route));
    }

    for (auto& value : property->GetValues<IPKeyValueContainer>())
    {
        const string& label = value.ToString();
        auto contagion = pNC->GetContagionByRouteAndProperty( tx_route, value );
        Accumulate( (prefix + label).c_str(), contagion );
    }
}

};