/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once
#include "BaseTextReport.h"
#include "IIndividualHuman.h"

namespace Kernel
{
    class INodeSTI;
    struct ISociety;

    class StiRelationshipQueueReporter : public BaseTextReport
    {
    public:
        StiRelationshipQueueReporter();
        virtual ~StiRelationshipQueueReporter();

        // IReport
        virtual void UpdateEventRegistration( float currentTime, 
                                              float dt, 
                                              std::vector<INodeEventContext*>& rNodeEventContextList,
                                              ISimulationEventContext* pSimEventContext ) override;
        virtual void LogNodeData( INodeContext* pNC ) override;

    protected:
        // BaseTextReport
        virtual std::string GetHeader() const override;

    private:
        std::string CreateLengthsString( const map<int, vector<int>>& rQueueLengthMap );
        std::string CreateBinString( const std::vector<int>& rQueue, int* pTotalCount );

        bool m_FirstTime ;
    };
}