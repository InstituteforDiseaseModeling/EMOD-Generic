/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include "IWaningEffect.h"
#include "IDistribution.h"
#include "InterpolatedValueMap.h"

namespace Kernel
{
    // --------------------------- WaningEffect ---------------------------
    class WaningEffect : public IWaningEffect, public WaningEffectFactory, public JsonConfigurable
    {
    friend class WaningEffectFactory;

    public:
        IMPLEMENT_NO_REFERENCE_COUNTING()
        virtual QueryResult QueryInterface(iid_t iid, void **ppvObject) { return e_NOINTERFACE; }

        virtual IWaningEffect* Clone()                               override;

        virtual bool  Configure(const Configuration*)                override;
        virtual void  Update(float)                                  override;
        virtual float Current()                                const override;
        virtual bool  Expired()                                const override;
        virtual void  SetContextTo(IIndividualHumanContext*)         override;
        virtual void  SetContextTo(INodeEventContext*)               override;
        virtual float GetInitial()                             const override;
        virtual void  SetInitial(float)                              override;

        virtual JsonConfigurable* GetConfigurable()                  override;

    protected:
        WaningEffect();
        WaningEffect(const WaningEffect&);
        virtual ~WaningEffect();

        void  Initialize();

        bool  m_enable_box_dist;
        bool  m_enable_init_dist;
        bool  m_expired;
        bool  m_initialized;

        float m_box_duration;
        float m_current_effect;
        float m_current_time;
        float m_decay_time;
        float m_init_effect;
        float m_init_effect_const;
        float m_init_effect_dist_a;
        float m_init_effect_dist_b;

        IIndividualHumanContext*  m_individual_context;
        INodeEventContext*        m_node_event_context;

        IDistribution*            m_box_duration_dist;

        InterpolatedValueMap      m_age_map;
        InterpolatedValueMap      m_time_map;

        DECLARE_SERIALIZABLE(WaningEffect);
    };
}
