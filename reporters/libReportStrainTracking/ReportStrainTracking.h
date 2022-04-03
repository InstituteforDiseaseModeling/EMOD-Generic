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

      bool  Configure(const Configuration * inputJson)               override;
      void  EndTimestep(float currentTime, float dt)                 override;
      void  Finalize()                                               override;
      
      std::string GetHeader()                                  const override;

      void  LogNodeData(INodeContext* node)                          override;

    private:
      float                m_num_rows;
      int                  m_num_rows_tot;

      bool                 m_all_done;
      float                m_time_start;
      float                m_time_end;

      std::vector<std::string>     m_timestep_vec;
      std::vector<std::string>     m_nodedict_vec;
  };
}

//******************************************************************************