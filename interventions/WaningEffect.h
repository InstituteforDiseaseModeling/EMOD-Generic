/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include <string>
#include <map>

#include "IWaningEffect.h"
#include "ISerializable.h"
#include "Configuration.h"
#include "Configure.h"
#include "FactorySupport.h"
#include "InterpolatedValueMap.h"

namespace Kernel
{
    // --------------------------- WaningEffectNull ---------------------------
    class IDMAPI WaningEffectNull : public IWaningEffect, public JsonConfigurable
    {
        DECLARE_FACTORY_REGISTERED_EXPORT(WaningEffectFactory, WaningEffectNull, IWaningEffect)

    public:
        IMPLEMENT_NO_REFERENCE_COUNTING()
        DECLARE_QUERY_INTERFACE()

        WaningEffectNull();
        WaningEffectNull(const WaningEffectNull& rOrig);
        virtual ~WaningEffectNull() {};

        virtual IWaningEffect* Clone()                override;

        virtual bool  Configure(const Configuration*) override {return true;};
        virtual void  Update(float dt)                override { };
        virtual void  SetCurrentTime(float)           override { };
        virtual void  SetInitial(float)               override { };
        virtual float Current()                 const override {return 0.0f;};
        virtual bool  Expired()                 const override {return true;};
        virtual void  SetContextTo(IIndividualHumanContext*) override {};
        virtual IWaningEffectCount* GetEffectCount()  override { return nullptr; };

        DECLARE_SERIALIZABLE(WaningEffectNull);
    };

    // --------------------------- WaningEffectConstant ---------------------------
    class IDMAPI WaningEffectConstant : public IWaningEffect, public JsonConfigurable
    {
        DECLARE_FACTORY_REGISTERED_EXPORT(WaningEffectFactory, WaningEffectConstant, IWaningEffect)

    public:
        DECLARE_CONFIGURED(WaningEffectConstant)
        IMPLEMENT_NO_REFERENCE_COUNTING()
        DECLARE_QUERY_INTERFACE()

        WaningEffectConstant();
        WaningEffectConstant( const WaningEffectConstant& rOrig );
        virtual ~WaningEffectConstant() {};

        virtual IWaningEffect* Clone() override;
        virtual void  Update(float dt) override;
        virtual void  SetCurrentTime(float dt) override {};
        virtual float Current() const override;
        virtual bool  Expired() const override { return false; };
        virtual void SetContextTo( IIndividualHumanContext *context ) override {};
        virtual void  SetInitial(float newVal) override;
        virtual IWaningEffectCount* GetEffectCount()  override { return nullptr; };

    protected:
        float currentEffect;
        bool usingDefault;
        const int notSetByUser = -1;

        DECLARE_SERIALIZABLE(WaningEffectConstant);
    };

    // --------------------------- WaningEffectExponential ---------------------------
    class IDMAPI WaningEffectExponential : public WaningEffectConstant
    {
        DECLARE_FACTORY_REGISTERED_EXPORT(WaningEffectFactory, WaningEffectExponential, IWaningEffect)

    public:
        DECLARE_QUERY_INTERFACE()

        WaningEffectExponential();
        WaningEffectExponential( const WaningEffectExponential& rOrig );
        virtual ~WaningEffectExponential() {};

        virtual bool Configure( const Configuration *config ) override;
        virtual IWaningEffect* Clone() override;
        virtual void  Update(float dt) override;

    protected:
        float decayTimeConstant;

        DECLARE_SERIALIZABLE(WaningEffectExponential);
    };

    // --------------------------- WaningEffectBox ---------------------------
    class IDMAPI WaningEffectBox : public WaningEffectConstant
    {
        DECLARE_FACTORY_REGISTERED_EXPORT(WaningEffectFactory, WaningEffectBox, IWaningEffect)

    public:
        DECLARE_QUERY_INTERFACE()

        WaningEffectBox();
        WaningEffectBox( const WaningEffectBox& rOrig );
        virtual ~WaningEffectBox() {};

        virtual bool Configure( const Configuration *config ) override;
        virtual IWaningEffect* Clone() override;
        virtual void  Update(float dt) override;
        virtual bool  Expired() const override;

    protected:
        float boxDuration;

        DECLARE_SERIALIZABLE(WaningEffectBox);
    };

    // --------------------------- WaningEffectBoxExponential ---------------------------
    class IDMAPI WaningEffectBoxExponential : public WaningEffectConstant
    {
        DECLARE_FACTORY_REGISTERED_EXPORT(WaningEffectFactory, WaningEffectBoxExponential, IWaningEffect)

    public:
        DECLARE_QUERY_INTERFACE()

        WaningEffectBoxExponential();
        WaningEffectBoxExponential( const WaningEffectBoxExponential& rOrig );
        virtual ~WaningEffectBoxExponential() {};

        virtual bool Configure( const Configuration *config ) override;
        virtual IWaningEffect* Clone() override;
        virtual void  Update(float dt) override;

    protected:
        float boxDuration;
        float decayTimeConstant;

        DECLARE_SERIALIZABLE(WaningEffectBoxExponential);
    };

    // --------------------------- WaningEffectRandomBox ---------------------------
    class WaningEffectRandomBox : public WaningEffectConstant
    {
        DECLARE_FACTORY_REGISTERED_EXPORT(WaningEffectFactory, WaningEffectRandomBox, IWaningEffect)

    public:
        //DECLARE_CONFIGURED(WaningEffectRandomBox)
        DECLARE_QUERY_INTERFACE()

        WaningEffectRandomBox();
        WaningEffectRandomBox( const WaningEffectRandomBox& master );
        virtual ~WaningEffectRandomBox();
        virtual bool Configure( const Configuration *config ) override;
        virtual IWaningEffect* Clone() override;
        virtual void  SetContextTo( IIndividualHumanContext *context ) override;
        virtual void  Update(float dt) override;
        virtual bool  Expired() const override;

    protected:
        float m_ExpectedDiscardTime = 0;
        float m_DiscardCounter;

        DECLARE_SERIALIZABLE(WaningEffectRandomBox);
    };

    // --------------------------- WaningEffectMapAbstract ---------------------------
    class IDMAPI WaningEffectMapAbstract : public WaningEffectConstant
    {
    public:
        DECLARE_QUERY_INTERFACE()

        virtual ~WaningEffectMapAbstract();
        virtual bool Configure( const Configuration *config ) override;
        virtual void  Update(float dt) override;
        virtual void  SetCurrentTime(float dt) override; // 'fast-forward'
        virtual bool  Expired() const override;
        virtual void  SetInitial(float newVal) override;

        virtual float GetMultiplier( float timeSinceStart ) const = 0;

    protected:
        WaningEffectMapAbstract( float maxTime = 999999.0f );
        WaningEffectMapAbstract( float minTime, float maxTime );
        WaningEffectMapAbstract( const WaningEffectMapAbstract& rOrig );

        virtual void UpdateEffect();
        virtual bool ConfigureExpiration( const Configuration* config );
        virtual bool ConfigureReferenceTimer( const Configuration* config );

        bool  m_Expired;
        float m_EffectOriginal;
        bool  m_ExpireAtDurationMapEnd;
        NonNegativeFloat m_TimeSinceStart;
        int   m_RefTime;
        InterpolatedValueMap m_DurationMap;

        static void serialize( IArchive&, WaningEffectMapAbstract*);
    };

    // --------------------------- WaningEffectMapLinear ---------------------------
    class IDMAPI WaningEffectMapLinear : public WaningEffectMapAbstract
    {
        DECLARE_FACTORY_REGISTERED_EXPORT(WaningEffectFactory, WaningEffectMapLinear, IWaningEffect)
    public:
        DECLARE_QUERY_INTERFACE()

        WaningEffectMapLinear( float maxTime = 999999.0f );
        WaningEffectMapLinear( const WaningEffectMapLinear& rOrig );
        virtual ~WaningEffectMapLinear();
        virtual IWaningEffect* Clone() override;

        virtual float GetMultiplier( float timeSinceStart ) const override;

    protected:
        DECLARE_SERIALIZABLE(WaningEffectMapLinear);
    };

    // --------------------------- WaningEffectMapPiecewise ---------------------------
    class IDMAPI WaningEffectMapPiecewise : public WaningEffectMapAbstract
    {
        DECLARE_FACTORY_REGISTERED_EXPORT(WaningEffectFactory, WaningEffectMapPiecewise, IWaningEffect)
    public:
        DECLARE_QUERY_INTERFACE()

        WaningEffectMapPiecewise();
        WaningEffectMapPiecewise( float minTime, float maxTime );
        WaningEffectMapPiecewise( const WaningEffectMapPiecewise& rOrig );
        virtual ~WaningEffectMapPiecewise();
        virtual IWaningEffect* Clone() override;

        virtual float GetMultiplier( float timeSinceStart ) const override;

    protected:
        DECLARE_SERIALIZABLE(WaningEffectMapPiecewise);
    };

    // --------------------------- WaningEffectMapLinearAge ---------------------------
    class IDMAPI WaningEffectMapLinearAge : public WaningEffectMapLinear
    {
        DECLARE_FACTORY_REGISTERED_EXPORT( WaningEffectFactory, WaningEffectMapLinearAge, IWaningEffect )
        DECLARE_QUERY_INTERFACE()
    public:
        WaningEffectMapLinearAge();
        WaningEffectMapLinearAge( const WaningEffectMapLinearAge& rOrig );
        virtual ~WaningEffectMapLinearAge();
        virtual IWaningEffect* Clone() override;

        virtual void SetContextTo( IIndividualHumanContext *context ) override;
        virtual float GetMultiplier( float timeSinceStart ) const override;

    protected:
        virtual bool ConfigureExpiration( const Configuration* config ) override;
        virtual bool ConfigureReferenceTimer( const Configuration* config ) override;

        IIndividualHumanContext* m_Parent;

        DECLARE_SERIALIZABLE( WaningEffectMapLinearAge );
    };

    // --------------------------- WaningEffectMapLinearSeasonal ---------------------------
    class IDMAPI WaningEffectMapLinearSeasonal : public WaningEffectMapLinear
    {
        DECLARE_FACTORY_REGISTERED_EXPORT( WaningEffectFactory, WaningEffectMapLinearSeasonal, IWaningEffect )
        DECLARE_QUERY_INTERFACE()
    public:
        WaningEffectMapLinearSeasonal();
        WaningEffectMapLinearSeasonal( const WaningEffectMapLinearSeasonal& rOrig );
        virtual ~WaningEffectMapLinearSeasonal();
        virtual IWaningEffect* Clone() override;

        virtual void  Update( float dt ) override;
        virtual void SetContextTo( IIndividualHumanContext *context ) override;

    protected:
        virtual bool ConfigureExpiration( const Configuration* config ) override;
        virtual bool ConfigureReferenceTimer( const Configuration* config ) override;

        IIndividualHumanContext* m_Parent;

        DECLARE_SERIALIZABLE( WaningEffectMapLinearSeasonal );
    };

    // --------------------------- WaningEffectMapCount ---------------------------
    class WaningEffectMapCount : public WaningEffectMapPiecewise, public IWaningEffectCount
    {
        DECLARE_FACTORY_REGISTERED_EXPORT( WaningEffectFactory, WaningEffectMapCount, IWaningEffect )
        DECLARE_QUERY_INTERFACE()
    public:
        WaningEffectMapCount();
        WaningEffectMapCount( const WaningEffectMapCount& rOrig );
        virtual ~WaningEffectMapCount();


        // WaningEffectMapAbstract methods
        virtual IWaningEffect* Clone() override;
        virtual void  Update( float dt ) override;
        virtual float Current() const override;
        virtual IWaningEffectCount* GetEffectCount() override;

        //IWaningEffectCount methods
        virtual int32_t AddRef( void ) { return WaningEffectMapPiecewise::AddRef(); }
        virtual int32_t Release( void ) { return WaningEffectMapPiecewise::Release(); }
        virtual void SetCount( uint32_t numCounts ) override;
        virtual bool IsValidConfiguration( uint32_t maxCount ) const override;

    protected:
        virtual bool ConfigureExpiration( const Configuration* config );
        virtual bool ConfigureReferenceTimer( const Configuration* config );

        bool m_SetCountCalled;

        DECLARE_SERIALIZABLE( WaningEffectMapCount );
    };

    // --------------------------- WaningEffectCollection ---------------------------
    class WaningEffectCollection : public JsonConfigurable, public IComplexJsonConfigurable
    {
    public:
        IMPLEMENT_NO_REFERENCE_COUNTING()
        DECLARE_QUERY_INTERFACE()

        WaningEffectCollection();
        virtual ~WaningEffectCollection();

        // IComplexJsonConfigurable methods
        virtual bool  HasValidDefault() const override { return false; }
        virtual json::QuickBuilder GetSchema() override;
        virtual void ConfigureFromJsonAndKey( const Configuration* inputJson, const std::string& key ) override;

        virtual void CheckConfiguration();
        virtual void Add( IWaningEffect* pwe );
        int Size() const;
        IWaningEffect* operator[]( int index ) const;

        static void serialize( IArchive& ar, WaningEffectCollection& map );

    protected:
        std::vector<IWaningEffect*> m_Collection;
    };

    // --------------------------- WaningEffectCombo ---------------------------
    class WaningEffectCombo : public JsonConfigurable, public IWaningEffect, public IWaningEffectCount
    {
        DECLARE_FACTORY_REGISTERED_EXPORT( WaningEffectFactory, WaningEffectCombo, IWaningEffect )

    public:
        IMPLEMENT_NO_REFERENCE_COUNTING()
        DECLARE_QUERY_INTERFACE()

        WaningEffectCombo();
        virtual ~WaningEffectCombo();

        virtual bool Configure( const Configuration *config ) override;

        // IWaningEffect methods
        virtual IWaningEffect* Clone() override;
        virtual void  Update( float dt ) override;
        virtual float Current() const override;
        virtual bool  Expired() const override;
        virtual void  SetContextTo( IIndividualHumanContext *context ) override;
        virtual void  SetInitial( float newVal ) override;
        virtual void  SetCurrentTime( float dt ) override;
        virtual IWaningEffectCount* GetEffectCount() override;

        // IWaningEffectCount methods
        virtual void SetCount( uint32_t numCounts ) override;
        virtual bool IsValidConfiguration( uint32_t maxCount ) const override;

    protected:
        WaningEffectCombo( WaningEffectCombo& rOrig );

        bool m_IsAdditive;
        bool m_IsExpiringWhenAllExpire;
        WaningEffectCollection m_EffectCollection;

        DECLARE_SERIALIZABLE( WaningEffectCombo );
    };

}
