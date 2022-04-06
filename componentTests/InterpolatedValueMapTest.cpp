/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"
#include "UnitTest++.h"
#include "componentTests.h"

#include "FileSystem.h"
#include "InterpolatedValueMap.h"

#include <string>
#include <climits>
#include <vector>
#include <iostream>
#include <memory> // unique_ptr

using namespace Kernel;
using namespace std;


SUITE(InterpolatedValueMapTest)
{
    TEST(TestValid)
    {
        InterpolatedValueMap map ;

        unique_ptr<Configuration> p_config( Environment::LoadConfigurationFile( "testdata/InterpolatedValueMapTest/Valid.json" ) );

        map.Configure( p_config.get() );

        CHECK_EQUAL( 0.0, map.getValueLinearInterpolation(  999 ) );
        CHECK_EQUAL( 1.0, map.getValueLinearInterpolation( 1000 ) );
        CHECK_EQUAL( 1.5, map.getValueLinearInterpolation( 1500 ) );
        CHECK_EQUAL( 2.0, map.getValueLinearInterpolation( 2000 ) );
        CHECK_EQUAL( 2.5, map.getValueLinearInterpolation( 2500 ) );
        CHECK_EQUAL( 3.0, map.getValueLinearInterpolation( 3000 ) );
        CHECK_EQUAL( 3.0, map.getValueLinearInterpolation( 3001 ) );
    }

    void TestHelper_Exception( int lineNumber, const std::string& rFilename, const std::string& rExpMsg )
    {
        try
        {

            unique_ptr<Configuration> p_config( Environment::LoadConfigurationFile( rFilename ) );

            InterpolatedValueMap map ;

            map.Configure( p_config.get() );

            CHECK_LN( false, lineNumber ); // should not get here
        }
        catch( DetailedException& re )
        {
            std::string msg = re.GetMsg();
            bool passed = msg.find( rExpMsg ) != string::npos ;
            if( !passed )
            {
                PrintDebug( "\n=== Expected ===\n" );
                PrintDebug( rExpMsg );
                PrintDebug( "\n=== Actual ===\n" );
                PrintDebug( msg );
                PrintDebug( "\n" );
            }
            CHECK_LN( passed, lineNumber );
        }
    }

    TEST(TestNotSameNumberOfElementsA)
    {
        TestHelper_Exception( __LINE__, "testdata/InterpolatedValueMapTest/TestNotSameNumberOfElementsA.json",
            "The number of elements in 'Times' (=3) does not match the number of elements in 'Values' (=2)." );
    }

    TEST(TestNotSameNumberOfElementsB)
    {
        TestHelper_Exception( __LINE__, "testdata/InterpolatedValueMapTest/TestNotSameNumberOfElementsB.json",
            "The number of elements in 'Times' (=2) does not match the number of elements in 'Values' (=3)." );
    }

    TEST(TestNegativeTime)
    {
        TestHelper_Exception( __LINE__, "testdata/InterpolatedValueMapTest/TestNegativeTime.json",
            "Configuration variable 'Times' with value -2000 out of range: less than 0." );
    }

    TEST(TestNegativeValue)
    {
        TestHelper_Exception( __LINE__, "testdata/InterpolatedValueMapTest/TestNegativeValue.json",
            "Configuration variable 'Values' with value -2 out of range: less than 0." );
    }

    TEST(TestTimeTooLarge)
    {
        TestHelper_Exception( __LINE__, "testdata/InterpolatedValueMapTest/TestTimeTooLarge.json",
            "Configuration variable 'Times' with value 1e+06 out of range: greater than 999999." );
    }

    TEST(TestBadTimeOrder)
    {
        TestHelper_Exception( __LINE__, "testdata/InterpolatedValueMapTest/TestBadTimeOrder.json",
            "The values in Times must be unique and in ascending order." );
    }
}