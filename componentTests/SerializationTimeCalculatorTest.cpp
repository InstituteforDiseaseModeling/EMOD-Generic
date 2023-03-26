/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <memory>
#include "UnitTest++.h"
#include "FakeLogger.h"

#include "SerializationTimeCalc.h"
#include "Exceptions.h"

using namespace std; 
using namespace Kernel; 

SUITE(SerializationTimeCalculatorTest)
{
    struct SerializationTimeCalculatorFixture
    {
        FakeLogger m_fakeLogger;

        SerializationTimeCalculatorFixture()
          :  m_fakeLogger(Logger::tLevel::WARNING)
        {
            Environment::Finalize();
            Environment::setLogger(&m_fakeLogger);
        }

        ~SerializationTimeCalculatorFixture()
        {
            Environment::Finalize();
        }
    };

    TEST_FIXTURE(SerializationTimeCalculatorFixture, TestNothingProvided)
    {
        unique_ptr<Configuration> p_config(Environment::LoadConfigurationFile("testdata/SerializationTimeCalculatorTest/TestNothingProvided.json"));
        int steps = 10;
        float start_time = 0.0f;
        float step_size  = 1.0f;

        SerializationTimeCalc stc;
        stc.Configure(p_config.get());
        std::deque<int32_t> serialization_time_steps = stc.GetSerializedTimeSteps(steps, start_time, step_size);

        CHECK(serialization_time_steps.empty());
        CHECK(m_fakeLogger.Empty());
    }
    
    TEST_FIXTURE(SerializationTimeCalculatorFixture, TestSimpleTimeSteps)
    {
        unique_ptr<Configuration> p_config(Environment::LoadConfigurationFile("testdata/SerializationTimeCalculatorTest/TestSimpleTimeSteps.json"));
        int steps = 10;
        float start_time = 0.0f;
        float step_size  = 1.0f;

        SerializationTimeCalc stc;
        stc.Configure(p_config.get());
        std::deque<int32_t> serialization_time_steps = stc.GetSerializedTimeSteps(steps, start_time, step_size);

        std::deque<int32_t> compared(
        { 0, 1, 2 }
        );
        CHECK_ARRAY_EQUAL(serialization_time_steps, compared, compared.size());
        CHECK(m_fakeLogger.Empty());

    }

    TEST_FIXTURE(SerializationTimeCalculatorFixture, TestSimpleTimes)
    {
        unique_ptr<Configuration> p_config(Environment::LoadConfigurationFile("testdata/SerializationTimeCalculatorTest/TestSimpleTimes.json"));
        int steps = 10;
        float start_time = 1.0f;
        float step_size  = 1.5f;

        SerializationTimeCalc stc;
        stc.Configure(p_config.get());
        std::deque<int32_t> serialization_time_steps = stc.GetSerializedTimeSteps(steps, start_time, step_size);

        std::deque<int32_t> compared(
        { 0, 1, 2 }
        );
        CHECK_ARRAY_EQUAL(serialization_time_steps, compared, compared.size());
        CHECK(m_fakeLogger.Empty());
    }

    TEST_FIXTURE(SerializationTimeCalculatorFixture, TestNegativeOneStep)
    {
        unique_ptr<Configuration> p_config(Environment::LoadConfigurationFile("testdata/SerializationTimeCalculatorTest/TestNegativeOneStep.json"));
        int steps = 10;
        float start_time = 0.0f;
        float step_size  = 1.0f;

        SerializationTimeCalc stc;
        stc.Configure(p_config.get());
        std::deque<int32_t> serialization_time_steps = stc.GetSerializedTimeSteps(steps, start_time, step_size);

        std::deque<int32_t> compared(
        { 0, 1, 2, 10 }
        );
        CHECK_ARRAY_EQUAL(serialization_time_steps, compared, compared.size());
        CHECK(m_fakeLogger.Empty());
    }

    TEST_FIXTURE(SerializationTimeCalculatorFixture, TestNegativeOneTime)
    {
        unique_ptr<Configuration> p_config(Environment::LoadConfigurationFile("testdata/SerializationTimeCalculatorTest/TestNegativeOneTime.json"));
        int steps = 10;
        float start_time = 1.0f;
        float step_size  = 1.5f;

        SerializationTimeCalc stc;
        stc.Configure(p_config.get());
        std::deque<int32_t> serialization_time_steps = stc.GetSerializedTimeSteps(steps, start_time, step_size);

        std::deque<int32_t> compared(
        { 0, 1, 2, 10 }
        );
        CHECK_ARRAY_EQUAL(serialization_time_steps, compared, compared.size());
        CHECK(m_fakeLogger.Empty());
    }

    TEST_FIXTURE(SerializationTimeCalculatorFixture, TestZeroTime)
    {
        unique_ptr<Configuration> p_config(Environment::LoadConfigurationFile("testdata/SerializationTimeCalculatorTest/TestZeroTime.json"));
        int steps = 10;
        float start_time = 10.0f;
        float step_size  =  1.5f;

        SerializationTimeCalc stc;
        stc.Configure(p_config.get());
        std::deque<int32_t> serialization_time_steps = stc.GetSerializedTimeSteps(steps, start_time, step_size);

        std::deque<int32_t> compared(
        { 0, 1, 2, 10 }
        );
        CHECK_ARRAY_EQUAL(serialization_time_steps, compared, compared.size());
        CHECK(m_fakeLogger.Empty());
    }

    TEST_FIXTURE(SerializationTimeCalculatorFixture, TestTimesBetweenSteps)
    {
        unique_ptr<Configuration> p_config(Environment::LoadConfigurationFile("testdata/SerializationTimeCalculatorTest/TestTimesBetweenSteps.json"));
        int steps = 10;
        float start_time = 1.0f;
        float step_size  = 1.5f;

        SerializationTimeCalc stc;
        stc.Configure(p_config.get());
        std::deque<int32_t> serialization_time_steps = stc.GetSerializedTimeSteps(steps, start_time, step_size);

        std::deque<int32_t> compared(
        { 0, 1, 2 }
        );
        CHECK_ARRAY_EQUAL(serialization_time_steps, compared, compared.size());
        CHECK(m_fakeLogger.Empty());
    }

    TEST_FIXTURE(SerializationTimeCalculatorFixture, TestDuplicateTimes)
    {
        unique_ptr<Configuration> p_config(Environment::LoadConfigurationFile("testdata/SerializationTimeCalculatorTest/TestDuplicateTimes.json"));
        int steps = 10;
        float start_time = 1.0f;
        float step_size  = 1.5f;

        SerializationTimeCalc stc;
        stc.Configure(p_config.get());
        std::deque<int32_t> serialization_time_steps = stc.GetSerializedTimeSteps(steps, start_time, step_size);

        std::deque<int32_t> compared(
        { 0, 1, 2 }
        );
        CHECK_ARRAY_EQUAL(serialization_time_steps, compared, compared.size());
        LogEntry entry = m_fakeLogger.Back();
        CHECK_EQUAL(entry.log_level, Logger::tLevel::WARNING);
        CHECK(entry.msg.find("Duplicate serialization step specified (step=2, time=4.000000), removed from list\n") != string::npos);
        m_fakeLogger.Pop_Back();
        entry = m_fakeLogger.Back();
        CHECK_EQUAL(entry.log_level, Logger::tLevel::WARNING);
        CHECK(entry.msg.find("Duplicate serialization step specified (step=0, time=1.000000), removed from list\n") != string::npos);
        m_fakeLogger.Pop_Back();
        CHECK(m_fakeLogger.Empty());
    }

    TEST_FIXTURE(SerializationTimeCalculatorFixture, TestDuplicateTimesteps)
    {
        unique_ptr<Configuration> p_config(Environment::LoadConfigurationFile("testdata/SerializationTimeCalculatorTest/TestDuplicateTimesteps.json"));
        int steps = 10;
        float start_time = 0.0f;
        float step_size  = 1.0f;

        SerializationTimeCalc stc;
        stc.Configure(p_config.get());
        std::deque<int32_t> serialization_time_steps = stc.GetSerializedTimeSteps(steps, start_time, step_size);

        std::deque<int32_t> compared(
        { 0, 1, 2 }
        );
        CHECK_ARRAY_EQUAL(serialization_time_steps, compared, compared.size());
        LogEntry entry = m_fakeLogger.Back();
        CHECK_EQUAL(entry.log_level, Logger::tLevel::WARNING);
        CHECK(entry.msg.find("Duplicate serialization step specified (step=2, time=2.000000), removed from list\n") != string::npos);
        m_fakeLogger.Pop_Back();
        entry = m_fakeLogger.Back();
        CHECK_EQUAL(entry.log_level, Logger::tLevel::WARNING);
        CHECK(entry.msg.find("Duplicate serialization step specified (step=0, time=0.000000), removed from list\n") != string::npos);
        m_fakeLogger.Pop_Back();
        CHECK(m_fakeLogger.Empty());
    }

    TEST_FIXTURE(SerializationTimeCalculatorFixture, TestTooEarlyTime)
    {
        unique_ptr<Configuration> p_config(Environment::LoadConfigurationFile("testdata/SerializationTimeCalculatorTest/TestTooEarlyTime.json"));
        int steps = 10;
        float start_time = 1.0f;
        float step_size  = 1.5f;

        try
        {
            SerializationTimeCalc stc;
            stc.Configure(p_config.get());
            std::deque<int32_t> serialization_time_steps = stc.GetSerializedTimeSteps(steps, start_time, step_size);
            CHECK(false); // Should not get here
        }
        catch (ConfigurationRangeException ex)
        {
            // Breaking the check across two messages to get around path being included in the exception
            std::string exMsg(ex.GetMsg());
            CHECK(exMsg.find("ConfigurationRangeException:") != string::npos); 
            CHECK(exMsg.find("Configuration variable Serialization_Times with value -3 out of range: less than -1.") != string::npos);
            CHECK(m_fakeLogger.Empty());
        }
    }

    TEST_FIXTURE(SerializationTimeCalculatorFixture, TestTooLateTime)
    {
        unique_ptr<Configuration> p_config(Environment::LoadConfigurationFile("testdata/SerializationTimeCalculatorTest/TestTooLateTime.json"));
        int steps = 10;
        float start_time = 1.0f;
        float step_size  = 1.5f;

        SerializationTimeCalc stc;
        stc.Configure(p_config.get());
        std::deque<int32_t> serialization_time_steps = stc.GetSerializedTimeSteps(steps, start_time, step_size);

        std::deque<int32_t> compared(
        { 0, 1, 2 }
        );
        CHECK_ARRAY_EQUAL(serialization_time_steps, compared, compared.size());
        LogEntry entry = m_fakeLogger.Back();
        CHECK_EQUAL(entry.log_level, Logger::tLevel::WARNING);
        CHECK(entry.msg.find("Out of range serialization step specified (step=11, time=17.500000), removed from list\n") != string::npos);
        m_fakeLogger.Pop_Back();
        CHECK(m_fakeLogger.Empty());
    }
}
