/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include "IReport.h"
#include <list>
#include <vector>
#include "suids.hpp"

namespace Kernel
{
    class KML_Demo_Reporter : public Kernel::BaseReport
    {
    public:
        KML_Demo_Reporter();
        virtual ~KML_Demo_Reporter() { }

        virtual void Initialize( unsigned int nrmSize ) override;
        virtual void Finalize() override;

        virtual void BeginTimestep() override;
        virtual void LogNodeData( INodeContext * pNC ) override;
        virtual bool IsCollectingIndividualData( float currentTime, float dt ) const override { return false ; } ;
        virtual void LogIndividualData( IIndividualHuman* individual ) override;
        virtual void EndTimestep( float currentTime, float dt ) override;

        virtual void Reduce() override;

        virtual std::string GetReportName() const override;

    protected:
        void WriteKmlData();

        typedef std::vector<float> tNode2DataMapVecType;
        typedef std::map< suids::suid_data_t, tNode2DataMapVecType > tNode2DataMapVec;  // Node ID -> Vector of float
        typedef std::map< std::string, tNode2DataMapVec > tChannel2Node2DataMapVec; // Channel string -> map
        tChannel2Node2DataMapVec nodeChannelMapVec;

        typedef std::map< suids::suid_data_t, float > tNode2DataMap;    // Node ID -> Single float
        typedef std::map< std::string, tNode2DataMap > tChannel2Node2DataMap;   // Channel string -> map
        tChannel2Node2DataMap nodeChannelMap;
    };
}
