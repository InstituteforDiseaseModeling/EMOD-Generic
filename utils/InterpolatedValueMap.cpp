/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"
#include "InterpolatedValueMap.h"

#include "Log.h"

SETUP_LOGGING( "InterpolatedValueMap" )

#define IVM_TIMES   "Times"
#define IVM_VALUE   "Values"

namespace Kernel
{
    InterpolatedValueMap::InterpolatedValueMap( float min_time, 
                                                float max_time,
                                                float min_value,
                                                float max_value )
        : m_TimeValueMap()
        , m_MinTime( min_time )
        , m_MaxTime( max_time )
        , m_MinValue( min_value )
        , m_MaxValue( max_value )
    { }

    InterpolatedValueMap::InterpolatedValueMap(const InterpolatedValueMap& existing_instance)
        : m_TimeValueMap()
        , m_MinTime(existing_instance.m_MinTime)
        , m_MaxTime(existing_instance.m_MaxTime)
        , m_MinValue(existing_instance.m_MinValue)
        , m_MaxValue(existing_instance.m_MaxValue)
    {
        m_TimeValueMap.insert(existing_instance.m_TimeValueMap.begin(), existing_instance.m_TimeValueMap.end());
    }

    bool InterpolatedValueMap::Configure( const Configuration * inputJson )
    {
        std::vector<float> times, values;
        initConfigTypeMap( IVM_TIMES, &times,  Interpolated_Value_Map_Times_DESC_TEXT, m_MinTime,  m_MaxTime, true );
        initConfigTypeMap( IVM_VALUE, &values, Interpolated_Value_Map_Value_DESC_TEXT, m_MinValue, m_MaxValue );

        bool ret = JsonConfigurable::Configure( inputJson );

        if( ret && !JsonConfigurable::_dryrun )
        {
            if( times.size() != values.size() )
            {
                std::stringstream ss;
                ss << "The number of elements in '" << IVM_TIMES << "' (=" << times.size() << ") "
                   << "does not match the number of elements in '" << IVM_VALUE << "' (=" << values.size() << ")." ;
                throw GeneralConfigurationException( __FILE__, __LINE__, __FUNCTION__, ss.str().c_str() );
            }
            for( int i = 0; i < times.size(); ++i )
            {
                m_TimeValueMap.insert( std::make_pair( times[ i ], values[ i ] ) );
            }
        }

        return ret;
    }

    size_t InterpolatedValueMap::size() const
    {
        return m_TimeValueMap.size();
    }

    std::map<float, float>::const_iterator InterpolatedValueMap::begin() const
    {
        return m_TimeValueMap.begin();
    }

    std::map<float, float>::const_iterator InterpolatedValueMap::end() const
    {
        return m_TimeValueMap.end();
    }

    std::map<float, float>::const_reverse_iterator InterpolatedValueMap::rbegin() const
    {
        return m_TimeValueMap.rbegin();
    }

    std::map<float, float>::const_reverse_iterator InterpolatedValueMap::rend() const
    {
        return m_TimeValueMap.rend();
    }

    void InterpolatedValueMap::add( float time, float value )
    {
        m_TimeValueMap.insert( std::make_pair( time, value ) );
    }

    float InterpolatedValueMap::getValuePiecewiseConstant( float current_time, float default_value ) const
    {
        // Returns default value if empty or before first time point
        float ret_rdd = default_value;

        for( auto &entry: m_TimeValueMap  )
        {
            auto map_time = entry.first;
            if( map_time > current_time )
            {
                break;
            }
            ret_rdd = m_TimeValueMap.at( float(map_time) );
        }

        return ret_rdd;
    }


    float InterpolatedValueMap::getValueLinearInterpolation( float current_time, float default_value ) const
    {
        // Returns default value if empty or before first time point
        if (m_TimeValueMap.empty() || m_TimeValueMap.begin()->first > current_time)
        {
            return default_value;
        }

        float map_time, previous_time;
        float map_value, previous_value;

        previous_time  = m_TimeValueMap.begin()->first;
        previous_value = m_TimeValueMap.at(previous_time);
        for (auto &entry : m_TimeValueMap)
        {
            map_time = entry.first;
            map_value = m_TimeValueMap.at(map_time);

            if (map_time > current_time)
            {
                if (map_time == previous_time)
                {
                    return m_TimeValueMap.at(map_time);
                }
                return ((current_time - previous_time)/(map_time - previous_time)) * (map_value - previous_value) + previous_value;
            }

            previous_time = map_time;
            previous_value = map_value;
        }

        return map_value;
    }

    bool InterpolatedValueMap::isAtEnd( float current_time ) const
    {
        if (m_TimeValueMap.empty())
        {
            return true;
        }

        return (m_TimeValueMap.rbegin()->first <= current_time);
    }

    void InterpolatedValueMap::serialize( IArchive& ar, InterpolatedValueMap& mapping )
    {
        size_t count = ar.IsWriter() ? mapping.m_TimeValueMap.size() : -1;

        ar.startArray(count);
        if( ar.IsWriter() )
        {
            for( auto& entry : mapping.m_TimeValueMap )
            {
                float key   = entry.first;
                float value = entry.second;
                ar.startObject();
                    ar.labelElement("key"  ) & key;
                    ar.labelElement("value") & value;
                ar.endObject();
            }
        }
        else
        {
            for (size_t i = 0; i < count; ++i)
            {
                float key=0.0;
                float value=0.0;
                ar.startObject();
                    ar.labelElement("key"  ) & key;
                    ar.labelElement("value") & value;
                ar.endObject();
                mapping.m_TimeValueMap[key] = value;
            }
        }
        ar.endArray();
    }
}
