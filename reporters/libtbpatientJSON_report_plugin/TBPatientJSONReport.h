/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include <vector>
#include <map>

#include "IReport.h"

typedef std::map<std::string, bool> status_t;

struct Event
{
    int t;
    std::string type;
    bool previous;
    bool current;
};

struct TBPatient
{
    TBPatient(int id_, float age_);
    ~TBPatient();

    int id;
    float initial_age;

    //remember the previous status
    status_t previous_status;

    //events
    std::vector<Event> events;
    int n_drug_treatments;
    std::vector<std::string> drug_treatments;
};

class TBPatientJSONReport : public Kernel::BaseReport
{
public:
    static IReport* CreateReport();
    TBPatientJSONReport();
    virtual ~TBPatientJSONReport();

    virtual void Initialize( unsigned int nrmSize ) override; // public because Simulation::Populate will call this function, passing in NodeRankMap size

    virtual void BeginTimestep() override;
    virtual void LogNodeData( Kernel::INodeContext * pNC ) override;
    virtual bool IsCollectingIndividualData( float currentTime, float dt ) const override { return true ; };
    virtual void LogIndividualData( Kernel::IIndividualHuman* individual ) override;
    virtual void EndTimestep( float currentTime, float dt ) override;

    // TODO: are we ever going to want to use this on multi-core?  Lot's of data output!
    virtual void Reduce() override;

    virtual std::string GetReportName() const override;
    virtual void Finalize() override;

protected:
    std::string report_name;

    typedef std::map<int, TBPatient*> patient_map_t;
    patient_map_t patient_map; // TODO: lots of patients --> unordered_map (!)
};

