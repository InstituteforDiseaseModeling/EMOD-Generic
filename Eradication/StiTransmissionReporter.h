/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once
#include "BaseTextReportEvents.h"
#include "ISimulation.h"
#include "IIndividualHuman.h"
#include "ExternalNodeId.h"
#include "Properties.h"

namespace Kernel
{
    struct StiTransmissionInfo
    {
        NonNegativeFloat time;
        NonNegativeFloat year;

        unsigned long rel_id; // relationship id
        ExternalNodeId_t node_id;
        unsigned long source_id;
        bool source_is_infected;
        unsigned int source_gender;
        float source_age;
        unsigned int source_current_relationship_count;
        unsigned int source_lifetime_relationship_count;
        unsigned int source_relationships_in_last_6_months;
        unsigned int source_extrarelational_flags;
        unsigned int source_is_circumcised;
        bool source_has_sti;
        bool source_is_superspreader;
        float source_infection_age;
        IPKeyValueContainer source_ip;
        unsigned long destination_id;
        bool destination_is_infected;
        unsigned int destination_gender;
        float destination_age;
        unsigned int destination_current_relationship_count;
        unsigned int destination_lifetime_relationship_count;
        unsigned int destination_relationships_in_last_6_months;
        unsigned int destination_extrarelational_flags;
        unsigned int destination_is_circumcised;
        bool destination_has_sti;
        bool destination_is_superspreader;
        IPKeyValueContainer destination_ip;
    };

    struct IIndividualHumanSTI;

    class StiTransmissionReporter : public BaseTextReportEvents
    {
    public:
        static IReport* Create(ISimulation* simulation);

        // IReport
        virtual bool Configure( const Configuration* inputJson ) override;
        virtual void Initialize( unsigned int nrmSize ) override;
        virtual void BeginTimestep() override;
        virtual void EndTimestep( float currentTime, float dt ) override;

    protected:
        StiTransmissionReporter();
        virtual ~StiTransmissionReporter();

        // BaseTextReport
        virtual std::string GetHeader() const override;

        virtual bool notifyOnEvent( IIndividualHumanEventContext *context, const EventTrigger::Enum& trigger ) override;

        // Methods for subclasses to override so that they can add data
        virtual void ClearData();
        virtual void CollectOtherData( unsigned int relationshipID,
                                       IIndividualHumanSTI* pPartnerA,
                                       IIndividualHumanSTI* pPartnerB ) {} ;
        virtual std::string GetOtherData( unsigned int relationshipID ) { return ""; };

        // member variables
        std::vector<StiTransmissionInfo> report_data;
        jsonConfigurable::tDynamicStringSet properties_to_report;
        std::vector<IPKey> keys_to_report;
    };
}