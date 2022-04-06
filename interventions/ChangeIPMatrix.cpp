/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"
#include "ChangeIPMatrix.h"

#include "Exceptions.h"
#include "INodeContext.h"
#include "NodeEventContext.h"
#include "InterventionFactory.h"

SETUP_LOGGING( "ChangeIPMatrix" )

namespace Kernel
{
    BEGIN_QUERY_INTERFACE_BODY(ChangeIPMatrix)
        HANDLE_INTERFACE(IConfigurable)
        HANDLE_INTERFACE(IBaseIntervention)
        HANDLE_INTERFACE(INodeDistributableIntervention)
        HANDLE_ISUPPORTS_VIA(INodeDistributableIntervention)
    END_QUERY_INTERFACE_BODY(ChangeIPMatrix)

    IMPLEMENT_FACTORY_REGISTERED(ChangeIPMatrix)

    ChangeIPMatrix::ChangeIPMatrix()
        : target_property_name()
        , revised_matrix()
    {
        // Schema documentation
        initSimTypes( 11, "GENERIC_SIM", "VECTOR_SIM", "MALARIA_SIM", "AIRBORNE_SIM", "POLIO_SIM", "TBHIV_SIM", "STI_SIM", "HIV_SIM", "PY_SIM", "TYPHOID_SIM", "ENVIRONMENTAL_SIM" );
    }

    ChangeIPMatrix::~ChangeIPMatrix()
    { }

    bool ChangeIPMatrix::Configure(const Configuration * inputJson)
    {
        bool retval = false;

        initConfigTypeMap( "Property_Name", &target_property_name,  CIPM_Property_Name_DESC_TEXT,  "");
        initConfigTypeMap( "New_Matrix",    &revised_matrix,        CIPM_New_Matrix_DESC_TEXT);

        retval = JsonConfigurable::Configure( inputJson );

        for(const auto& row_value: revised_matrix)
        {
            for(const auto& coeff_value: row_value)
            {
                if( coeff_value < 0.0f )
                {
                    std::ostringstream msg;
                    msg << "Invalid New_Matrix for property '" << target_property_name << "'. Matrix must be non-negative." ;
                    throw GeneralConfigurationException( __FILE__, __LINE__, __FUNCTION__, msg.str().c_str() );
                }
            }
        }

        return retval;
    }

    bool ChangeIPMatrix::Distribute(INodeEventContext *context, IEventCoordinator2* pEC)
    {
        context->GetNodeContext()->ChangePropertyMatrix(target_property_name, revised_matrix);

        return true;
    }
    
    void ChangeIPMatrix::Update( float dt )
    {
        // Distribute doesn't call GiveIntervention for this intervention, so it isn't added to the NodeEventContext's list of NDI
    }
}