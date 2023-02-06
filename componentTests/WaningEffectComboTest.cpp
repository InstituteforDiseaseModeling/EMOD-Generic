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
#include "componentTests.h"

#include "IWaningEffect.h"
#include "IndividualHumanContextFake.h"

using namespace Kernel;

SUITE( WaningEffectComboTest )
{
    TEST( TestRead )
    {
        unique_ptr<Configuration> p_config( Environment::LoadConfigurationFile( "testdata/WaningEffectComboTest/TestRead.json" ) );

        IWaningEffect* combo = WaningEffectFactory::CreateInstance();

        try
        {
            combo->Configure( p_config.get() );
        }
        catch( DetailedException& re )
        {
            PrintDebug( re.GetMsg() );
            CHECK( false );
        }

        unique_ptr<IndividualHumanContextFake> p_human( new IndividualHumanContextFake( nullptr, nullptr, nullptr, nullptr ) );

        combo->SetContextTo( p_human.get() );

        p_human->SetAge( 12.0*DAYSPERYEAR );
        combo->Update( 1.0 );
        float current = combo->Current();
        CHECK_CLOSE( 0.0, current, 0.000001 ); // zero because age is < 13

        p_human->SetAge( 15.0*DAYSPERYEAR );
        combo->Update( 1.0 );
        current = combo->Current();
        CHECK_CLOSE( 0.751477, current, 0.000001 ); // age > 13 & second day of exponential with 7-day decay

        combo->Update( 1.0 );
        current = combo->Current();
        CHECK_CLOSE( 0.651439, current, 0.000001 ); // age > 13 & third day of exponential with 7-day decay

        combo->Update( 1.0 );
        current = combo->Current();
        CHECK_CLOSE( 0.564718, current, 0.000001 ); // age > 13 & fourth day of exponential with 7-day decay

        combo->Update( 1.0 );
        current = combo->Current();
        CHECK_CLOSE( 0.489542, current, 0.000001 ); // age > 13 & fifth day of exponential with 7-day decay
    }

    TEST( TestExpireFirst )
    {
        unique_ptr<Configuration> p_config( Environment::LoadConfigurationFile( "testdata/WaningEffectComboTest/TestExpireFirst.json" ) );

        IWaningEffect* combo = WaningEffectFactory::CreateInstance();

        try
        {
            combo->Configure( p_config.get() );
        }
        catch( DetailedException& re )
        {
            PrintDebug( re.GetMsg() );
            CHECK( false );
        }

        unique_ptr<IndividualHumanContextFake> p_human( new IndividualHumanContextFake( nullptr, nullptr, nullptr, nullptr ) );
        p_human->SetAge( 12.0*DAYSPERYEAR );

        combo->SetContextTo( p_human.get() );

        combo->Update( 1.0 );
        float current = combo->Current();
        CHECK_CLOSE( 0.1, current, 0.000001 ); // 0.2 from Linear times 0.5 from Box
        CHECK_EQUAL( false, combo->Expired() ); 

        combo->Update( 1.0 );
        current = combo->Current();
        CHECK_CLOSE( 0.2, current, 0.000001 ); // 0.4 from Linear times 0.5 from Box
        CHECK_EQUAL( false, combo->Expired() );

        combo->Update( 1.0 );
        current = combo->Current();
        CHECK_CLOSE( 0.3, current, 0.000001 ); // 0.6 from Linear times 0.5 from Box
    }

    TEST( TestExpireAll )
    {
        unique_ptr<Configuration> p_config( Environment::LoadConfigurationFile( "testdata/WaningEffectComboTest/TestExpireAll.json" ) );

        IWaningEffect* combo = WaningEffectFactory::CreateInstance();

        try
        {
            combo->Configure( p_config.get() );
        }
        catch( DetailedException& re )
        {
            PrintDebug( re.GetMsg() );
            CHECK( false );
        }

        unique_ptr<IndividualHumanContextFake> p_human( new IndividualHumanContextFake( nullptr, nullptr, nullptr, nullptr ) );
        p_human->SetAge( 12.0*DAYSPERYEAR );

        combo->SetContextTo( p_human.get() );

        combo->Update( 1.0 );
        float current = combo->Current();
        CHECK_CLOSE( 0.1, current, 0.000001 ); // 0.2 from Linear times 0.5 from Piecewise
        CHECK_EQUAL( false, combo->Expired() );

        combo->Update( 1.0 );
        current = combo->Current();
        CHECK_CLOSE( 0.2, current, 0.000001 ); // 0.4 from Linear times 0.5 from Piecewise
        CHECK_EQUAL( false, combo->Expired() );

        combo->Update( 1.0 );
        current = combo->Current();
        CHECK_CLOSE( 0.3, current, 0.000001 ); // 0.6 from Linear times 0.5 from Piecewise
        CHECK_EQUAL( false, combo->Expired() ); // end of Linear is time 2 but Box expires after 5

        combo->Update( 1.0 );
        current = combo->Current();
        CHECK_CLOSE( 0.3, current, 0.000001 ); // 0.6 from Linear times 0.5 from Piecewise
        CHECK_EQUAL( false, combo->Expired() ); // end of Linear is time 2 but Box expires after 5

        combo->Update( 1.0 );
        current = combo->Current();
        CHECK_CLOSE( 0.3, current, 0.000001 ); // 0.6 from Linear times 0.5 from Piecewise
        CHECK_EQUAL( false, combo->Expired() ); // end of Linear is time 2 but Box expires after 5
    }

    TEST( TestAdd )
    {
        unique_ptr<Configuration> p_config( Environment::LoadConfigurationFile( "testdata/WaningEffectComboTest/TestAdd.json" ) );

        IWaningEffect* combo = WaningEffectFactory::CreateInstance();

        try
        {
            combo->Configure( p_config.get() );
        }
        catch( DetailedException& re )
        {
            PrintDebug( re.GetMsg() );
            CHECK( false );
        }

        unique_ptr<IndividualHumanContextFake> p_human( new IndividualHumanContextFake( nullptr, nullptr, nullptr, nullptr ) );
        p_human->SetAge( 12.0*DAYSPERYEAR );

        combo->SetContextTo( p_human.get() );

        combo->Update( 1.0 );
        float current = combo->Current();
        CHECK_CLOSE( 0.7, current, 0.000001 ); // 0.2 from Linear plus 0.5 from Box
        CHECK_EQUAL( false, combo->Expired() );

        combo->Update( 1.0 );
        current = combo->Current();
        CHECK_CLOSE( 0.9, current, 0.000001 ); // 0.4 from Linear plus 0.5 from Box
        CHECK_EQUAL( false, combo->Expired() );

        combo->Update( 1.0 );
        current = combo->Current();
        CHECK_CLOSE( 1.0, current, 0.000001 ); // 0.6 from Linear plus 0.5 from Box = capped at 1.0
    }

    TEST( TestSetInitial )
    {
        unique_ptr<Configuration> p_config( Environment::LoadConfigurationFile( "testdata/WaningEffectComboTest/TestSetInitial.json" ) );

        IWaningEffect* combo = WaningEffectFactory::CreateInstance();

        try
        {
            combo->Configure( p_config.get() );
        }
        catch( DetailedException& re )
        {
            PrintDebug( re.GetMsg() );
            CHECK( false );
        }

        unique_ptr<IndividualHumanContextFake> p_human( new IndividualHumanContextFake( nullptr, nullptr, nullptr, nullptr ) );
        p_human->SetAge( 12.0*DAYSPERYEAR );

        combo->SetContextTo( p_human.get() );

        combo->SetInitial( 0.25 );

        combo->Update( 1.0 );
        float current = combo->Current();
        CHECK_CLOSE( 0.175, current, 0.000001 );
        CHECK_EQUAL( false, combo->Expired() );

        combo->Update( 1.0 );
        current = combo->Current();
        CHECK_CLOSE( 0.225, current, 0.000001 );
        CHECK_EQUAL( false, combo->Expired() );

        combo->Update( 1.0 );
        current = combo->Current();
        CHECK_CLOSE( 0.250, current, 0.000001 );
    }
}
