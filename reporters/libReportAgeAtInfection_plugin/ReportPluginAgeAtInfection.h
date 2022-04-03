/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include "BaseChannelReport.h"

class ReportPluginAgeAtInfection : public BaseChannelReport
{
public:

    ReportPluginAgeAtInfection();
    virtual ~ReportPluginAgeAtInfection() { }

    virtual void EndTimestep( float currentTime, float dt ) override;
    virtual void LogNodeData( Kernel::INodeContext * pNC ) override;
    virtual bool IsCollectingIndividualData( float currentTime, float dt ) const override { return true ; } ;
    virtual void LogIndividualData( Kernel::IIndividualHuman* individual ) override;
    virtual void Reduce() override;
    virtual void Finalize() override;

protected:
    virtual void populateSummaryDataUnitsMap( std::map<std::string, std::string> &units_map ) override;
    virtual void postProcessAccumulatedData() override;
    float timestep;
    std::vector<float> ages;
    float sampling_ratio;

    std::map< unsigned int, std::vector<float> > time_age_map;

private:
};

