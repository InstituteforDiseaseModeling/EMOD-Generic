/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"

#ifdef ENABLE_PYTHON

#include "Debug.h"
#include "RANDOM.h"
#include "Environment.h"
#include "IndividualPy.h"
#include "SusceptibilityPy.h"
#include "InfectionPy.h"
#include "IContagionPopulation.h"
#include "PyInterventionsContainer.h"
#include "IdmString.h"
#include "PythonSupport.h"
#include "StrainIdentity.h"
#include "INodeContext.h"

#ifndef WIN32
#include <sys/time.h>
#endif

#pragma warning(disable: 4244)

SETUP_LOGGING( "IndividualPy" )

#define UNINIT_TIMER (-100.0f)


namespace Kernel
{
    BEGIN_QUERY_INTERFACE_DERIVED(IndividualHumanPy, IndividualHuman)
        HANDLE_INTERFACE(IIndividualHumanPy)
    END_QUERY_INTERFACE_DERIVED(IndividualHumanPy, IndividualHuman)

    IndividualHumanPy::IndividualHumanPy(suids::suid _suid, float monte_carlo_weight, float initial_age, int gender) 
        : IndividualHuman(_suid, monte_carlo_weight, initial_age, gender)
    {
#ifdef ENABLE_PYTHON_FEVER
        // Call into python script to notify of new individual 
        PyObject* pFunc = static_cast<PyObject*>(Kernel::PythonSupport::GetPyFunction( Kernel::PythonSupport::SCRIPT_PYTHON_FEVER, "create" ));
        if( pFunc )
        {
            // pass individual id
            PyObject* vars   = Py_BuildValue( "lffs", _suid.data, monte_carlo_weight, initial_age, (gender==0 ? "MALE" : "FEMALE") );
            PyObject* retVal = PyObject_CallObject( pFunc, vars );

            Py_XDECREF(vars);
            Py_XDECREF(retVal);
        }
#endif
    }

    IndividualHumanPy::~IndividualHumanPy()
    {
#ifdef ENABLE_PYTHON_FEVER
        // Call into python script to notify of new individual
        PyObject* pFunc = static_cast<PyObject*>(PythonSupport::GetPyFunction( PythonSupport::SCRIPT_PYTHON_FEVER, "destroy" ));
        if( pFunc )
        {
            PyObject* vars   = Py_BuildValue( "(l)", GetSuid().data );
            PyObject* retVal = PyObject_CallObject( pFunc, vars );

            Py_XDECREF(vars);
            Py_XDECREF(retVal);
        }
#endif
    }

    IndividualHumanPy *IndividualHumanPy::CreateHuman(INodeContext *context, suids::suid id, float monte_carlo_weight, float initial_age, int gender)
    {
        IndividualHumanPy *newhuman = _new_ IndividualHumanPy(id, monte_carlo_weight, initial_age, gender);
        
        newhuman->SetContextTo(context);
        LOG_DEBUG_F( "Created human with age=%f\n", newhuman->m_age );
        return newhuman;
    }

    void IndividualHumanPy::PropagateContextToDependents()
    {
        IndividualHuman::PropagateContextToDependents();
        pydemo_susceptibility = static_cast<SusceptibilityPy*>(susceptibility);
    }

    void IndividualHumanPy::setupInterventionsContainer()
    {
        interventions = _new_ PyInterventionsContainer();
    }

    void IndividualHumanPy::CreateSusceptibility(float imm_mod, float risk_mod)
    {
        SusceptibilityPy *newsusceptibility = SusceptibilityPy::CreateSusceptibility(this, imm_mod, risk_mod);
        pydemo_susceptibility = newsusceptibility;
        susceptibility = newsusceptibility;
    }

    void IndividualHumanPy::Expose( const IContagionPopulation* cp, float dt, TransmissionRoute::Enum tx_route )
    { 
        if( cp->GetTotalContagion() == 0 )
        {
            return;
        }

#ifdef ENABLE_PYTHON_FEVER
        LOG_DEBUG_F( "Calling py:expose with contagion pop %f\n", cp->GetTotalContagion() );

        PyObject* pFunc = static_cast<PyObject*>(PythonSupport::GetPyFunction( PythonSupport::SCRIPT_PYTHON_FEVER, "expose" ));
        if( pFunc )
        {
            // pass individual id AND dt
            PyObject* vars   = Py_BuildValue( "lfll", GetSuid().data, cp->GetTotalContagion(), static_cast<int>(dt), (tx_route==TransmissionRoute::ENVIRONMENTAL ? 0 : 1) );
            PyObject* retVal = PyObject_CallObject( pFunc, vars );

            if( retVal == nullptr )
            {
                PyErr_Print();
                std::stringstream msg;
                msg << "Embedded python code failed: PyObject_CallObject failed in call to 'expose'.";
                throw Kernel::IllegalOperationException( __FILE__, __LINE__, __FUNCTION__, msg.str().c_str() );
            }
            bool val = false;
            PyArg_Parse( retVal, "b", &val );
            if( val )
            {
                StrainIdentity strainId;
                AcquireNewInfection(&strainId, tx_route, -1.0f);
            }

            Py_XDECREF(vars);
            Py_XDECREF(retVal);
        }
#endif
        return;
    }

    void IndividualHumanPy::UpdateInfectiousness(float dt)
    {
#ifdef ENABLE_PYTHON_FEVER
        for( auto &route: parent->GetTransmissionRoutes() )
        {
            PyObject* pFunc = static_cast<PyObject*>(PythonSupport::GetPyFunction( PythonSupport::SCRIPT_PYTHON_FEVER, "update_and_return_infectiousness" ));
            if( pFunc )
            {
                // pass individual id ONLY
                PyObject* vars   = Py_BuildValue( "ls", GetSuid().data, TransmissionRoute::pairs::lookup_key(route) );
                PyObject* retVal = PyObject_CallObject( pFunc, vars );

                if( retVal == nullptr )
                {
                    PyErr_Print();
                    std::stringstream msg;
                    msg << "Embedded python code failed: PyObject_CallObject failed in call to 'update_and_return_infectiousness'.";
                    throw Kernel::IllegalOperationException( __FILE__, __LINE__, __FUNCTION__, msg.str().c_str() );
                }
                auto val = PyFloat_AsDouble(retVal);
                infectiousness += val;
                StrainIdentity tmp_strainID;
                release_assert( transmissionGroupMembershipByRoute.find( route ) != transmissionGroupMembershipByRoute.end() );
                if( val > 0 )
                {
                    LOG_DEBUG_F("Depositing %f to route %s: (clade=%d, substain=%d)\n", val, TransmissionRoute::pairs::lookup_key(route), tmp_strainID.GetCladeID(), tmp_strainID.GetGeneticID());
                    parent->DepositFromIndividual( tmp_strainID, (float) val, transmissionGroupMembershipByRoute.at( route ) );
                }

                Py_XDECREF(vars);
                Py_XDECREF(retVal);
            }
        }
#endif
        return;
    }

    Infection* IndividualHumanPy::createInfection( suids::suid _suid )
    {
        return InfectionPy::CreateInfection(this, _suid);
    }

    std::string IndividualHumanPy::processPrePatent( float dt )
    {
        return state_to_report;
    }

    void IndividualHumanPy::Update( float currenttime, float dt)
    {
#ifdef ENABLE_PYTHON_FEVER
        static PyObject* pFunc = static_cast<PyObject*>(PythonSupport::GetPyFunction( PythonSupport::SCRIPT_PYTHON_FEVER, "update" ));
        if( pFunc )
        {
            // pass individual id AND dt
            PyObject* vars   = Py_BuildValue( "ll", GetSuid().data, int(dt) );
            PyObject* retVal = PyObject_CallObject( pFunc, vars );

            if( retVal != nullptr )
            {
                char * state = "UNSET";
                PyArg_ParseTuple(retVal,"si",&state, &state_changed );
                state_to_report = state;
            }
            else
            {
                PyErr_Print();
                ostringstream msg;
                msg << "Failed to call python function 'update'. ";
                throw IllegalOperationException( __FILE__, __LINE__, __FUNCTION__, msg.str().c_str() );
                state_to_report = "D";
            }
            PyErr_Print();

            Py_DECREF(vars);
            Py_DECREF(retVal);
        }
        else
        {
            PyErr_Print();
            ostringstream msg;
            msg << "Failed to call python function 'update'. ";
            throw IllegalOperationException( __FILE__, __LINE__, __FUNCTION__, msg.str().c_str() );
        }
        LOG_DEBUG_F( "state_to_report for individual %d = %s; Infected = %d, change = %d.\n", GetSuid().data, state_to_report.c_str(), IsInfected(), state_changed );

        if( state_to_report == "S" && state_changed && GetInfections().size() > 0 )
        {
            LOG_DEBUG_F( "[Update] Somebody cleared their infection.\n" );
            // ClearInfection
            auto inf = GetInfections().front();
            IInfectionPy * inf_pydemo  = NULL;
            if (s_OK != inf->QueryInterface(GET_IID(IInfectionPy ), (void**)&inf_pydemo) )
            {
                throw QueryInterfaceException( __FILE__, __LINE__, __FUNCTION__, "inf", "IInfectionPy ", "Infection" );
            }
            // get InfectionPy pointer
            inf_pydemo->Clear();
        }
        else if( state_to_report == "D" && state_changed )
        {
            LOG_INFO_F( "[Update] Somebody died from their infection.\n" );
        }
#endif
        return IndividualHuman::Update( currenttime, dt);
    }

    void IndividualHumanPy::AcquireNewInfection( const IStrainIdentity *infstrain, TransmissionRoute::Enum tx_route, float incubation_period_override )
    {
        IndividualHuman::AcquireNewInfection( infstrain, tx_route, incubation_period_override );

#ifdef ENABLE_PYTHON_FEVER
        PyObject* pFunc = static_cast<PyObject*>(PythonSupport::GetPyFunction( PythonSupport::SCRIPT_PYTHON_FEVER, "acquire_infection" ));
        if( pFunc )
        {
            // pass individual id ONLY
            PyObject* vars   = Py_BuildValue( "(l)", GetSuid().data );
            PyObject* retVal = PyObject_CallObject( pFunc, vars );

            if( retVal == nullptr )
            {
                PyErr_Print();
                std::stringstream msg;
                msg << "Embedded python code failed: PyObject_CallObject failed in call to 'acquire_infection'.";
                throw Kernel::IllegalOperationException( __FILE__, __LINE__, __FUNCTION__, msg.str().c_str() );
            }

            Py_XDECREF(vars);
            Py_XDECREF(retVal);
        }
#endif
        return;
    }

    HumanStateChange IndividualHumanPy::GetStateChange() const
    {
        HumanStateChange retVal = StateChange;
        //auto parsed = IdmString(state_to_report).split();
        if( state_to_report == "D" )
        {
            LOG_INFO_F( "[GetStateChange] Somebody died from their infection.\n" );
            retVal = HumanStateChange::KilledByInfection;
        }
        return retVal;
    }
}

#endif // ENABLE_PYTHON
