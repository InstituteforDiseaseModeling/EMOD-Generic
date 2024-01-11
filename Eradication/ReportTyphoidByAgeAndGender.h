/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include <sstream>
#include "SimulationEnums.h"
#include "BaseTextReportEvents.h"
#include "PropertyReport.h" // for some types and a (static) function

#define MAX_AGE (100)

namespace Kernel {
    struct ISimulation;

    class ReportTyphoidByAgeAndGender : public BaseTextReportEvents
    {
        GET_SCHEMA_STATIC_WRAPPER(ReportTyphoidByAgeAndGender)
    public:
        ReportTyphoidByAgeAndGender( const ISimulation* sim = nullptr );

        static IReport* Create(const ISimulation* parent) { return new ReportTyphoidByAgeAndGender(parent); }

        // -----------------------------
        // --- BaseTextReportEvents
        // -----------------------------
        virtual bool Configure( const Configuration* inputJson );
        virtual bool Validate( const ISimulationContext* parent_sim ) override;
        virtual void Initialize( unsigned int nrmSize ) override;

        virtual void UpdateEventRegistration( float currentTime, float dt, 
                                              std::vector<INodeEventContext*>& rNodeEventContextList,
                                              ISimulationEventContext* pSimEventContext );
        virtual std::string GetHeader() const ;
        virtual bool notifyOnEvent(IIndividualHumanEventContext *context, const EventTrigger::Enum& StateChange);

        virtual bool IsCollectingIndividualData( float currentTime, float dt ) const ;
        virtual void LogIndividualData( IIndividualHuman* individual );
        virtual void LogNodeData( INodeContext* pNC );

    private:
        bool doReport;

#define MAX_PROPS 10 // figure this out
        float population[Gender::Enum::COUNT][MAX_AGE][ MAX_PROPS ];         //Gender, Age --> Population
        float infected[Gender::Enum::COUNT][MAX_AGE][ MAX_PROPS ];           //Gender, Age --> Infected
        float newly_infected[Gender::Enum::COUNT][MAX_AGE][ MAX_PROPS ];     //Gender, Age --> Newly Infected 

        float chronic[Gender::Enum::COUNT][MAX_AGE][ MAX_PROPS ];           
        float subClinical[Gender::Enum::COUNT][MAX_AGE][ MAX_PROPS ];           
        float acute[Gender::Enum::COUNT][MAX_AGE][ MAX_PROPS ];           
        float prePatent[Gender::Enum::COUNT][MAX_AGE][ MAX_PROPS ];           

        float chronic_inc[Gender::Enum::COUNT][MAX_AGE][ MAX_PROPS ];           
        float subClinical_inc[Gender::Enum::COUNT][MAX_AGE][ MAX_PROPS ];           
        float acute_inc[Gender::Enum::COUNT][MAX_AGE][ MAX_PROPS ];           
        float prePatent_inc[Gender::Enum::COUNT][MAX_AGE][ MAX_PROPS ];           

        float startYear;
        float stopYear;
        const ISimulation* _parent;

        std::map< std::string, int > bucketToIdMap;
    };

}

