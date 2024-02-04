/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"

#include "HIVTransmissionReporter.h"
#include "Exceptions.h"
#include "IIndividualHumanContext.h"
#include "IIndividualHumanSTI.h"
#include "IIndividualHumanHIV.h"
#include "IndividualEventContext.h"
#include "IRelationship.h"
#include "InfectionHIV.h"
#include "SusceptibilityHIV.h"

SETUP_LOGGING( "HIVTransmissionReporter" )

namespace Kernel
{
    IReport* HIVTransmissionReporter::Create(ISimulation* simulation)
    {
        return new HIVTransmissionReporter();
    }

    HIVTransmissionReporter::HIVTransmissionReporter()
        : StiTransmissionReporter()
        , hiv_report_data()
    {
    }

    HIVTransmissionReporter::~HIVTransmissionReporter()
    {
    }

    std::string HIVTransmissionReporter::GetHeader() const
    {
        std::string header = StiTransmissionReporter::GetHeader();
        header += "," ;
        header += "SRC_CD4," ;
        header += "SRC_VIRAL_LOAD," ;
        header += "SRC_STAGE," ;
        header += "DEST_CD4," ;
        header += "DEST_VIRAL_LOAD," ;
        header += "DEST_STAGE" ;
        return header;
    }

    void HIVTransmissionReporter::ClearData()
    {
        StiTransmissionReporter::ClearData();
        hiv_report_data.clear();
    }

    void HIVTransmissionReporter::CollectOtherData( unsigned int relationshipID, IIndividualHumanSTI* pPartnerSource, IIndividualHumanSTI* pPartnerDest )
    {
        release_assert( pPartnerSource );
        release_assert( pPartnerDest );

        IIndividualHumanHIV* p_hiv_source = pPartnerSource->GetEventContext()->GetIndividual()->GetIndividualContext()->GetIndividualHIV();
        IIndividualHumanHIV* p_hiv_dest   = pPartnerDest->GetEventContext()->GetIndividual()->GetIndividualContext()->GetIndividualHIV();

        release_assert( p_hiv_source );
        release_assert( p_hiv_dest );

        float source_cd4 = p_hiv_source->GetHIVSusceptibility()->GetCD4count();
        float dest_cd4   = p_hiv_dest->GetHIVSusceptibility()->GetCD4count();

        float source_viral_load    = -1.0 ;
        int   source_disease_stage = 0 ;
        if( pPartnerSource->IsInfected() )
        {
            source_viral_load    = p_hiv_source->GetHIVInfection()->GetViralLoad();
            source_disease_stage = p_hiv_source->GetHIVInfection()->GetStage();
        }

        float dest_viral_load    = -1.0 ;
        int   dest_disease_stage = 0 ;
        if( pPartnerDest->IsInfected() )
        {
            dest_viral_load    = p_hiv_dest->GetHIVInfection()->GetViralLoad();
            dest_disease_stage = p_hiv_dest->GetHIVInfection()->GetStage();
        }

        std::stringstream ss ;
        ss                         << ","
           << source_cd4           << ","
           << source_viral_load    << ","
           << source_disease_stage << ","
           << dest_cd4             << ","
           << dest_viral_load      << ","
           << dest_disease_stage   ;

        hiv_report_data[ relationshipID ] = ss.str();
    }

    std::string HIVTransmissionReporter::GetOtherData( unsigned int relationshipID )
    {
        return hiv_report_data[ relationshipID ] ;
    }
}
