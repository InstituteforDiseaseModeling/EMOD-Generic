/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"

#include "WaningEffect.h"
#include "RANDOM.h"
#include "DistributionFactory.h"
#include "IIndividualHumanContext.h"
#include "IndividualEventContext.h"
#include "NodeEventContext.h"

SETUP_LOGGING("WaningEffect")

namespace Kernel
{
    WaningEffect::WaningEffect()
        : m_box_duration(0.0f)
        , m_current_effect(0.0f)
        , m_current_time(0.0f)
        , m_decay_time(0.0f)
        , m_init_effect(0.0f)
        , m_init_effect_const(0.0f)
        , m_init_effect_dist_a(0.0f)
        , m_init_effect_dist_b(0.0f)
        , m_enable_box_dist(false)
        , m_enable_init_dist(false)
        , m_expired(false)
        , m_initialized(false)
        , m_box_duration_dist(nullptr)
        , m_individual_context(nullptr)
        , m_node_event_context(nullptr)
        , m_age_map(0.0f,FLT_MAX,0.0f,1.0f)
        , m_time_map(0.0f,FLT_MAX,0.0f,1.0f)
    { }

    WaningEffect::WaningEffect(const WaningEffect& existing_instance)
        : m_box_duration(existing_instance.m_box_duration)
        , m_current_effect(existing_instance.m_current_effect)
        , m_current_time(existing_instance.m_current_time)
        , m_decay_time(existing_instance.m_decay_time)
        , m_init_effect(existing_instance.m_init_effect)
        , m_init_effect_const(existing_instance.m_init_effect_const)
        , m_init_effect_dist_a(existing_instance.m_init_effect_dist_a)
        , m_init_effect_dist_b(existing_instance.m_init_effect_dist_b)
        , m_enable_box_dist(existing_instance.m_enable_box_dist)
        , m_enable_init_dist(existing_instance.m_enable_init_dist)
        , m_expired(existing_instance.m_expired)
        , m_initialized(false)
        , m_box_duration_dist(nullptr)
        , m_individual_context(nullptr)
        , m_node_event_context(nullptr)
        , m_age_map(existing_instance.m_age_map)
        , m_time_map(existing_instance.m_time_map)
    {
        if(existing_instance.m_box_duration_dist)
        {
            m_box_duration_dist = existing_instance.m_box_duration_dist->Clone();
        }
    }

    WaningEffect::~WaningEffect()
    {
        delete m_box_duration_dist;
        m_box_duration_dist = nullptr;
    }

    IWaningEffect* WaningEffectFactory::CreateInstance()
    {
        return new WaningEffect();
    }

    IWaningEffect* WaningEffect::Clone()
    {
        return new WaningEffect(*this);
    }

    JsonConfigurable* WaningEffect::GetConfigurable()
    {
        return static_cast<JsonConfigurable*>(this);
    }

    bool WaningEffect::Configure(const Configuration* p_config)
    {
        // Parameter dependencies
        const std::map<std::string, std::string> dset_we01 {{"Enable_Box_Duration_Distribution","1"}};
        const std::map<std::string, std::string> dset_we02 {{"Enable_Initial_Effect_Distribution","1"}};

        // Parameters
        initConfigTypeMap("Enable_Box_Duration_Distribution",    &m_enable_box_dist,       WE_Enable_Box_Duration_Distribution_DESC_TEXT,    false);
        initConfigTypeMap("Enable_Initial_Effect_Distribution",  &m_enable_init_dist,      WE_Enable_Initial_Effect_Distribution_DESC_TEXT,  false);

        initConfigTypeMap("Initial_Effect",                      &m_init_effect_const,     WE_Initial_Effect_DESC_TEXT,                        0.0f,     1.0f,    0.0f);
        initConfigTypeMap("Box_Duration",                        &m_box_duration,          WE_Box_Duration_DESC_TEXT,                          0.0f,  FLT_MAX, FLT_MAX);
        initConfigTypeMap("Decay_Time_Constant",                 &m_decay_time,            WE_Decay_Time_Constant_DESC_TEXT,                   0.0f,  FLT_MAX,    0.0f);

        WEBoxDist::Enum box_dist_funct(WEBoxDist::NOT_INITIALIZED);
        initConfig("Box_Duration_Distribution", box_dist_funct, p_config, MetadataDescriptor::Enum("Box_Duration_Distribution", WE_Box_Duration_Distribution_DESC_TEXT, MDD_ENUM_ARGS(WEBoxDist)),  nullptr,  nullptr,  &dset_we01);
        m_box_duration_dist = DistributionFactory::CreateDistribution(this, p_config, "Box_Duration", WEBoxDist::pairs::lookup_key(box_dist_funct), WEBoxDist::pairs::get_keys());

        initConfigTypeMap("Initial_Effect_Distribution_Alpha",   &m_init_effect_dist_a,    WE_Initial_Effect_Distribution_Alpha_DESC_TEXT,  FLT_MIN,  FLT_MAX,  -1.0f,  nullptr,  nullptr,  &dset_we02);
        initConfigTypeMap("Initial_Effect_Distribution_Beta",    &m_init_effect_dist_b,    WE_Initial_Effect_Distribution_Beta_DESC_TEXT,   FLT_MIN,  FLT_MAX,  -1.0f,  nullptr,  nullptr,  &dset_we02);

        initConfigTypeMap("Age_Multiplier",                      &m_age_map,               WE_Age_Multiplier_DESC_TEXT);
        initConfigTypeMap("Durability_Map",                      &m_time_map,              WE_Durability_Map_DESC_TEXT);

        bool ret_bool = JsonConfigurable::Configure(p_config);


        // Box_Duration is always consumed (for backward compatability); not used with distribution, so non-default (non-FLT_MAX) is an error
        if(m_enable_box_dist && m_box_duration<FLT_MAX)
        {
            throw IncoherentConfigurationException( __FILE__, __LINE__, __FUNCTION__, "Box_Duration", m_box_duration, "Enable_Box_Duration_Distribution", static_cast<float>(m_enable_box_dist));
        }

        // User specifing NOT_INITIALIZED after requesting distribution is an error
        if(m_enable_box_dist && box_dist_funct==WEBoxDist::NOT_INITIALIZED)
        {
            throw IncoherentConfigurationException( __FILE__, __LINE__, __FUNCTION__, "Enable_Box_Duration_Distribution", static_cast<float>(m_enable_box_dist), "Box_Duration_Distribution", WEBoxDist::pairs::lookup_key(box_dist_funct).c_str());
        }

        // Initial_Effect is always consumed (for backward compatability); not used with distribution, so non-default (non-zero) value is an error
        if(m_enable_init_dist && m_init_effect_const>0.0f)
        {
            throw IncoherentConfigurationException( __FILE__, __LINE__, __FUNCTION__, "Initial_Effect", m_init_effect_const, "Enable_Initial_Effect_Distribution", static_cast<float>(m_enable_init_dist));
        }

        // Specifying Decay_Time_Constant without Box_Duration reduces default Box_Duration to zero (for backward compatability)
        if(!m_enable_box_dist && m_decay_time > 0.0f && m_box_duration == FLT_MAX)
        {
            m_box_duration = 0.0f;
        }

        return ret_bool;
    }

    void WaningEffect::SetContextTo(IIndividualHumanContext* context)
    {
        m_individual_context = context;
        if(!m_initialized)
        {
            Initialize();
        }

        return;
    }

    void WaningEffect::SetContextTo(INodeEventContext* context)
    {
        if(m_age_map.size())
        {
            throw InitializationException( __FILE__, __LINE__, __FUNCTION__, "Cannot use Age_Multiplier in node intervention." );
        }

        m_node_event_context = context;
        if(!m_initialized)
        {
            Initialize();
        }

        return;
    }

    void WaningEffect::Initialize()
    {
        float init_effect = m_init_effect_const;
        if(m_enable_init_dist)
        {
            if(m_individual_context)
            {
                init_effect = m_individual_context->GetRng()->rand_beta(m_init_effect_dist_a, m_init_effect_dist_b);
            }
            else if(m_node_event_context)
            {
                init_effect = m_node_event_context->GetRng()->rand_beta(m_init_effect_dist_a, m_init_effect_dist_b);
            }
            else
            {
                release_assert(false);
            }
        }
        SetInitial(init_effect);

        if(m_enable_box_dist)
        {
            release_assert(m_box_duration_dist);

            if(m_individual_context)
            {
                m_box_duration = m_box_duration_dist->Calculate(m_individual_context->GetRng());
            }
            else if(m_node_event_context)
            {
                m_box_duration = m_box_duration_dist->Calculate(m_node_event_context->GetRng());
            }
            else
            {
                release_assert(false);
            }

            delete m_box_duration_dist;
            m_box_duration_dist = nullptr;
        }

        m_initialized = true;

        return;
    }

    void WaningEffect::SetInitial(float in_val)
    {
        m_init_effect = in_val;
        Update(0.0f);

        return;
    }

    float WaningEffect::GetInitial() const
    {
        return m_init_effect;
    }

    void WaningEffect::Update(float dt)
    {
        m_current_time += dt;

        m_current_effect = m_init_effect;
        if(m_current_time > m_box_duration)
        {
            #define PROMPT_DECAY (1.0e-6)
            if(m_decay_time > PROMPT_DECAY)
            {
                m_current_effect *= exp((m_box_duration-m_current_time)/m_decay_time);
            }
            else
            {
                m_current_effect = 0.0f;
            }
        }
        m_expired |= !m_current_effect;

        if(m_time_map.size())
        {
            // Undefined before map start; multiply by 1.0 as default
            m_current_effect *= m_time_map.getValueLinearInterpolation(m_current_time, 1.0f);
            m_expired |= (!m_current_effect && m_time_map.isAtEnd(m_current_time));
        }

        if(m_age_map.size() && m_individual_context)
        {
            float age_years = m_individual_context->GetEventContext()->GetAge()/DAYSPERYEAR;
            // Undefined before map start; multiply by 1.0 as default
            m_current_effect *= m_age_map.getValueLinearInterpolation(age_years, 1.0f);
            m_expired |= (!m_current_effect && m_time_map.isAtEnd(m_current_time) && m_age_map.isAtEnd(age_years));
        }

        return;
    }

    float WaningEffect::Current() const
    {
        // Logic error elsewhere if expired
        release_assert(!m_expired);

        return m_current_effect;
    }

    bool WaningEffect::Expired() const
    {
        return m_expired;
    }

    REGISTER_SERIALIZABLE(WaningEffect);

    void WaningEffect::serialize(IArchive& ar, WaningEffect* obj)
    {
        WaningEffect& effect = *obj;

        // Parameters
        ar.labelElement("m_age_map")              & effect.m_age_map;
        ar.labelElement("m_time_map")             & effect.m_time_map;
        ar.labelElement("m_box_duration")         & effect.m_box_duration;
        ar.labelElement("m_decay_time")           & effect.m_decay_time;
        ar.labelElement("m_enable_init_dist")     & effect.m_enable_init_dist;
        ar.labelElement("m_enable_box_dist")      & effect.m_enable_box_dist;
        ar.labelElement("m_init_effect")          & effect.m_init_effect;
        ar.labelElement("m_init_effect_const")    & effect.m_init_effect_const;
        ar.labelElement("m_init_effect_dist_a")   & effect.m_init_effect_dist_a;
        ar.labelElement("m_init_effect_dist_b")   & effect.m_init_effect_dist_b;

        // Variables
        ar.labelElement("m_current_effect")       & effect.m_current_effect;
        ar.labelElement("m_current_time")         & effect.m_current_time;
        ar.labelElement("m_expired")              & effect.m_expired;
        ar.labelElement("m_initialized")          & effect.m_initialized;
    }
}
