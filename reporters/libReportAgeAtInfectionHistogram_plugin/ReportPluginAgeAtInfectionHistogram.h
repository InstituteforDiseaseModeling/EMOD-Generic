/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include "BaseChannelReport.h"

class ReportPluginAgeAtInfectionHistogram : public BaseChannelReport
{
public:
    ReportPluginAgeAtInfectionHistogram();
    virtual ~ReportPluginAgeAtInfectionHistogram() { }

    virtual void BeginTimestep() override;
    virtual void EndTimestep( float currentTime, float dt ) override;
    virtual void LogNodeData( Kernel::INodeContext * pNC ) override;
    virtual bool IsCollectingIndividualData( float currentTime, float dt ) const override { return true ; } ;
    virtual void LogIndividualData( Kernel::IIndividualHuman* individual ) override;
    virtual void Reduce() override;
    virtual void Finalize() override;

protected:
    virtual bool Configure( const Configuration* config) override;
    virtual void populateSummaryDataUnitsMap( std::map<std::string, std::string> &units_map ) override;
    virtual void postProcessAccumulatedData() override;
    int  GetAgeBin(double age);

    float time_since_last_report;
    float reporting_interval_in_years;

    std::vector< float > temp_binned_accumulated_counts;
    std::vector< std::vector< float > > binned_accumulated_counts;

    std::vector< float > age_bin_upper_edges_in_years;

private:
};

