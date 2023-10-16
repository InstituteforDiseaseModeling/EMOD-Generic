/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"

#ifdef ENABLE_TYPHOID

#include <numeric> // for std::accumulate
#include "ReportTyphoid.h" // for base class
#include "NodeTyphoid.h" // for base class
#include "Debug.h" // for base class
#include "IdmDateTime.h"

SETUP_LOGGING( "ReportTyphoid" )


namespace Kernel
{
GET_SCHEMA_STATIC_WRAPPER_IMPL(ReportTyphoid,ReportTyphoid)

ReportTyphoid::ReportTyphoid()
{
}

void ReportTyphoid::BeginTimestep()
{
    chron_carriers_counter         = 0.0f;
    subclinical_infections_counter = 0.0f;
    acute_infections_counter       = 0.0f;

    return Report::BeginTimestep();
}

void ReportTyphoid::LogIndividualData( IIndividualHuman * individual )
{
    Report::LogIndividualData( individual );

    IIndividualHumanTyphoid* typhoid_individual = nullptr;
    if( individual->QueryInterface( GET_IID( IIndividualHumanTyphoid ), (void**)&typhoid_individual ) != s_OK )
    {
        throw QueryInterfaceException( __FILE__, __LINE__, __FUNCTION__, "individual", "IIndividualTyphoid", "IndividualHuman" );
    }

    float mcw = individual->GetMonteCarloWeight();
    if( typhoid_individual->IsChronicCarrier( false ) )
    {
        chron_carriers_counter += mcw;
    }

    if( individual->IsInfected() )
    {
        if( typhoid_individual->IsSubClinical() )
        {
            subclinical_infections_counter += mcw;
        }
        else if( typhoid_individual->IsAcute() )
        {
            acute_infections_counter += mcw;
        }
    }
}

void ReportTyphoid::LogNodeData( INodeContext * pNC )
{
    Report::LogNodeData( pNC );

    Accumulate( "Number of Chronic Carriers",            chron_carriers_counter  );
    Accumulate( "Number of New Sub-Clinical Infections", subclinical_infections_counter );
    Accumulate( "Number of New Acute Infections",        acute_infections_counter );
}

}

#endif // ENABLE_TYPHOID