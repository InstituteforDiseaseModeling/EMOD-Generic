/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include "Configure.h"

namespace Kernel
{
    class IDMAPI InterpolatedValueMap : public JsonConfigurable
    {
        IMPLEMENT_DEFAULT_REFERENCE_COUNTING()
        virtual QueryResult QueryInterface(iid_t iid, void **ppvObject) { return e_NOINTERFACE; }
        public:
            InterpolatedValueMap( float min_time = 0.0f,
                                  float max_time = 999999.0f,
                                  float min_value = 0.0f,
                                  float max_value = FLT_MAX );

            InterpolatedValueMap(const InterpolatedValueMap&);

            virtual bool Configure( const Configuration* inputJson ) override;

            size_t size() const;
            std::map<float, float>::const_iterator begin() const;
            std::map<float, float>::const_iterator end() const;
            std::map<float, float>::const_reverse_iterator rbegin() const;
            std::map<float, float>::const_reverse_iterator rend() const;

            void add( float time, float value );

            float getValuePiecewiseConstant( float current_time, float default_value = 0) const;
            float getValueLinearInterpolation( float current_time, float default_value = 0) const;
            bool isAtEnd( float current_time ) const;

            static void serialize( IArchive& ar, InterpolatedValueMap& map );
        private:
            std::map<float, float> m_TimeValueMap;
            float m_MinTime;
            float m_MaxTime;
            float m_MinValue;
            float m_MaxValue;
    };
}
