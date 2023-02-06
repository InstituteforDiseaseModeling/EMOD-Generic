/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <memory> // unique_ptr
#include "UnitTest++.h"
#include "IWaningEffect.h"
#include "componentTests.h"

using namespace Kernel; 



SUITE(WaningEffectMapTest)
{
    TEST(TestPiecewiseGetMultiplier)
    {
        unique_ptr<Configuration> p_config( Environment::LoadConfigurationFile( "testdata/WaningEffectMapTest.json" ) );

        IWaningEffect* piecewise = WaningEffectFactory::CreateInstance();
        IIndividualHumanContext* p_indiv = nullptr;

        piecewise->Configure( p_config.get() );
        piecewise->SetContextTo(p_indiv);

        CHECK_EQUAL( 0.1f, piecewise->Current() );
        piecewise->Update(0.5f);
        CHECK_EQUAL( 0.1f, piecewise->Current() );
        piecewise->Update(0.5f);
        CHECK_EQUAL( 0.1f, piecewise->Current() );
        piecewise->Update(0.5f);
        CHECK_EQUAL( 0.1f, piecewise->Current() );
        piecewise->Update(0.5f);
        CHECK_EQUAL( 0.3f, piecewise->Current() );
        piecewise->Update(0.5f);
        CHECK_EQUAL( 0.3f, piecewise->Current() );
        piecewise->Update(0.5f);
        CHECK_EQUAL( 0.5f, piecewise->Current() );
        piecewise->Update(0.5f);
        CHECK_EQUAL( 0.5f, piecewise->Current() );
        piecewise->Update(0.5f);
        CHECK_EQUAL( 0.7f, piecewise->Current() );
        piecewise->Update(0.5f);
        CHECK_EQUAL( 0.7f, piecewise->Current() );
        piecewise->Update(0.5f);
        CHECK_EQUAL( 0.5f, piecewise->Current() );
        piecewise->Update(0.5f);
        CHECK_EQUAL( 0.5f, piecewise->Current() );
        piecewise->Update(0.5f);
        CHECK_EQUAL( 0.3f, piecewise->Current() );
        piecewise->Update(0.5f);
        CHECK_EQUAL( 0.1f, piecewise->Current() );
        piecewise->Update(0.5f);
        CHECK_EQUAL( 0.1f, piecewise->Current() );

        delete piecewise;
    }
}