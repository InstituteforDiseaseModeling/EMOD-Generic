/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"
#include "ConfigParams.h"
#include <iomanip> // setprecision
#include <iostream> // defaultfloat
#include <cmath>
#include "Debug.h"
#include "FileSystem.h"
#include "ReportTyphoidByAgeAndGender.h"
#include "NodeTyphoid.h"
#include "SusceptibilityTyphoid.h"
#include "InfectionTyphoid.h"
#include "TyphoidInterventionsContainer.h"
#include "NodeLevelHealthTriggeredIV.h"
#include "ISimulation.h"
#include "PropertyReport.h"
#include "Simulation.h"

SETUP_LOGGING( "ReportTyphoidByAgeAndGender" )

namespace Kernel 
{
    GET_SCHEMA_STATIC_WRAPPER_IMPL(ReportTyphoidByAgeAndGender,ReportTyphoidByAgeAndGender)

    ReportTyphoidByAgeAndGender::ReportTyphoidByAgeAndGender( const ISimulation* parent )
        : BaseTextReportEvents("ReportTyphoidByAgeAndGender.csv")
        , doReport( false )
        , _parent( parent )
        , startYear(0.0)
        , stopYear(0.0)
    {
        if( !JsonConfigurable::_dryrun )
        { 
            eventTriggerList.push_back( EventTrigger::NewInfection );
        }

        ZERO_ARRAY( population );
        ZERO_ARRAY( infected );
        ZERO_ARRAY( newly_infected );
        ZERO_ARRAY( chronic );
        ZERO_ARRAY( subClinical );
        ZERO_ARRAY( acute );
        ZERO_ARRAY( prePatent );
        ZERO_ARRAY( chronic_inc );
        ZERO_ARRAY( subClinical_inc );
        ZERO_ARRAY( acute_inc );
        ZERO_ARRAY( prePatent_inc );
    }

    bool ReportTyphoidByAgeAndGender::Configure( const Configuration* inputJson )
    {
        initConfigTypeMap( "Report_Typhoid_ByAgeAndGender_Start_Year", &startYear, Report_Typhoid_ByAgeAndGender_Start_Year_DESC_TEXT, 0.0f, FLT_MAX, 0.0f );
        initConfigTypeMap( "Report_Typhoid_ByAgeAndGender_Stop_Year",  &stopYear,  Report_Typhoid_ByAgeAndGender_Stop_Year_DESC_TEXT,  0.0f, FLT_MAX, FLT_MAX );

        bool ret = JsonConfigurable::Configure( inputJson );

        return ret ;
    }

    bool ReportTyphoidByAgeAndGender::Validate( const ISimulationContext* parent_sim )
    {
        if( IPFactory::GetInstance()->GetIPList().size() > 1 )
        {
            throw IncoherentConfigurationException( __FILE__, __LINE__, __FUNCTION__,
            "Report_Typhoid_ByAgeAndGender", "1",
            "Individual Property Keys", "2+" );
        }

        return true;
    }

    void ReportTyphoidByAgeAndGender::Initialize( unsigned int nrmSize )
    {
        BaseTextReportEvents::Initialize( nrmSize );

        // Get integer that represents reportingBucket
        if(IPFactory::GetInstance())
        {
            for( auto reportingBucket : IPFactory::GetInstance()->GetAllPossibleKeyValueCombinations<IPKeyValueContainer>() )
            {
                int id_val = static_cast<int>(bucketToIdMap.size());
                bucketToIdMap.insert( std::make_pair(reportingBucket, id_val) );
            }
        }

        if(bucketToIdMap.empty())
        {
            bucketToIdMap.insert( std::make_pair("", 0) );
        }
    }

    void ReportTyphoidByAgeAndGender::UpdateEventRegistration( float currentTime, float dt, 
                                                               std::vector<INodeEventContext*>& rNodeEventContextList,
                                                               ISimulationEventContext* pSimEventContext )
    {
        auto sim_year = _parent->GetSimulationTime().Year();
        doReport      = false;

        if(sim_year > startYear && sim_year < stopYear)
        {
            // Record events only between start year and stop year
            BaseTextReportEvents::UpdateEventRegistration( currentTime, dt, rNodeEventContextList, pSimEventContext );

            // Generate report on last time step of each year
            if((std::fmod(currentTime, DAYSPERYEAR) > std::fmod(currentTime+dt, DAYSPERYEAR)) || dt > DAYSPERYEAR)
            {
                doReport = true;
            }
        }
    }

    bool ReportTyphoidByAgeAndGender::IsCollectingIndividualData( float currentTime, float dt ) const
    {
        // Only collect individual data between start year and stop year
        auto sim_year = _parent->GetSimulationTime().Year();

        return ((sim_year > startYear) && (sim_year < stopYear));
    }

    std::string ReportTyphoidByAgeAndGender::GetHeader() const
    {
        std::stringstream header;
        header << "Time Of Report (Year), "
               << "NodeId, "
               << "Gender, "
               << "Age, "
               << "HINT Group, "
               << "Population, "
               << "Infected, "
               << "Newly Infected, "
               << "Chronic (Prev), "
               << "Sub-Clinical (Prev), "
               << "Acute (Prev), "
               << "Pre-Patent (Prev), "
               << "Chronic (Inc) , "
               << "Sub-Clinical (Inc), "
               << "Acute (Inc), "
               << "Pre-Patent (Inc), ";

        return header.str();
    }

    void ReportTyphoidByAgeAndGender::LogNodeData( INodeContext* pNC )
    {
        if(!doReport)
        {
            return;
        }

        LOG_INFO_F( "Writing accumulated data to disk for this reporting period.\n" );
        for( int gender = 0; gender < Gender::Enum::COUNT; gender++ ) 
        {
            for( int age_bin = 0; age_bin < MAX_AGE; age_bin++ ) 
            {
                // iterate over reporting buckets
                for( auto ip_entry : bucketToIdMap )
                {
                    unsigned int ip_idx = ip_entry.second;
                    // Following is for cross-platform correctness
                    GetOutputStream() 
                        << std::fixed << std::setprecision(3) 
                        << _parent->GetSimulationTime().Year();
                    GetOutputStream().unsetf(std::ios_base::floatfield);
                    GetOutputStream()
                        << "," << pNC->GetExternalID()
                        << "," << gender
                        << "," << age_bin
                        << "," << ip_entry.first
                        << "," << population[gender][age_bin][ip_idx]
                        << "," << infected[gender][age_bin][ip_idx]
                        << "," << newly_infected[gender][age_bin][ip_idx]

                        << "," << chronic[gender][age_bin][ip_idx]
                        << "," << subClinical[gender][age_bin][ip_idx]
                        << "," << acute[gender][age_bin][ip_idx]
                        << "," << prePatent[gender][age_bin][ip_idx]

                        << "," << chronic_inc[gender][age_bin][ip_idx]
                        << "," << subClinical_inc[gender][age_bin][ip_idx]
                        << "," << acute_inc[gender][age_bin][ip_idx]
                        << "," << prePatent_inc[gender][age_bin][ip_idx]
                        << endl;
                }
            }
        }

        ZERO_ARRAY( population );
        ZERO_ARRAY( infected );
        ZERO_ARRAY( newly_infected );
        ZERO_ARRAY( chronic );
        ZERO_ARRAY( subClinical );
        ZERO_ARRAY( acute );
        ZERO_ARRAY( prePatent );
        ZERO_ARRAY( chronic_inc );
        ZERO_ARRAY( subClinical_inc );
        ZERO_ARRAY( acute_inc );
        ZERO_ARRAY( prePatent_inc );
    }

    void ReportTyphoidByAgeAndGender::LogIndividualData( IIndividualHuman* p_iih )
    {
        IIndividualHumanTyphoid* p_iiht = p_iih->GetIndividualContext()->GetIndividualTyphoid();

        float mc_weight =  p_iih->GetMonteCarloWeight();
        float age_yr     = p_iih->GetAge()/static_cast<float>(DAYSPERYEAR);
        int   age_bin    = static_cast<int>(floor( (std::min)(static_cast<float>(MAX_AGE)-1.0f, age_yr) ));
        auto  gen_int    = p_iih->GetGender();
        int   gender     = (gen_int == Gender::MALE) ? 0 : 1;
        auto  rbi        = bucketToIdMap.at( p_iih->GetPropertyReportString() );
        bool  is_infect  = p_iih->IsInfected();
        auto  suid_dat   = p_iih->GetSuid().data;

        bool  is_chronic = p_iiht->IsChronicCarrier();
        bool  is_subclin = p_iiht->IsSubClinical();
        bool  is_acute   = p_iiht->IsAcute(); 
        bool  is_pre_pat = p_iiht->IsPrePatent(); 

        // We do the incidences throughout the reporting period
        if(is_chronic)
        {
            chronic_inc[ gender ][ age_bin ][ rbi ] += mc_weight;
        }
        if(is_subclin)
        {
            subClinical_inc[ gender ][ age_bin ][ rbi ] += mc_weight;
        }
        if(is_acute)
        {
            acute_inc[ gender ][ age_bin ][ rbi ] += mc_weight;
        }

        LOG_VALID_F( "[Inc] Individual %d (age=%f,sex=%d), infected = %d, isPrePatent = %d, isChronic = %d, isSub = %d, isAcute = %d\n",
                     suid_dat, age_yr, gen_int, is_infect, is_pre_pat, is_chronic, is_subclin, is_acute);

        // We do the prevalences only in the snapshot timestep in which we are writing out the report.
        if( doReport )
        {
            LOG_VALID_F( "doReport = true.\n" );
            bool is_chronic_f = p_iiht->IsChronicCarrier( false );
            bool is_subclin_f = p_iiht->IsSubClinical( false );
            bool is_acute_f   = p_iiht->IsAcute( false );
            bool is_pre_pat_f = p_iiht->IsPrePatent( false );

            population[ gender ][ age_bin ][ rbi ] += mc_weight;

            if(is_infect)
            {
                infected[ gender ][ age_bin ][ rbi ] += mc_weight;
            }

            if(is_chronic_f)
            {
                chronic[ gender ][ age_bin ][ rbi ] += mc_weight;
            }
            if(is_subclin_f)
            {
                subClinical[ gender ][ age_bin ][ rbi ] += mc_weight;
            }
            if(is_acute_f)
            {
                acute[ gender ][ age_bin ][ rbi ] += mc_weight;
            }
            if(is_pre_pat_f)
            {
                prePatent[ gender ][ age_bin ][ rbi ] += mc_weight;
                LOG_VALID_F( "prePatent[ %d ][ %d ][ %d ] = %f\n", gender, age_bin, prePatent[ gender ][ age_bin ][ rbi ] );
            }
            LOG_VALID_F( "[Prev] Individual %d (age=%f,sex=%d), infected = %d, isPrePatent = %d, isChronic = %d, isSub = %d, isAcute = %d\n",
                         suid_dat, age_yr, gen_int, is_infect, is_pre_pat_f, is_chronic_f, is_subclin_f, is_acute_f);
        }
    }

    bool ReportTyphoidByAgeAndGender::notifyOnEvent( IIndividualHumanEventContext* context, const EventTrigger::Enum& StateChange)
    {
        LOG_DEBUG_F( "Individual %d experienced event %s\n", context->GetSuid().data, EventTrigger::pairs::lookup_key( StateChange ).c_str() );

        float       mc_weight = context->GetMonteCarloWeight();
        int         gender    = context->GetGender() == Gender::MALE ? 0 : 1;
        int         age_bin   = static_cast<int>(floor( (std::min)(static_cast<float>(MAX_AGE)-1.0f, context->GetAge()/static_cast<float>(DAYSPERYEAR)) ));
        auto        rbi       = bucketToIdMap.at( context->GetIndividual()->GetPropertyReportString() );

        if( StateChange == EventTrigger::NewInfection )
        {
            LOG_DEBUG_F( "NewInfection for individual %lu with age(bin) %d and gender %d.\n", context->GetSuid().data, age_bin, gender );
            newly_infected[ gender ][ age_bin ][ rbi ] += mc_weight;
            prePatent_inc[ gender ][ age_bin ][ rbi ] += mc_weight;
        }

        return true;
    }
}

