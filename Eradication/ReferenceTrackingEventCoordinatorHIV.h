/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "ReferenceTrackingEventCoordinator.h"

namespace Kernel
{
    class ReferenceTrackingEventCoordinatorHIV : public ReferenceTrackingEventCoordinator 
    {
        DECLARE_FACTORY_REGISTERED_EXPORT(EventCoordinatorFactory, ReferenceTrackingEventCoordinatorHIV, IEventCoordinator)    

    public:
        DECLARE_CONFIGURED(ReferenceTrackingEventCoordinatorHIV)
        IMPLEMENT_DEFAULT_REFERENCE_COUNTING()
        DECLARE_QUERY_INTERFACE()

        ReferenceTrackingEventCoordinatorHIV();
        virtual ~ReferenceTrackingEventCoordinatorHIV() { } 

        virtual bool qualifiesDemographically( const IIndividualHumanEventContext * pIndividual );

    protected:
        TargetDiseaseStateType::Enum target_disease_state;
    };
}
