/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"
#include "UnitTest++.h"
#include "CampaignEventByYear.h"
#include "IdmMpi.h"
#include "EventTrigger.h"
#include "Simulation.h"
#include "FakeLogger.h"
#include "componentTests.h"
#include <memory> // unique_ptr

using namespace std; 
using namespace Kernel; 

SUITE(CampaignEventByYearTest)
{
    static int m_NextId = 1 ;

    struct CampaignEventByYearFixture
    {
        IdmMpi::MessageInterface* m_pMpi;
        float m_oldBaseYear;

        CampaignEventByYearFixture()
        {
            JsonConfigurable::ClearMissingParameters();
            Environment::Finalize();
            Environment::setLogger( new FakeLogger( Logger::tLevel::WARNING ) );

            m_pMpi = IdmMpi::MessageInterface::CreateNull();

            int argc      = 1;
            char* exeName = "componentTests.exe";
            char** argv   = &exeName;
            string configFilename("testdata/CampaignEventByYearTest/config.json");
            string inputPath("testdata/CampaignEventByYearTest");
            string outputPath("testdata/CampaignEventByYearTest/output");
            string statePath("testdata/CampaignEventByYearTest");
            string dllPath("testdata/CampaignEventByYearTest");

            Environment::Initialize( m_pMpi, configFilename, inputPath, outputPath, /*statePath, */dllPath, false);
        }

        ~CampaignEventByYearFixture()
        {
            Environment::Finalize();
        }
    };
}
