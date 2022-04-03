#include <Python.h>
#include <iostream>

#include "MalariaChallenge.h"
#include "NodeMalariaEventContext.h"

using namespace Kernel;

Kernel::MalariaChallenge _instance;

Configuration * configStubJson = nullptr;

static PyObject *my_callback = NULL;

static PyObject *
my_set_callback(PyObject *dummy, PyObject *args)
{
    PyObject *result = NULL;
    PyObject *temp;

    if (PyArg_ParseTuple(args, "O:set_callback", &temp))
    {
        if (!PyCallable_Check(temp)) {
            PyErr_SetString(PyExc_TypeError, "parameter must be callable");
            return NULL;
        } 
        Py_XINCREF(temp);         /* Add a reference to new callback */ 
        Py_XDECREF(my_callback);  /* Dispose of previous callback */ 
        my_callback = temp;       /* Remember new callback */
        /* Boilerplate to return "None" */
        Py_INCREF(Py_None);
        result = Py_None;
    }
    return result;
}

static void initMC( bool dr = false )
{
    if( dr == true )
    {
        Kernel::JsonConfigurable::_dryrun = dr;
        _instance.Configure( nullptr  );
        Kernel::JsonConfigurable::_dryrun = !false;
    }
    if( configStubJson == nullptr )
    {
        configStubJson = Configuration::Load("malariachallenge.json");
        std::cout << "Configuring Mc intervention from json." << std::endl;
        _instance.Configure( configStubJson  );
    }
}

class StubMalariaNode : public INodeEventContext, public ISporozoiteChallengeConsumer
{
    public:
        virtual int32_t AddRef() {}
        virtual int32_t Release() {}

        Kernel::QueryResult QueryInterface( iid_t iid, void **ppinstance )
        {
            assert(ppinstance);

            if ( !ppinstance )
                return e_NULL_POINTER;

            ISupports* foundInterface;

            if ( iid == GET_IID(ISporozoiteChallengeConsumer)) 
                foundInterface = static_cast<ISporozoiteChallengeConsumer*>(this);
            // -->> add support for other I*Consumer interfaces here <<--      
            else if ( iid == GET_IID(ISupports)) 
                foundInterface = static_cast<ISupports*>(static_cast<ISporozoiteChallengeConsumer*>(this));
            else
                foundInterface = 0;

            QueryResult status;
            if ( !foundInterface )
                status = e_NOINTERFACE;
            else
            {
                //foundInterface->AddRef();           // not implementing this yet!
                status = s_OK;
            }

            *ppinstance = foundInterface;
            return status;
        }

        // This is so we can pass a faux-node
        virtual void VisitIndividuals(individual_visit_function_t func) {}
        virtual int VisitIndividuals(IVisitIndividual* pIndividualVisitImpl ) {} 
        virtual const NodeDemographics& GetDemographics() {}
        virtual bool GetUrban() const {}
        virtual const IdmDateTime& GetTime() const {}
        virtual void UpdateInterventions(float = 0.0f) {} 
        virtual void RegisterTravelDistributionSource(ITravelLinkedDistributionSource *tles, TravelEventType type) {}
        virtual void UnregisterTravelDistributionSource(ITravelLinkedDistributionSource *tles, TravelEventType type) {} 
        virtual const suids::suid & GetId() const {}
        virtual void SetContextTo(INodeContext* context) {}
        virtual std::list<INodeDistributableIntervention*> GetInterventionsByType(const std::string& type_name) {}
        virtual void PurgeExisting( const std::string& iv_name ) {} 
        virtual bool IsInPolygon(float* vertex_coords, int numcoords) {}
        virtual bool IsInPolygon( const json::Array &poly ) {}
        virtual bool IsInExternalIdSet( const std::list<ExternalNodeId_t>& nodelist ) {}
        virtual RANDOMBASE* GetRng() {}
        virtual INodeContext* GetNodeContext() {} 
        virtual int GetIndividualHumanCount() const {}
        virtual ExternalNodeId_t GetExternalId() const {}

        virtual void ChallengeWithInfectiousBites( int n_challenged_objects, float coverage, tAgeBitingFunction=nullptr ) override
        {
            assert( my_callback );
            PyObject *arglist = Py_BuildValue("(s, i, f)", "Bites", n_challenged_objects, coverage );
            PyObject_CallObject(my_callback, arglist);
            Py_DECREF(arglist);
        }
        virtual void ChallengeWithSporozoites( int n_challenged_objects, float coverage, tAgeBitingFunction=nullptr ) override
        {
            assert( my_callback );
            PyObject *arglist = Py_BuildValue("(s, i, f)", "Sporozoites", n_challenged_objects, coverage );
            PyObject_CallObject(my_callback, arglist);
            Py_DECREF(arglist);
        }
};

static PyObject*
distribute(PyObject* self, PyObject* args)
{
    bool ret = false;

    initMC();
    StubMalariaNode node;
    ret = _instance.Distribute( &node, nullptr );
    return Py_BuildValue( "b", ret );;
}

static PyObject*
getSchema(PyObject* self, PyObject* args)
{
    bool ret = false;

    initMC( true );
    auto schema = _instance.GetSchema();
    std::ostringstream schema_ostream;
    json::Writer::Write( schema, schema_ostream );
    return Py_BuildValue("s", schema_ostream.str().c_str() );;
}

static PyMethodDef DtkMalariaChallengeMethods[] =
{
     {"distribute", distribute, METH_VARARGS, "Update."},
     {"my_set_callback", my_set_callback, METH_VARARGS, "Update."},
     {"get_schema", getSchema, METH_VARARGS, "Update."},
     {NULL, NULL, 0, NULL}
};

PyMODINIT_FUNC
initdtk_malariachallenge(void)
{
     (void) Py_InitModule("dtk_malariachallenge", DtkMalariaChallengeMethods);
}
