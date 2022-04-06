//******************************************************************************
//
// Reporter for strain tracking
//
//******************************************************************************

#pragma once

#include "BaseTextReport.h"

//******************************************************************************

namespace Kernel
{
    class ReportStrainTracking : public BaseTextReport
    {
        public:
            ReportStrainTracking();
            ~ReportStrainTracking();

            bool  Configure(const Configuration * inputJson)       override;
            void  EndTimestep(float currentTime, float dt)         override;

            std::string GetHeader()                          const override;

            void  LogNodeData(INodeContext* node)                  override;

        private:
            bool                 m_all_done;
            float                m_time_start;
            float                m_time_end;
  };
}

//******************************************************************************