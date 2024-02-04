/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include <string>
#include <list>
#include <vector>

#include "Interventions.h"
#include "InterventionsContainer.h"
#include "HIVInterventionsContainer.h"

namespace Kernel
{
    // this container has the necessary other interventionscontainers

    class IMasterInterventionsContainer
        : public ISupports
    {
        virtual bool GiveIntervention( IDistributableIntervention * pIV ) = 0;
    };

    class MasterInterventionsContainer
        : public InterventionsContainer
        , public IMasterInterventionsContainer

    {
        IMPLEMENT_DEFAULT_REFERENCE_COUNTING()
            friend class IndividualHumanCoInfection;

    public:
        MasterInterventionsContainer();
        virtual ~MasterInterventionsContainer();

        // IIndividualHumanInterventionsContext
        virtual void SetContextTo(IIndividualHumanContext* context) override;
        virtual IIndividualHumanContext* GetParent() override;
        virtual std::list<IDistributableIntervention*> GetInterventionsByType(const std::string& type_name) override;
        virtual void PurgeExisting( const std::string& iv_name ) override;

        virtual IHIVInterventionsContainer* GetContainerHIV() override;
        virtual ITBInterventionsContainer*  GetContainerTB()  override;

        // ISupports
        virtual QueryResult QueryInterface(iid_t iid, void** pinstance) override;

        // IVaccineConsumer
        virtual void UpdateIVAcquireRate(  float acq,  IVRoute::Enum vax_route);
        virtual void UpdateIVTransmitRate( float xmit, IVRoute::Enum vax_route);
        virtual void UpdateIVMortalityRate(float mort, IVRoute::Enum vax_route);

        virtual float GetInterventionReducedAcquire(TransmissionRoute::Enum)  const;
        virtual float GetInterventionReducedTransmit(TransmissionRoute::Enum)  const;
        virtual float GetInterventionReducedMortality(TransmissionRoute::Enum)  const;
 
        virtual bool GiveIntervention( IDistributableIntervention * pIV ) override;

        virtual void InfectiousLoopUpdate( float dt ) override; // update only interventions that need updating in Infectious Update loop
        virtual void Update( float dt ) override;               // update non-infectious loop interventions once per time step

    private:
        void InitInterventionContainers();
        std::list <InterventionsContainer* > InterventionsContainerList;

        DECLARE_SERIALIZABLE(MasterInterventionsContainer);
    };
}
