/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include <list>

#include "Interventions.h"

namespace Kernel
{
    struct IVaccineConsumer
        : public ISupports
    {
        virtual void UpdateIVAcquireRate(  float acq,  IVRoute::Enum vax_route) = 0;
        virtual void UpdateIVTransmitRate( float xmit, IVRoute::Enum vax_route) = 0;
        virtual void UpdateIVMortalityRate(float mort, IVRoute::Enum vax_route) = 0;

        virtual float GetInterventionReducedAcquire(TransmissionRoute::Enum)   const = 0;
        virtual float GetInterventionReducedTransmit(TransmissionRoute::Enum)  const = 0;
        virtual float GetInterventionReducedMortality(TransmissionRoute::Enum) const = 0;

        virtual float GetIVReducedAcquire(IVRoute::Enum)   const = 0;
        virtual float GetIVReducedTransmit(IVRoute::Enum)  const = 0;
        virtual float GetIVReducedMortality(IVRoute::Enum) const = 0;
    };

    class InterventionsContainer
        : public IIndividualHumanInterventionsContext
        , public IVaccineConsumer
        , public IInterventionConsumer
    {
        IMPLEMENT_DEFAULT_REFERENCE_COUNTING()

    public:
        InterventionsContainer();
        virtual ~InterventionsContainer();

        // IIndividualHumanInterventionsContext
        virtual void SetContextTo(IIndividualHumanContext* context) override;
        virtual IIndividualHumanContext* GetParent() override;
        virtual std::list<IDistributableIntervention*> GetInterventionsByType(const std::string& type_name) override;
        virtual std::list<IDistributableIntervention*> GetInterventionsByName(const std::string& intervention_name) override;
        virtual void PurgeExisting( const std::string& iv_name ) override;
        virtual void PurgeExistingByName( const std::string& iv_name ) override;
        virtual bool ContainsExisting( const std::string& iv_name ) override;
        virtual bool ContainsExistingByName( const std::string &iv_name ) override;

        virtual std::list<IDrug*> GetDrugInterventions() override;

        virtual void ChangeProperty( const char *property, const char* new_value ) override;

        virtual IInterventionConsumer*        GetInterventionConsumer()  override;
        virtual ISTIInterventionsContainer*   GetContainerSTI()          override { return nullptr; }
        virtual IHIVInterventionsContainer*   GetContainerHIV()          override { return nullptr; }

        // IUnknown
        virtual QueryResult QueryInterface(iid_t iid, void** pinstance) override;

        // IVaccineConsumer
        virtual void UpdateIVAcquireRate(  float acq,  IVRoute::Enum vax_route) override;
        virtual void UpdateIVTransmitRate( float xmit, IVRoute::Enum vax_route) override;
        virtual void UpdateIVMortalityRate(float mort, IVRoute::Enum vax_route) override;

        virtual float GetInterventionReducedAcquire(TransmissionRoute::Enum)   const override;
        virtual float GetInterventionReducedTransmit(TransmissionRoute::Enum)  const override;
        virtual float GetInterventionReducedMortality(TransmissionRoute::Enum) const override;

        virtual float GetIVReducedAcquire(IVRoute::Enum)   const override;
        virtual float GetIVReducedTransmit(IVRoute::Enum)  const override;
        virtual float GetIVReducedMortality(IVRoute::Enum) const override;

        virtual bool GiveIntervention( IDistributableIntervention * pIV ) override;

        virtual void PreInfectivityUpdate( float dt );  // update only interventions that need updating before accumulating infectivity
        virtual void InfectiousLoopUpdate( float dt );  // update only interventions that need updating in Infectious Update loop
        virtual void Update( float dt );                // update non-infectious loop update interventions once per time step

    protected:
        std::vector<float> reduced_acquire;
        std::vector<float> reduced_transmit;
        std::vector<float> reduced_mortality;

        std::list<IDistributableIntervention*> interventions;

        virtual void PropagateContextToDependents(); // pass context to interventions if they need it

        IIndividualHumanContext* parent;    // context for this interventions container

    private:
        DECLARE_SERIALIZABLE(InterventionsContainer);
    };
}
