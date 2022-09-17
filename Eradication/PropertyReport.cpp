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
    , permutationsList()
    , disease_deaths()
    , infected()
    , new_contact_infections()
    , new_enviro_infections()
    , new_infections()
    , new_reported_infections()
    , statPop()
{
}

// OK, so here's what we've got to do. Node has an unknown collection of key-value pairs.
// E.g., QoC:Terrible->Great, Accessibility:Urban->Rural, LifeStage:Home->Retired. We need:
// QoC:Terrible,Access:Urban,LifeStage:Home,
// QoC:Terrible,Access:Urban,LifeStage:School,
// QoC:Terrible,Access:Urban,LifeStage:Work,
// etc.
// This is a set of maps?
void PropertyReport::GenerateAllPermutationsOnce( std::set< std::string > keys, tKeyValuePair perm, tPermutations &permsSet )
{
    if( keys.size() )
    {
        const std::string key = *keys.begin();
        keys.erase( key );
        const IndividualProperty* p_ip = IPFactory::GetInstance()->GetIP( key );
        for( auto kv : p_ip->GetValues<IPKeyValueContainer>() )
        {
            std::string value = kv.GetValueAsString();
            auto kvp = perm;
            kvp.insert( make_pair( key, value ) );
            GenerateAllPermutationsOnce( keys, kvp, permsSet );
        }
    }
    else
    {
        permsSet.insert( perm );
    }
}

/////////////////////////
// steady-state methods
/////////////////////////

template<typename T> static std::set< std::string > getKeys( const T &propMap )
{
    // put all keys in set
    std::set< std::string > allKeys;
    for (auto& entry : propMap)
    {
        allKeys.insert( entry.first );
    }
    return allKeys;
}

void PropertyReport::LogIndividualData( Kernel::IIndividualHuman* individual )
{
    std::string reportingBucket = individual->GetPropertyReportString();
    if( reportingBucket.empty() )
    {
        auto permKeys = IPFactory::GetInstance()->GetKeysAsStringSet();
        if( permutationsSet.size() == 0 )
        {
            // put all keys in set
            tKeyValuePair actualPerm;
            GenerateAllPermutationsOnce( permKeys, actualPerm, permutationsSet ); // call this just first time.
        }

        tProperties prop_map = individual->GetProperties()->GetOldVersion();
        if( prop_map.size() == 0 )
        {
            LOG_WARN_F( "Individual %lu aged %f (years) had no properties in %s.\n", individual->GetSuid().data, individual->GetAge()/DAYSPERYEAR, __FUNCTION__ );
            // This seems to be people reaching "old age" (i.e., over 125)
            return;
        }

        // Copy all property keys from src to dest but only if present in permKeys
        auto src = getKeys( prop_map );

        // copy-if setup
        std::vector<std::string> dest( src.size() );
        auto it = std::copy_if (src.begin(), src.end(), dest.begin(), [&](std::string test)
            {
                return ( std::find( permKeys.begin(), permKeys.end(), test ) != permKeys.end() );
            }
        );
        dest.resize(std::distance(dest.begin(),it));

        // new map from those keys that make it through filter
        tProperties permProps;
        for( auto &entry : dest )
        {
            permProps.insert( std::make_pair( entry, prop_map.at(entry) ) );
        }
        // Try an optimized solution that constructs a reporting bucket string based entirely
        // on the properties of the individual. But we need some rules. Let's start with simple
        // alphabetical ordering of category names
        reportingBucket = PropertiesToString( permProps );
        individual->SetPropertyReportString( reportingBucket );
    }

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
    for (auto& kvp : permutationsSet)
    {
        std::string reportingBucket = PropertiesToString( kvp );

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