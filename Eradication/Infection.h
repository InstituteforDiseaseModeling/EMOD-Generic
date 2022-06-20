/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once
#include <string>
#include <map>

#include "suids.hpp"

class Configuration;

#include "Sugar.h"
#include "SimulationEnums.h"
#include "IDistribution.h"
#include "IInfection.h"
#include "Configure.h"

namespace Kernel
{
    class StrainIdentity;
    class Susceptibility;
    struct IIndividualHumanContext;

    class InfectionConfig : public JsonConfigurable
    {
    public:
        virtual bool Configure( const Configuration* config ) override;

        static bool enable_disease_mortality;

    protected:
        friend class Infection;

        static IDistribution* infectious_distribution;
        static IDistribution* incubation_distribution;
        static IDistribution* infectivity_distribution;

        static float base_mortality;
        static MortalityTimeCourse::Enum                          mortality_time_course;                            // MORTALITY_TIME_COURSE

        GET_SCHEMA_STATIC_WRAPPER(InfectionConfig)
        IMPLEMENT_DEFAULT_REFERENCE_COUNTING()
        DECLARE_QUERY_INTERFACE()
    };

    // generic infection base class
    // may not necessary want to derive from this for real infections
    class Infection : public IInfection
    {
        IMPLEMENT_DEFAULT_REFERENCE_COUNTING()
        DECLARE_QUERY_INTERFACE()

    public:
        static Infection *CreateInfection(IIndividualHumanContext *context, suids::suid _suid);
        virtual ~Infection();

        virtual void SetContextTo(IIndividualHumanContext* context) override;
        IIndividualHumanContext* GetParent();

        virtual suids::suid GetSuid() const;

        virtual void SetParameters(IStrainIdentity* infstrain, float incubation_period_override, TransmissionRoute::Enum tx_route );
        virtual void Update(float, ISusceptibilityContext* = nullptr) override;

        virtual InfectionStateChange::_enum GetStateChange() const override;
        virtual float GetInfectiousness() const override;
        virtual float GetInfectiousnessByRoute(TransmissionRoute::Enum txRoute) const override;

        virtual TransmissionRoute::Enum GetSourceRoute() const override;

        virtual void InitInfectionImmunology(ISusceptibilityContext* _immunity) override;
        virtual void GetInfectiousStrainID(IStrainIdentity* infstrain); // the ID of the strain being shed
        virtual const IStrainIdentity* Infection::GetStrain() const override;
        virtual bool IsActive() const override;
        virtual NonNegativeFloat GetDuration() const override;
        virtual bool StrainMatches( IStrainIdentity * pStrain );

        virtual bool IsSymptomatic() const override;

    protected:
        IIndividualHumanContext *parent;

        suids::suid suid; // unique id of this infection within the system

        float duration;         // local timer
        float total_duration;
        float incubation_timer;
        float infectious_timer;
        float infectiousness;

        TransmissionRoute::Enum     m_source_route;
        InfectionStateChange::_enum StateChange;    //  Lets individual know something has happened

        StrainIdentity* infection_strain;           // this a pointer because disease modules may wish to implement derived types 

        Infection();
        Infection(IIndividualHumanContext *context);
        virtual void Initialize(suids::suid _suid);

        virtual void CreateInfectionStrain(IStrainIdentity* infstrain);
        virtual void EvolveStrain(ISusceptibilityContext* immunity, float dt);

        virtual bool  IsNewlySymptomatic() const override;
        virtual void  UpdateSymptomatic( float const duration, float const incubation_timer );
        virtual bool  DetermineSymptomatology( float const duration, float const incubation_timer );

    private:
        bool m_is_newly_symptomatic;
        bool m_is_symptomatic;

        DECLARE_SERIALIZABLE(Infection);
    };
}
