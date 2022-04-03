//******************************************************************************
//
// Reporter for generating line-list style outputs
//
//******************************************************************************

#pragma once

#include "BaseTextReport.h"

//******************************************************************************

namespace Kernel
{
  class ReportLineList : public BaseTextReport
  {
    public:
       ReportLineList();
      ~ReportLineList();

      bool  Configure(const Configuration * inputJson)               override;
      void  EndTimestep(float currentTime, float dt)                 override;

      std::string GetHeader()                                  const override;

      void  LogNodeData(INodeContext* node)                          override;
      bool  IsCollectingIndividualData(float cTime, float dt)  const override;
      void  LogIndividualData(IIndividualHuman* individual)          override;

    private:
      bool                 m_all_done;
      float                m_time_start;
      float                m_time_end;

      std::string          m_rep_type;

      float                m_ni_rate;
  };
}

//******************************************************************************