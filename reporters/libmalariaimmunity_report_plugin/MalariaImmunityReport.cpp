/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"
#include "MalariaImmunityReport.h"

#include <algorithm>

#include "DllInterfaceHelper.h"
#include "FileSystem.h"
#include "Environment.h"
#include "Exceptions.h"
#include "IndividualMalaria.h"
#include "SusceptibilityMalaria.h"
#include "ReportUtilities.h"

//******************************************************************************

//******************************************************************************

SETUP_LOGGING( "MalariaImmunityReport" )

static const char* _sim_types[] = {"MALARIA_SIM", nullptr};

Kernel::DllInterfaceHelper DLL_HELPER( _module, _sim_types );

//******************************************************************************
// DLL Methods
//******************************************************************************

#ifdef __cplusplus
extern "C" {
#endif

DTK_DLLEXPORT char*
__cdecl GetEModuleVersion(char* sVer, const Environment* pEnv)
{
    return DLL_HELPER.GetEModuleVersion( sVer, pEnv );
}

DTK_DLLEXPORT void
__cdecl GetSupportedSimTypes(char* simTypes[])
{
    DLL_HELPER.GetSupportedSimTypes( simTypes );
}

DTK_DLLEXPORT const char*
__cdecl GetType()
{
    return DLL_HELPER.GetType();
}

DTK_DLLEXPORT Kernel::IReport*
__cdecl GetReportInstantiator()
{
    return new Kernel::MalariaImmunityReport();
}

#ifdef __cplusplus
}
#endif

//******************************************************************************

// ----------------------------------------
// --- ImmunityData Methods
// ----------------------------------------

namespace Kernel
{
    ImmunityData::ImmunityData()
    : IIntervalData()
    , sum_population_by_agebin()
    , sum_MSP_by_agebin()
    , sum_nonspec_by_agebin()
    , sum_pfemp1_by_agebin()
    , sumsqr_MSP_by_agebin()
    , sumsqr_nonspec_by_agebin()
    , sumsqr_pfemp1_by_agebin()
    {
    }

    ImmunityData::~ImmunityData()
    {
    }

    void ImmunityData::SetVectorSize( int size )
    {
        sum_population_by_agebin.resize( size, 0 );
        sum_MSP_by_agebin.resize(        size, 0 );
        sum_nonspec_by_agebin.resize(    size, 0 );
        sum_pfemp1_by_agebin.resize(     size, 0 );
        sumsqr_MSP_by_agebin.resize(     size, 0 );
        sumsqr_nonspec_by_agebin.resize( size, 0 );
        sumsqr_pfemp1_by_agebin.resize(  size, 0 );
    }

    void ImmunityData::Clear()
    {
        std::fill( sum_population_by_agebin.begin(), sum_population_by_agebin.end(), 0 );
        std::fill( sum_MSP_by_agebin.begin(),        sum_MSP_by_agebin.end(),        0 );
        std::fill( sum_nonspec_by_agebin.begin(),    sum_nonspec_by_agebin.end(),    0 );
        std::fill( sum_pfemp1_by_agebin.begin(),     sum_pfemp1_by_agebin.end(),     0 );
        std::fill( sumsqr_MSP_by_agebin.begin(),     sumsqr_MSP_by_agebin.end(),     0 );
        std::fill( sumsqr_nonspec_by_agebin.begin(), sumsqr_nonspec_by_agebin.end(), 0 );
        std::fill( sumsqr_pfemp1_by_agebin.begin(),  sumsqr_pfemp1_by_agebin.end(),  0 );
    }

    void ImmunityData::Update( const IIntervalData& rOtherData )
    {
        const ImmunityData& rOther = static_cast<const ImmunityData&>(rOtherData);

        ReportUtilities::AddVector( this->sum_population_by_agebin , rOther.sum_population_by_agebin );
        ReportUtilities::AddVector( this->sum_MSP_by_agebin        , rOther.sum_MSP_by_agebin        );
        ReportUtilities::AddVector( this->sum_nonspec_by_agebin    , rOther.sum_nonspec_by_agebin    );
        ReportUtilities::AddVector( this->sum_pfemp1_by_agebin     , rOther.sum_pfemp1_by_agebin     );
        ReportUtilities::AddVector( this->sumsqr_MSP_by_agebin     , rOther.sumsqr_MSP_by_agebin     );
        ReportUtilities::AddVector( this->sumsqr_nonspec_by_agebin , rOther.sumsqr_nonspec_by_agebin );
        ReportUtilities::AddVector( this->sumsqr_pfemp1_by_agebin  , rOther.sumsqr_pfemp1_by_agebin  );
    }

    void ImmunityData::Serialize(json::Object& root)
    {
        ReportUtilities::SerializeVector( root, "sum_population" , sum_population_by_agebin );
        ReportUtilities::SerializeVector( root, "sum_MSP"        , sum_MSP_by_agebin        );
        ReportUtilities::SerializeVector( root, "sum_nonspec"    , sum_nonspec_by_agebin    );
        ReportUtilities::SerializeVector( root, "sum_pfemp1"     , sum_pfemp1_by_agebin     );
        ReportUtilities::SerializeVector( root, "sumsqr_MSP"     , sumsqr_MSP_by_agebin     );
        ReportUtilities::SerializeVector( root, "sumsqr_nonspec" , sumsqr_nonspec_by_agebin );
        ReportUtilities::SerializeVector( root, "sumsqr_pfemp1"  , sumsqr_pfemp1_by_agebin  );
    }

    void ImmunityData::Deserialize(json::Object& root)
    {
        ReportUtilities::DeserializeVector( root, true, "sum_population" , sum_population_by_agebin );
        ReportUtilities::DeserializeVector( root, true, "sum_MSP"        , sum_MSP_by_agebin        );
        ReportUtilities::DeserializeVector( root, true, "sum_nonspec"    , sum_nonspec_by_agebin    );
        ReportUtilities::DeserializeVector( root, true, "sum_pfemp1"     , sum_pfemp1_by_agebin     );
        ReportUtilities::DeserializeVector( root, true, "sumsqr_MSP"     , sumsqr_MSP_by_agebin     );
        ReportUtilities::DeserializeVector( root, true, "sumsqr_nonspec" , sumsqr_nonspec_by_agebin );
        ReportUtilities::DeserializeVector( root, true, "sumsqr_pfemp1"  , sumsqr_pfemp1_by_agebin  );
    }


// ----------------------------------------
// --- MalariaImmunityReport Methods
// ----------------------------------------

    MalariaImmunityReport::MalariaImmunityReport() 
        : BaseEventReportIntervalOutput( _module, false, new ImmunityData(), new ImmunityData() ) //false => only one file
        , ages()
        , m_pImmunityData(nullptr)
        , MSP_mean_by_agebin()
        , MSP_std_by_agebin()
        , nonspec_mean_by_agebin()
        , nonspec_std_by_agebin()
        , PfEMP1_mean_by_agebin()
        , PfEMP1_std_by_agebin()
    {
    }

    bool MalariaImmunityReport::Configure( const Configuration * inputJson )
    {
        if( inputJson->Exist("Age_Bins") )
        {
            initConfigTypeMap("Age_Bins", &ages, "Age Bins (in years) to aggregate within and report", 0, MAX_HUMAN_AGE, true);
        }
        else
        {
            ages.push_back(   10.0f );
            ages.push_back(   20.0f );
            ages.push_back(   30.0f );
            ages.push_back(   40.0f );
            ages.push_back(   50.0f );
            ages.push_back(   60.0f );
            ages.push_back(   70.0f );
            ages.push_back(   80.0f );
            ages.push_back(   90.0f );
            ages.push_back(  100.0f );
            ages.push_back( 1000.0f );
        }

        bool configured = BaseEventReportIntervalOutput::Configure( inputJson );

        if( configured )
        {
            static_cast<ImmunityData*>(m_pIntervalData         )->SetVectorSize( ages.size() );
            static_cast<ImmunityData*>(m_pMulticoreDataExchange)->SetVectorSize( ages.size() );
            m_pImmunityData = static_cast<ImmunityData*>(m_pIntervalData);
        }

        return configured;
    }

    MalariaImmunityReport::~MalariaImmunityReport()
    {
    }

    bool MalariaImmunityReport::notifyOnEvent( IIndividualHumanEventContext *context, const EventTrigger::Enum& trigger )
    {
        LOG_DEBUG_F( "MalariaImmunityReport notified of event by %d-year old individual.\n", (int) (context->GetAge() / DAYSPERYEAR) );

        // individual context for suid
        IIndividualHumanContext * iindividual = NULL;
        if (s_OK != context->QueryInterface(GET_IID(IIndividualHumanContext), (void**)&iindividual) )
        {
            throw QueryInterfaceException(__FILE__, __LINE__, __FUNCTION__, "context", "IIndividualHumanContext", "IIndividualHumanEventContext");
        }
        m_has_data = true ;

        int    id        = iindividual->GetSuid().data;
        double mc_weight = context->GetMonteCarloWeight();
        double age       = context->GetAge();
        int    agebin    = ReportUtilities::GetAgeBin( (float)age, ages );

        m_pImmunityData->sum_population_by_agebin.at(agebin) += mc_weight;

        // get malaria contexts
        IMalariaHumanContext * individual_malaria = NULL;
        if (s_OK != context->QueryInterface(GET_IID(IMalariaHumanContext), (void**)&individual_malaria) )
        {
            throw QueryInterfaceException(__FILE__, __LINE__, __FUNCTION__, "context", "IMalariaHumanContext", "IIndividualHumanEventContext");
        }
        IMalariaSusceptibility* susceptibility_malaria = individual_malaria->GetMalariaSusceptibilityContext();

        // push back fraction of immune variants to which individual has been exposed
        double msp = susceptibility_malaria->get_fraction_of_variants_with_antibodies(Kernel::MalariaAntibodyType::MSP1);
        double mspw = mc_weight * msp;
        m_pImmunityData->sum_MSP_by_agebin.at(agebin)    += mspw;
        m_pImmunityData->sumsqr_MSP_by_agebin.at(agebin) += mspw * mspw;

        double nonspec = susceptibility_malaria->get_fraction_of_variants_with_antibodies(Kernel::MalariaAntibodyType::PfEMP1_minor);
        double nonspecw = mc_weight * nonspec;
        m_pImmunityData->sum_nonspec_by_agebin.at(agebin)    += nonspecw;
        m_pImmunityData->sumsqr_nonspec_by_agebin.at(agebin) += nonspecw * nonspecw;

        double pfemp1 = susceptibility_malaria->get_fraction_of_variants_with_antibodies(Kernel::MalariaAntibodyType::PfEMP1_major);
        double pfemp1w = mc_weight * pfemp1;
        m_pImmunityData->sum_pfemp1_by_agebin.at(agebin)    += pfemp1w;
        m_pImmunityData->sumsqr_pfemp1_by_agebin.at(agebin) += pfemp1w * pfemp1w;

        return true;
    }

    double CalcStdDev( double sumsqr, double num, double mean )
    {
        double var = (sumsqr / num) - (mean * mean);
        double std_dev = 0.0;
        if( var > 0.0 )
        {
            std_dev = sqrt( var );
        }
        return std_dev;
    }

    void MalariaImmunityReport::AccumulateOutput()
    {
        LOG_INFO_F("Aggregating output on reporting interval of %d days\n", (int)m_reporting_interval);

        agebinned_t msp_means;
        agebinned_t nonspec_means;
        agebinned_t pfemp1_means;

        agebinned_t msp_stds;
        agebinned_t nonspec_stds;
        agebinned_t pfemp1_stds;

        for(int i = 0; i<ages.size(); i++)
        {
            double sum_pop = m_pImmunityData->sum_population_by_agebin.at(i);
            if( sum_pop > 0 )
            {

                double mean_msp     = m_pImmunityData->sum_MSP_by_agebin.at(i)     / sum_pop;
                double mean_nonspec = m_pImmunityData->sum_nonspec_by_agebin.at(i) / sum_pop;
                double mean_pfemp1  = m_pImmunityData->sum_pfemp1_by_agebin.at(i)  / sum_pop;

                msp_means.push_back(     mean_msp     );
                nonspec_means.push_back( mean_nonspec );
                pfemp1_means.push_back(  mean_pfemp1  );

                msp_stds.push_back(     CalcStdDev( m_pImmunityData->sumsqr_MSP_by_agebin.at(i),     sum_pop, mean_msp     ) );
                nonspec_stds.push_back( CalcStdDev( m_pImmunityData->sumsqr_nonspec_by_agebin.at(i), sum_pop, mean_nonspec ) );
                pfemp1_stds.push_back(  CalcStdDev( m_pImmunityData->sumsqr_pfemp1_by_agebin.at(i),  sum_pop, mean_pfemp1  ) );
            }
            else
            {
                msp_means.push_back(0.0);
                nonspec_means.push_back(0.0);
                pfemp1_means.push_back(0.0);

                msp_stds.push_back(0.0);
                nonspec_stds.push_back(0.0);
                pfemp1_stds.push_back(0.0);
            }
        }

        MSP_mean_by_agebin.push_back(     msp_means     );
        nonspec_mean_by_agebin.push_back( nonspec_means );
        PfEMP1_mean_by_agebin.push_back(  pfemp1_means  );
        MSP_std_by_agebin.push_back(      msp_stds      );
        nonspec_std_by_agebin.push_back(  nonspec_stds  );
        PfEMP1_std_by_agebin.push_back(   pfemp1_stds   );
    }

    void MalariaImmunityReport::SerializeOutput( float currentTime, json::Object& root )
    {
        ReportUtilities::SerializeVector( root, "Age Bins" , ages );

        ReportUtilities::SerializeVector( root, "MSP Mean by Age Bin"            , MSP_mean_by_agebin     );
        ReportUtilities::SerializeVector( root, "Non-Specific Mean by Age Bin"   , nonspec_mean_by_agebin );
        ReportUtilities::SerializeVector( root, "PfEMP1 Mean by Age Bin"         , PfEMP1_mean_by_agebin  );
        ReportUtilities::SerializeVector( root, "MSP StdDev by Age Bin"          , MSP_std_by_agebin      );
        ReportUtilities::SerializeVector( root, "Non-Specific StdDev by Age Bin" , nonspec_std_by_agebin  );
        ReportUtilities::SerializeVector( root, "PfEMP1 StdDev by Age Bin"       , PfEMP1_std_by_agebin   );
    }
}
