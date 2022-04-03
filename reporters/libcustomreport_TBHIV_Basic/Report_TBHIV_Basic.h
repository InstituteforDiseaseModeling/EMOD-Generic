/***************************************************************************************************

Copyright (c) 2016 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include "BaseTextReportEvents.h"

namespace Kernel
{
    ENUM_DEFINE(CD4_Stage,
		ENUM_VALUE_SPEC(NA             , 0)  
        ENUM_VALUE_SPEC(HIV_NEGATIVE   , 1) 
        ENUM_VALUE_SPEC(CD4_UNDER_200  , 2) 
        ENUM_VALUE_SPEC(CD4_200_TO_350 , 3) 
        ENUM_VALUE_SPEC(CD4_350_TO_500 , 4)
        ENUM_VALUE_SPEC(CD4_ABOVE_500  , 5)
        ENUM_VALUE_SPEC(COUNT          , 6) )   // Needed for array initialization below

	ENUM_DEFINE(ARTStatusLocal,
	    ENUM_VALUE_SPEC(NA      ,    0)
		ENUM_VALUE_SPEC(OFFART  ,    1)
		ENUM_VALUE_SPEC(ONART   ,    2)
		ENUM_VALUE_SPEC(COUNT   ,    3))

     ENUM_DEFINE(TB_State, 
	     ENUM_VALUE_SPEC(NA                     , 0)
	     ENUM_VALUE_SPEC(Negative               , 1)
	     ENUM_VALUE_SPEC(Latent                 , 2)
	     ENUM_VALUE_SPEC(ActivePreSymptomatic   , 3)
	     ENUM_VALUE_SPEC(ActiveSmearPos         , 4)
	     ENUM_VALUE_SPEC(ActiveSmearNeg         , 5)
	     ENUM_VALUE_SPEC(ActiveExtraPulm        , 6)
         ENUM_VALUE_SPEC(COUNT                  , 7))

     ENUM_DEFINE(MDR_State, 
	     ENUM_VALUE_SPEC(NA                     , 0)
	     ENUM_VALUE_SPEC(Negative               , 1)
	     ENUM_VALUE_SPEC(MDR                    , 2)
	     ENUM_VALUE_SPEC(COUNT                  , 3) )

    class Report_TBHIV_Basic : public BaseTextReportEvents
    {
    public:
        Report_TBHIV_Basic();
        virtual ~Report_TBHIV_Basic();

        // BaseEventReport
        virtual bool Configure( const Configuration* ) override;
        virtual void Initialize( unsigned int nrmSize ) override;

        virtual void UpdateEventRegistration( float currentTime, 
                                              float dt, 
                                              std::vector<INodeEventContext*>& rNodeEventContextList,
                                              ISimulationEventContext* pSimEventContext ) override;

        virtual std::string GetHeader() const override;
        virtual bool IsCollectingIndividualData( float currentTime, float dt ) const override;
        virtual void LogIndividualData( Kernel::IIndividualHuman* individual ) override;
        virtual void LogNodeData( Kernel::INodeContext * pNC ) override;
        //virtual void Reduce();
        //virtual void Finalize();
        virtual bool notifyOnEvent( IIndividualHumanEventContext *context, 
                                    const EventTrigger::Enum& trigger ) override;
    private:

        const float report_tbhiv_half_period;
        float next_report_time;
        bool doReport;
        float startYear;
        float stopYear;
        bool is_collecting_data;

        CD4_Stage::Enum ComputeCD4Stage(IIndividualHumanEventContext *context);
		TB_State::Enum ComputeTBState(IIndividualHumanEventContext *context);
		MDR_State::Enum ComputeMDRState(IIndividualHumanEventContext* context);
		ARTStatusLocal::Enum ComputeARTStatus(IIndividualHumanEventContext* context);

        //Care_Stage::Enum ComputeCareStage(IIndividualHumanEventContext *context);

        float Population[TB_State::Enum::COUNT][CD4_Stage::Enum::COUNT][MDR_State::COUNT][ARTStatusLocal::COUNT];      // CD4_Stage, Care_Stage --> Population
		float DiseaseDeaths[TB_State::Enum::COUNT][CD4_Stage::Enum::COUNT][MDR_State::COUNT][ARTStatusLocal::COUNT];   
        float NonDiseaseDeaths;
		float ARTDropouts[CD4_Stage::Enum::COUNT];
        float ART_Initiations[CD4_Stage::Enum::COUNT];                          // CD4_Stage             --> ART initiations
        float New_Activations[CD4_Stage::Enum::COUNT][MDR_State::COUNT][ARTStatusLocal::COUNT];                                                  //                       --> Infections
        float New_TBDiagnoses[CD4_Stage::Enum::COUNT][MDR_State::COUNT][ARTStatusLocal::COUNT];  
		//Treatment Initiations
		//Treatment Prevalence
	                                                                           
    };
}
