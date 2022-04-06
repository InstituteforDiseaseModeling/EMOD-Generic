/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include <Python.h>
#include <iostream>
#include "RANDOM.h"

#include "ConfigParams.h"
#include "Environment.h"
#include "Exceptions.h"
#include "NodeDemographics.h"
#include "Node.h"
#include "Individual.h"
#include "INodeContext.h"
#include "Properties.h"
#include "ISimulationContext.h"
#include "ISimulation.h"
#include "IdmDateTime.h"

Kernel::IndividualHuman * person = nullptr;
Configuration * configStubJson = nullptr;
static PyObject *create_person_callback = NULL;
static PyObject *conceive_baby_callback = NULL;
static PyObject *update_preg_callback = NULL;
Kernel::NodeDemographicsFactory* demographics_factory = nullptr;
static std::map< std::string, float > userParams;
static std::map< std::string, std::string > userParamsEnum;
static std::vector< Kernel::Node* > nodes;

using namespace Kernel;

void
pyNodeDemogInit()
{
}

// 
// We'll need a stub node object as the individual's parent. Most functions are empty.
//


// Boiler-plate callback setting function that will be put in common file.
PyObject*
my_set_callback(PyObject *dummy, PyObject *args)
{
    PyObject *temp;

    if (PyArg_ParseTuple(args, "O:set_callback", &temp))
    {
        if (!PyCallable_Check(temp)) {
            PyErr_SetString(PyExc_TypeError, "parameter must be callable");
            return NULL;
        } 
        Py_XINCREF(temp);         /* Add a reference to new callback */ 
        Py_XDECREF(create_person_callback);  /* Dispose of previous callback */ 
        create_person_callback = temp;       /* Remember new callback */
    }

    Py_RETURN_NONE;
}

PyObject*
set_conceive_baby_callback(PyObject *dummy, PyObject *args)
{
    PyObject *temp;

    if (PyArg_ParseTuple(args, "O:set_callback", &temp))
    {
        if (!PyCallable_Check(temp)) {
            PyErr_SetString(PyExc_TypeError, "parameter must be callable");
            return NULL;
        } 
        Py_XINCREF(temp);         /* Add a reference to new callback */ 
        Py_XDECREF(conceive_baby_callback);  /* Dispose of previous callback */ 
        conceive_baby_callback = temp;       /* Remember new callback */
    }

    Py_RETURN_NONE;
}

PyObject*
set_update_preg_callback(PyObject *dummy, PyObject *args)
{
    PyObject *temp;

    if (PyArg_ParseTuple(args, "O:set_callback", &temp))
    {
        if (!PyCallable_Check(temp)) {
            PyErr_SetString(PyExc_TypeError, "parameter must be callable");
            return NULL;
        } 
        Py_XINCREF(temp);         /* Add a reference to new callback */ 
        Py_XDECREF(update_preg_callback);  /* Dispose of previous callback */ 
        update_preg_callback = temp;       /* Remember new callback */
    }

    Py_RETURN_NONE;
}
 

// Simulation class is a friend which is necessary for calling Configure
namespace Test {
    class TestSimulation : public Kernel::ISimulationContext, public Kernel::ISimulation
    {
        public:
            //static std::string result;
            TestSimulation()
                : time(1.0f)
                , fake_reports()
            {
                Kernel::JsonConfigurable::_dryrun = true;
                Kernel::JsonConfigurable::_track_missing = false;
                Kernel::IndividualHumanConfig adam;
                //adam.Configure( nullptr ); // protected
                auto schema = adam.GetSchema();
                std::ostringstream schema_ostream;
                json::Writer::Write( schema, schema_ostream );
                //std::cout << schema_ostream.str() << std::endl;
                //result = schema_ostream.str();
                Kernel::JsonConfigurable::_dryrun = false;
            }

            QueryResult QueryInterface( iid_t iid, void** ppinstance )
            {
                release_assert( ppinstance );

                ISupports* foundInterface;
                if( iid == GET_IID( ISimulationContext ) )
                    foundInterface = static_cast<ISimulationContext*>(this);
                else if( iid == GET_IID( ISimulation ) )
                    foundInterface = static_cast<ISimulation*>(this);
                else if( iid == GET_IID( ISupports ) )
                    foundInterface = static_cast<ISupports*>(static_cast<ISimulationContext*>(this));
                else
                    foundInterface = nullptr;

                QueryResult status = e_NOINTERFACE;
                if( foundInterface )
                {
                    foundInterface->AddRef();
                    status = s_OK;
                }

                *ppinstance = foundInterface;
                return status;
            }

            static void SeedRNG(int rng_seed)
            {
                if(!m_pRNG)
                {
                    m_pRNG = new PSEUDO_DES(rng_seed);
                }
            }

            virtual int32_t AddRef() override { return 0; }
            virtual int32_t Release() override { return 0; }

            virtual float GetNodeInboundMultiplier( const suids::suid& rNodeSuid ) override
            {
                throw Kernel::NotYetImplementedException( __FILE__, __LINE__, __FUNCTION__, "The method or operation is not implemented.");
            }

            virtual const IInterventionFactory* GetInterventionFactory() const override { return nullptr; }
            virtual const DemographicsContext* GetDemographicsContext() const override { return demographics_factory->CreateDemographicsContext(); }
            virtual const SimParams* GetParams() const override { return nullptr; }

            // time services
            virtual const IdmDateTime& GetSimulationTime() const override
            {
                return time;
            }

            void setBaseYear( float baseYear )
            {
                time.setBaseYear( baseYear ); // hmm, GENERIC_SIM doesn't use base year

                time = IdmDateTime(); // easiest way to set time back to 0 without adding nasty test-only API method
            }

            virtual void Update()
            {
                // TBD: Parse dt
                float dt = 1.0;
                time.Update(dt);
            }

            // id services
            virtual suids::suid      GetNextInfectionSuid() override { return suids::nil_suid(); }
            virtual suids::suid      GetNodeSuid( ExternalNodeId_t external_node_id ) override { return suids::nil_suid(); }
            virtual ExternalNodeId_t GetNodeExternalID( const suids::suid& rNodeSuid ) override { return ExternalNodeId_t(0); }
            virtual uint32_t         GetNodeRank( const suids::suid& rNodeSuid ) override { return ExternalNodeId_t (0); }


            // migration
            virtual void PostMigratingIndividualHuman(IIndividualHuman *i) override {}
            virtual bool CanSupportFamilyTrips() const override { return false; }

            // events
            virtual void DistributeEventToOtherNodes( const EventTrigger::Enum& rEventTrigger, INodeQualifier* pQualifier ) override {} 
            virtual void UpdateNodeEvents() override {}
            virtual ISimulationEventContext* GetSimulationEventContext() override { return nullptr; }

            // reporting
            virtual std::vector<IReport*>& GetReports() override { return fake_reports; }
            virtual std::vector<IReport*>& GetReportsNeedingIndividualData() override { return fake_reports; }

            std::vector<IReport*> fake_reports; 
            // ISimulation
            virtual RANDOMBASE* GetRng() override
            {
                return m_pRNG;
            }

            virtual void Initialize( const ::Configuration *config ) override{};
            virtual bool Populate() override { return false; };

            virtual int  GetSimulationTimestep() const override { return 1; };
            virtual bool TimeToStop() override { return false; };

            virtual void RegisterNewNodeObserver( void* id, Kernel::ISimulation::callback_t observer ) override {};
            virtual void UnregisterNewNodeObserver( void* id ) override {};

            virtual void WriteReportsData() override {};

            void SetBaseYear( float baseYear )
            {
                time.setBaseYear( baseYear );
            }
            
    private:
            IdmDateTime time;
            static RANDOMBASE* m_pRNG;
    };

    class TestNode : public Kernel::Node
    {
        public:
            TestNode( Kernel::ISimulationContext * parent_sim,  ExternalNodeId_t externalNodeId, Kernel::suids::suid _suid )
            : Kernel::Node( parent_sim, externalNodeId, _suid )
            {
                //SetContextTo( parent_sim ); // optional?
                Initialize();
            }

            void getSchema()
            {
                Kernel::JsonConfigurable::_dryrun = true;

                std::ostringstream schema_ostream;
                for (auto& entry : Kernel::JsonConfigurable::get_registration_map())
                {
                    const std::string& classname = entry.first;
                    if( classname == "Node" )
                    {
                        json::QuickBuilder config_schema = ((*(entry.second))());
                        json::Writer::Write( config_schema, schema_ostream );
                    }
                }

                //auto schema = configStubJson->GetSchema();
                //std::cout << schema_ostream.str() << std::endl;
                result = schema_ostream.str();
                Kernel::JsonConfigurable::_dryrun = false;
            }

            virtual IIndividualHuman* addNewIndividual(
                float monte_carlo_weight = 1.0,
                float initial_age = 0,
                int gender = 0,
                int initial_infections = 0,
                float immunity_parameter = 1.0,
                float risk_parameter = 1.0
            ) override
            {
                //std::cout << "Would have created individual with mcw=" << monte_carlo_weight << ", age = " << initial_age << ", gender = " << gender << std::endl;
                if( create_person_callback == nullptr )
                {
                    std::cout << "Need to set create_person callback." << std::endl;
                    return nullptr;
                }
                PyObject *arglist = Py_BuildValue("(f, f, i)", monte_carlo_weight, initial_age, gender );
                PyObject_CallObject(create_person_callback, arglist);
                Py_XDECREF(arglist);
                return nullptr;
            }
        public:
            virtual float initiatePregnancyForIndividual( int individual_id, float duration ) override
            {
                //std::cout << "initiatePregnancyForIndividual: individual_id = " << individual_id << " and duration " << duration << std::endl;
                // Call out to Py layer so it can call into dtk_generic_individual API
                PyObject *arglist = Py_BuildValue("(i, f)", individual_id, duration );
                PyObject_CallObject(conceive_baby_callback, arglist);
                Py_XDECREF(arglist);
                return 0;
            }
            virtual bool updatePregnancyForIndividual( int individual_id, float dt ) override
            {
                //std::cout << "updatePregnancyForIndividual: individual_id = " << individual_id << std::endl;
                // TBD: Call out to Py layer so it can call into dtk_generic_individual API
                PyObject *arglist = Py_BuildValue("(i, f)", individual_id, dt );
                PyObject *pyRetValue = PyObject_CallObject(update_preg_callback, arglist);
                bool retValue = false;
                PyArg_Parse( pyRetValue, "b", &retValue );
                Py_XDECREF(arglist);
                Py_XDECREF(pyRetValue);
                return retValue;
            }
            virtual void populateNewIndividualFromMotherId( unsigned int mother_id ) // override?
            {
                // Need to go up to Py layer and out to Py Individuals to get the info for this mom
                populateNewIndividualFromMotherParams( 1.0 /*mcw)*/, 0 /*child_infections*/ );
            }
            static std::string result;
    };

    std::string TestNode::result = "";
    RANDOMBASE* TestSimulation::m_pRNG = nullptr;
}

Test::TestSimulation* p_testParentSim = nullptr;

// Json-configure & Initialize a (single) individual
// Use json file on disk.
static void initSim( const char* filename = "nd.json" )
{
    if( configStubJson != nullptr )
    {
        delete configStubJson;
    }
    configStubJson = Configuration::Load( filename );

    int rng_seed = 0;
    if( configStubJson->Exist("Run_Number") )
    {
        rng_seed = (*configStubJson)["Run_Number"].As<json::Number>();
    }
    Test::TestSimulation::SeedRNG(rng_seed);

    // create test sim if it does not exist
    if( p_testParentSim != nullptr )
    {
        delete p_testParentSim;
        p_testParentSim = nullptr;
    }    
    p_testParentSim = new Test::TestSimulation();

    // set Base_Year in the sim if value given
    float base_year = 0.0;
    if( configStubJson->Exist("Base_Year") )
    {
        base_year = (*configStubJson)["Base_Year"].As<json::Number>();
    }
    p_testParentSim->SetBaseYear( base_year );
    
#ifndef WIN32
    const json::Object& config_obj = json_cast<const Object&>( *configStubJson );
    json::Object * config_obj_mutable = const_cast<Object*>( &config_obj );
    //std::cout << "Overriding loaded config.json with " << userParams.size() << " parameters." << std::endl;
    for( auto config : userParams )
    {
        std::string key = config.first;
        float value = config.second;
        std::cout << "Overriding " << key << " with value " << value << std::endl;
        (*config_obj_mutable)[ key ] = json::Number( value );
        std::cout << "[verbose] Overrode " << key << " with value " << value << std::endl;
    }
    for( auto config : userParamsEnum )
    {
        std::string key = config.first;
        auto value = config.second;
        std::cout << "Overriding " << key << " with value " << value << std::endl;
        (*config_obj_mutable)[ key ] = json::String( value );
    }
#endif
    EnvPtr->InputPaths.push_back( "." );
    EnvPtr->Config = configStubJson;
    NPFactory::CreateFactory();
    IPFactory::CreateFactory();

    NodeConfig         node_config_obj;
    node_config_obj.Configure(configStubJson);
}

// create nodes and populate from input files
static PyObject*
pop(PyObject* self, PyObject* args)
{
    std::cout << "pop: calling initSim." << std::endl;
    char * config_filename = "nd.json";
    if( !PyArg_ParseTuple(args, "|s", &config_filename ) )
    {
        std::cout << "Failure parsing demographics filename." << std::endl;
    }

    try {
        for (auto node : nodes)
        {
            delete node;
        }
        nodes.clear();

        // Initialize node demographics from file
        initSim( config_filename );
        pyNodeDemogInit();
        demographics_factory = Kernel::NodeDemographicsFactory::CreateNodeDemographicsFactory(EnvPtr->Config);
        if (demographics_factory == nullptr)
        {
            throw InitializationException( __FILE__, __LINE__, __FUNCTION__, "Failed to create NodeDemographicsFactory" );
        }

        vector<uint32_t> nodeIDs = demographics_factory->GetNodeIDs(); 

        auto suid_gen = suids::distributed_generator(0,0);
        // Add nodes according to demographics-and climate file specifications
        for (auto node_id : nodeIDs)
        {
            suids::suid node_suid = suid_gen();
            std::cout << "Creating node." << std::endl;
            auto *node = new Test::TestNode(p_testParentSim, node_id, node_suid);
            release_assert( node );
            std::cout << "Got node." << std::endl;
            Kernel::JsonConfigurable::_useDefaults = false; 
            node->SetParameters(demographics_factory, nullptr, false );
            std::cout << "Set Parameters." << std::endl;
            node->PopulateFromDemographics();
            std::cout << "Back from PopulateFromDemographics." << std::endl;
            nodes.push_back( node );
        }
    }
    catch( Kernel::DetailedException &exc )
    {
        std::cout << exc.GetMsg() << std::endl;
        PyErr_SetString( PyExc_RuntimeError, "config json file not found probably." );
        return NULL;
    }

    Py_RETURN_NONE;
}

// 
// Supporting GetSchema here
//

static PyObject*
getSchema(PyObject* self, PyObject* args)
{
    bool ret = false;
    //PyModKernel::Simulation ti;
    //std::cout << ti.result.c_str() << std::endl;
    //return Py_BuildValue("s", ti.result.c_str() );

    initSim();
    pyNodeDemogInit();
    
    // This is copy-pasted just to get everything initialized just so we can call GetSchema
    demographics_factory = Kernel::NodeDemographicsFactory::CreateNodeDemographicsFactory(EnvPtr->Config);
    vector<uint32_t> nodeIDs = demographics_factory->GetNodeIDs(); 
    auto suid_gen = suids::distributed_generator(0,0);
    Test::TestSimulation testParentSim_tmp;
    suids::suid node_suid = suid_gen();
    auto *node = new Test::TestNode(&testParentSim_tmp, 1, node_suid);
    node->getSchema();
    
    return Py_BuildValue("s", Test::TestNode::result.c_str() );
}

static PyObject*
setParam(PyObject* self, PyObject* args)
{
    char * param_name;
    float param_value;
    if( !PyArg_ParseTuple(args, "(sf)", &param_name, &param_value ) )
    {
        std::cout << "Failed to parse in setParam as float." << std::endl;
        //abort();
    } 
    else
    {
        userParams[ param_name ] = param_value;
    }
    std::cout << "Set param " << param_name << " to value " << param_value << std::endl;
    
    Py_RETURN_NONE;
}

static PyObject*
setEnumParam(PyObject* self, PyObject* args)
{
    char * param_name;
    char * param_value_enum;
    if( !PyArg_ParseTuple(args, "(ss)", &param_name, &param_value_enum ) )
    {
        std::cout << "Failed to parse in setParam as string." << std::endl;
        //abort();
    }
    else
    {
        userParamsEnum[ param_name ] = param_value_enum;
    }
    std::cout << "Set param " << param_name << " to (enum) value " << param_value_enum << std::endl;
    
    Py_RETURN_NONE;
}

static PyObject*
updateFertility(PyObject* self, PyObject* args)
{
    // TBD: Parse dt
    float dt = 1.0;

    nodes[0]->updateVitalDynamics( dt );
    nodes[0]->resetNodeStateCounters();
    p_testParentSim->Update();
    Py_RETURN_NONE;
}

static PyObject*
considerIndividualForPregnancy(PyObject* self, PyObject* args)
{
    bool poss_mom, is_preg;
    float age, dt;
    int ind_id;
    if( !PyArg_ParseTuple(args, "(bbiff)", &poss_mom, &is_preg, &ind_id, &age, &dt ) )
    {
        std::cerr << "Failed to parse in considerIndividualForPregnancy." << std::endl;
        //abort();
    }
    nodes[0]->considerPregnancyForIndividual( poss_mom, is_preg, age, ind_id, dt );
    Py_RETURN_NONE;
}

static PyObject*
getMortalityRate(PyObject* self, PyObject* args)
{
    float age = 0.0f;
    int sex = 0;
    float rate = 0;
    if( !PyArg_ParseTuple(args, "(fi)", &age, &sex ) )
    {
        std::cerr << "Failed to parse in getMortalityRate." << std::endl;
        //abort();
    }
    else
    {
        rate = nodes[0]->GetNonDiseaseMortalityRateByAgeAndSex( age, Gender::Enum(sex) );
    }
    return Py_BuildValue("f", rate );
}

static PyObject*
updateNodeStats(PyObject* self, PyObject* args)
{
    float mcw = 1.0f;
    float infectiousness = 0.0f;
    bool is_poss_mom = false;
    bool is_infected = false;
    bool is_symptomatic = false;
    bool is_newly_symptomatic = false;
    if( !PyArg_ParseTuple(args, "(ffbb)", &mcw, &infectiousness, &is_poss_mom, &is_infected ) )
    {
        std::cerr << "Failed to parse in updateNodeStats." << std::endl;
        //abort();
    }
    else
    {
        nodes[0]->accumulateIndividualPopStatsByValue( mcw, infectiousness, is_poss_mom, is_infected, is_symptomatic, is_newly_symptomatic );
    }
    Py_RETURN_NONE;
}

static PyObject*
resetBaseYear(PyObject* self, PyObject* args)
{
    float year = 0.0f;
    if( !PyArg_ParseTuple(args, "f", &year ) )
    {
        std::cerr << "Failed to parse year in resetBaseYear. We'll just use 0 (which is what the value should be anyway in Generic_Sim)." << std::endl;
    }
    std::cout << "Setting base year to " << year << std::endl;
    //testParentSim.setBaseYear( year );
    p_testParentSim->setBaseYear( year );
    Py_RETURN_NONE;
}

static PyObject*
reset(PyObject* self, PyObject* args)
{
    userParams.clear();
    userParamsEnum.clear();
    Py_RETURN_NONE;
}

// PyMod contract code below
static PyMethodDef NodeDemogMethods[] =
{
     {"populate_from_files", pop, METH_VARARGS, "Ask NodeDemographics code to load and process demographics files and call create_person callback accordingly."},
     {"set_callback", my_set_callback, METH_VARARGS, "Set callback for creating people."},
     {"set_conceive_baby_callback", set_conceive_baby_callback, METH_VARARGS, "Set callback for conceiving a baby."},
     {"set_update_preg_callback", set_update_preg_callback, METH_VARARGS, "Set callback updating individual pregnancies."},
     {"get_schema", getSchema, METH_VARARGS, "Get config schema."},
     {"set_param", setParam, METH_VARARGS, "Setting a config.json-type param."},
     {"set_enum_param", setEnumParam, METH_VARARGS, "Setting a config.json-type param."},
     {"update_fertility", updateFertility, METH_VARARGS, "Check for new births (assuming non-individual model)."},
     {"consider_for_pregnancy", considerIndividualForPregnancy, METH_VARARGS, "consider_for_pregnancy"},
     {"get_mortality_rate", getMortalityRate, METH_VARARGS, "get mortality rate"},
     {"update_node_stats", updateNodeStats, METH_VARARGS, "update node stats so fertility values can be calculated correctly"},
     {"reset", reset, METH_VARARGS, "Clear user params variables."},
     {"reset_base_year", resetBaseYear, METH_VARARGS, "Reset base year (temporary hopefully)"},
     {NULL, NULL, 0, NULL}
};

static struct PyModuleDef moduledef = {
    PyModuleDef_HEAD_INIT,
    "dtk_nodedemog",
    nullptr,
    0,
    NodeDemogMethods,
    nullptr,
    nullptr,
    nullptr,
    nullptr
};

PyMODINIT_FUNC
PyInit_dtk_nodedemog(void)
{
    PyObject* module = PyModule_Create( &moduledef );
    reset( nullptr, nullptr );

    return module;
}
