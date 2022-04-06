//******************************************************************************
//
// Reporter for serosurveys
//
//******************************************************************************

#pragma once

#include "BaseTextReport.h"

//******************************************************************************

namespace Kernel
{
    class ReportSerosurvey : public BaseTextReport
    {
        public:
            ReportSerosurvey();
            ~ReportSerosurvey();

            bool  Configure(const Configuration * inputJson)       override;
            void  EndTimestep(float currentTime, float dt)         override;

            std::string GetHeader()                          const override;

            void  LogNodeData(INodeContext* node)                  override;

        private:
            float                m_sus_thresh;

            std::string          m_targ_prop;

            std::vector<float>   m_age_bins;
            std::vector<float>   m_time_stamps;
  };
}

//******************************************************************************