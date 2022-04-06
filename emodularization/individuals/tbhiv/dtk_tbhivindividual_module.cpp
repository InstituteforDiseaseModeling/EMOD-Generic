/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include <Python.h>
#include <iostream>
#include "RANDOM.h"

#include "suids.hpp"
#include "ConfigParams.h"
#include "Environment.h"
#include "NodeEventContext.h"
#include "IndividualCoInfection.h"
#include "INodeContext.h"
#include "Properties.h"
#include "JsonFullWriter.h"
#include "InfectionTB.h"
#include "InfectionHIV.h"
#include "SusceptibilityTB.h"
#include "SusceptibilityHIV.h"
#include "NodeTB.h"
#include "NodeTBHIV.h"
#include "StrainIdentity.h"
#include "IdmDateTime.h"

Kernel::IndividualHumanCoInfection * person = nullptr;
Configuration * configStubJson = nullptr;

using namespace Kernel;
void pyMathFuncInit() { }
#include "../pymod_stubnode.h"
#include "../base_pymod_individual.h"

PyObject * StubNode::my_callback = nullptr;
PyObject * StubNode::deposit_callback = nullptr;
PyObject * StubNode::mortality_callback = nullptr;

static suids::distributed_generator * individualHumanSuidGenerator = new suids::distributed_generator(0,1);
std::map< int, Kernel::IndividualHumanCoInfection * > population;

static std::map< std::string, float > userParams;
static std::map< std::string, std::string > userParamsEnum;

// Declare single node here
StubNode node;


// Boiler-plate callback setting function that will be put in common file.
PyObject* my_set_callback(PyObject *dummy, PyObject *args)
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
        Py_XDECREF(StubNode::my_callback);  /* Dispose of previous callback */ 
        StubNode::my_callback = temp;       /* Remember new callback */
        /* Boilerplate to return "None" */
        Py_INCREF(Py_None);
        result = Py_None;
    }
    return result;
}

// Boiler-plate callback setting function that will be put in common file.
PyObject* set_mortality_callback(PyObject *dummy, PyObject *args)
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
        Py_XDECREF(StubNode::mortality_callback);  /* Dispose of previous callback */ 
        StubNode::mortality_callback = temp;       /* Remember new callback */
        /* Boilerplate to return "None" */
        Py_INCREF(Py_None);
        result = Py_None;
    }
    return result;
}

// Boiler-plate callback setting function that will be put in common file.
PyObject* set_deposit_callback(PyObject *dummy, PyObject *args)
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
        Py_XDECREF(StubNode::deposit_callback);  /* Dispose of previous callback */ 
        StubNode::deposit_callback = temp;       /* Remember new callback */
        /* Boilerplate to return "None" */
        Py_INCREF(Py_None);
        result = Py_None;
    }
    return result;
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


static void setConfigJson()
{
    if( configStubJson != nullptr )
    {
        delete configStubJson;
    }

    configStubJson = Configuration::Load("tbhiv.json");
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
    EnvPtr->Config = configStubJson;
    Environment::setSimulationConfig( configStubJson );
}

// Json-configure & Initialize a (single) individual
// Use json file on disk.
static Kernel::IndividualHumanCoInfection* initInd( int sex, float age, float mcw )
{
    auto person = Kernel::IndividualHumanCoInfection::CreateHuman( &node, (*individualHumanSuidGenerator)(), mcw, age, sex );
    if( configStubJson == nullptr )
    {
        //configStubJson = Configuration::Load("gi.json");
        setConfigJson();

        std::cout << "configStubJson initialized from gi.json." << std::endl;
        //Kernel::JsonConfigurable::_useDefaults = true;

        ConfigParams                       gen_config_obj;
        IndividualHumanConfig              gen_individual_config_obj;
        SusceptibilityConfig               gen_susceptibility_config_obj;
        InfectionConfig                    gen_infection_config_obj;

        InfectionAirborneConfig            air_infection_config;

        IndividualHumanCoInfectionConfig   coi_individual_config_obj;
        SusceptibilityTBConfig             coi_susceptibility_config_obj;
        InfectionTBConfig                  coi_infection_config_obj;

        InfectionSTIConfig                 sti_infection_config_obj;

        SusceptibilityHIVConfig            hiv_susceptibility_config_obj;
        InfectionHIVConfig                 hiv_infection_config_obj;

        gen_config_obj.Configure( configStubJson );
        gen_individual_config_obj.Configure( configStubJson );
        gen_susceptibility_config_obj.Configure( configStubJson );
        gen_infection_config_obj.Configure( configStubJson );

        air_infection_config.Configure( configStubJson );

        coi_individual_config_obj.Configure( configStubJson );
        coi_susceptibility_config_obj.Configure( configStubJson );
        coi_infection_config_obj.Configure( configStubJson );

        if( IndividualHumanCoInfectionConfig::enable_coinfection )
        {
            coi_individual_config_obj.SetCD4Map( coi_infection_config_obj.GetCD4Map() );

            sti_infection_config_obj.Configure( configStubJson );

            hiv_susceptibility_config_obj.Configure( configStubJson );
            hiv_infection_config_obj.Configure( configStubJson );
        }

        std::cout << "Initialized Statics from gi.json." << std::endl;
    }
    Kernel::JsonConfigurable::_useDefaults = false; 
    person->SetParameters( &node, 0.0f, 1.0f, 1.0f );
    return person;
}


// create individual human 
static PyObject*
create(PyObject* self, PyObject* args)
{
    int sex;
    float age;
    float mcw;
    if( !PyArg_ParseTuple(args, "(iff)", &sex, &age, &mcw ) )
    {
        std::cout << "Failed to parse individual create params." << std::endl;
    }
    try
    {
        auto person = initInd( sex, age, mcw );
        std::cout << "Created individual with id " << person->GetSuid().data << std::endl;
        population[ person->GetSuid().data ] = person;
        return Py_BuildValue( "i", person->GetSuid().data );
    }
    catch( const std::exception& exc )
    {
        std::cerr << exc.what() << std::endl;
        PyErr_SetString( PyExc_RuntimeError, "gi.json file not found probably." );
        return NULL;
    }
}

//
// Some functions to interact with the human
//
// update individual (hard-coded time values)
static PyObject*
update(PyObject* self, PyObject* args)
{
    int id;
    if( !PyArg_ParseTuple(args, "i", &id ) )
    {
        std::cout << "Failed to parse id update (as int)." << std::endl;
    }
    if( population.find( id ) != population.end() )
    {
        //std::cout << "Calling updateInfectiousness on individual " << id << std::endl;
        population.at(id)->UpdateInfectiousness( 1.0 ); // includes shedding
        //std::cout << "Calling update on individual " << id << std::endl;
        population.at(id)->Update(1.0f, 1.0f); // includes exposure & natural mortality
        //std::cout << "Called update on individual " << id << std::endl;
    }
    else
    {
        std::cout << "ERROR: Didn't find individual with id " << id << " in our population map." << std::endl;
    }
    Py_RETURN_NONE;
}

static PyObject*
updateShedding(PyObject* self, PyObject* args)
{
    int id;
    if( !PyArg_ParseTuple(args, "i", &id ) )
    {
        std::cout << "Failed to parse id update (as int)." << std::endl;
    }
    if( population.find( id ) != population.end() )
    {
        //std::cout << "Calling UpdateInfectiousness on individual " << id << std::endl;
        population.at(id)->UpdateInfectiousness( 1.0 ); // includes shedding
    }
    else
    {
        std::cout << "ERROR: Didn't find individual with id " << id << " in our population map." << std::endl;
    }
    
    Py_RETURN_NONE;
}

static PyObject*
updateMain(PyObject* self, PyObject* args)
{
    int id;
    if( !PyArg_ParseTuple(args, "i", &id ) )
    {
        std::cout << "Failed to parse id update (as int)." << std::endl;
    }
    if( population.find( id ) != population.end() )
    {
        //std::cout << "Calling update on individual " << id << std::endl;
        population.at(id)->Update(1.0f, 1.0f); // includes exposure & natural mortality
        //std::cout << "Called update on individual " << id << std::endl;
    }
    else
    {
        std::cout << "ERROR: Didn't find individual with id " << id << " in our population map." << std::endl;
    }
    Py_DECREF( args );
    
    Py_RETURN_NONE;
}

/*
static PyObject*
setEnumParam(PyObject* self, PyObject* args)
{
    char * param_name;
    person->Update( 0.0f, 1.0f );

    Py_RETURN_NONE;
}
*/

static PyObject*
getAge(PyObject* self, PyObject* args)
{
    int id;
    if( !PyArg_ParseTuple(args, "i", &id ) )
    {
        std::cout << "Failed to parse id update (as int)." << std::endl;
    }
    auto age = population[id]->GetAge();
    return Py_BuildValue("f", age );
}

static PyObject*
isPossibleMother(PyObject* self, PyObject* args)
{
    int id;
    if( !PyArg_ParseTuple(args, "i", &id ) )
    {
        std::cout << "Failed to parse id for isPossibleMother (as int)." << std::endl;
    }
    bool answer = population[id]->IsPossibleMother();
    return Py_BuildValue("b", answer );
}

static PyObject*
isPregnant(PyObject* self, PyObject* args)
{
    int id;
    if( !PyArg_ParseTuple(args, "i", &id ) )
    {
        std::cout << "Failed to parse id for isPregnant (as int)." << std::endl;
    }
    auto answer = population[id]->IsPregnant();
    return Py_BuildValue("b", answer );
}

static PyObject*
initiatePregnancy(PyObject* self, PyObject* args)
{
    int id;
    if( !PyArg_ParseTuple(args, "i", &id ) )
    {
        std::cout << "Failed to parse id for initiatePregnancy (as int)." << std::endl;
    }
    std::cout << "Initiating pregnancy for individual " << id << " of age " << population.at(id)->GetAge() << std::endl;
    population.at(id)->InitiatePregnancy();
    return Py_BuildValue("b", true );
}

static PyObject*
updatePregnancy(PyObject* self, PyObject* args)
{
    int id;
    int dt;
    if( !PyArg_ParseTuple(args, "ii", &id, &dt ) )
    {
        std::cout << "Failed to parse id for updatePregnancy (as int & float)." << std::endl;
        return Py_BuildValue( "b", 0 );
    }
    //std::cout << "Updating pregnancy for individual " << id << " with dt " << dt << std::endl;
    if( population.find( id ) == population.end() )
    {
        return Py_BuildValue( "b", 0 );
    }

    bool up = population.at(id)->UpdatePregnancy( dt );
    return Py_BuildValue("b", up );
}

static PyObject*
isDead(PyObject* self, PyObject* args)
{
    int id;
    if( !PyArg_ParseTuple(args, "i", &id ) )
    {
        std::cout << "Failed to parse id for isDead (as int)." << std::endl;
    }
    auto answer = population[id]->IsDead();
    PyObject * retObj = Py_BuildValue("b", answer );
    //Py_DECREF( retObj );
    return retObj;
}

static PyObject*
isInfected(PyObject* self, PyObject* args)
{
    // TBD: Get id. For now, assume 0
    int id;
    if( !PyArg_ParseTuple(args, "i", &id ) )
    {
        std::cout << "Failed to parse id for isInfected (as int)." << std::endl;
    }
    //std::cout << "IsInfected accessing individual 0 in population map of size " << population.size() << std::endl;
    bool inf_status = population.at( id )->IsInfected();
    //std::cout << "IsInfected returned " << inf_status  << std::endl;
    return Py_BuildValue("b", inf_status );
}

static PyObject*
getImmunity(PyObject* self, PyObject* args)
{
    // TBD: Get id. 
    int id;
    if( !PyArg_ParseTuple(args, "i", &id ) )
    {
        std::cout << "Failed to parse id for getImmunity (as int)." << std::endl;
    }
    float imm = population[ id ]->GetImmunityReducedAcquire();
    return Py_BuildValue("f", imm );
}

// Supporting GetSchema here
//
// Simulation class is a friend which is necessary for calling Configure
namespace Kernel {
    class Simulation
    {
        public:
            static std::string result;
            Simulation()
            {
                Kernel::JsonConfigurable::_dryrun = true;
                Kernel::IndividualHumanCoInfectionConfig adam;
                adam.Configure( nullptr ); // protected
                auto schema = adam.GetSchema();
                json::Object root;
                root["Individual"] = schema;

                // TBD: I want Infection.GetSchema() here too.
                Kernel::InfectionTBConfig fakeInfection;
                fakeInfection.Configure( nullptr ); // protected
                auto schema2 = fakeInfection.GetSchema();
                root["Infection"] = schema2;

                Kernel::SusceptibilityTBConfig fakeImmunity;
                fakeImmunity.Configure( nullptr ); // protected
                auto schema3 = fakeImmunity.GetSchema();
                root["Immunity"] = schema3;

                std::ostringstream schema_ostream;
                json::Writer::Write( root, schema_ostream );
                std::cout << schema_ostream.str() << std::endl;
                result = schema_ostream.str();
                Kernel::JsonConfigurable::_dryrun = false;
            }
    };
    std::string Simulation::result = "";
}

static PyObject*
getSchema(PyObject* self, PyObject* args)
{
    bool ret = false;
    Kernel::Simulation ti;
    //std::cout << ti.result.c_str() << std::endl;
    return Py_BuildValue("s", ti.result.c_str() );
}

static PyObject*
serialize(PyObject* self, PyObject* args)
{
    int id;
    if( !PyArg_ParseTuple(args, "i", &id ) )
    {
        std::cout << "Failed to parse id in serialize (as int)." << std::endl;
    }
    IArchive* writer = static_cast<IArchive*>(new JsonFullWriter());
    ISerializable* serializable = dynamic_cast<ISerializable*>(population.at( id ));
    (*writer).labelElement( "individual" ) & serializable;
    std::string serialized_man = (*writer).GetBuffer(); // , (*writer).GetBufferSize(), t, true );
    delete writer;
    //std::cout << serialized_man.c_str() << std::endl;
    return Py_BuildValue("s", serialized_man.c_str() );
}

static PyObject*
reset(PyObject* self, PyObject* args)
{
    configStubJson = nullptr;
    population.clear();
    if( individualHumanSuidGenerator != nullptr )
    {
        delete individualHumanSuidGenerator;
    }
    individualHumanSuidGenerator = new suids::distributed_generator(0,1);
    Py_RETURN_NONE;
}

static PyObject*
shouldInfect(PyObject* self, PyObject* args)
{
    // Get id. 
    int id;
    float contagion;
    if( !PyArg_ParseTuple(args, "(if)", &id, &contagion ) )
    {
        std::cout << "Failed to parse id for isInfected (as int)." << std::endl;
    }
    bool should_infect = population.at( id )->ShouldAcquire( contagion, 1, -1 );
    //std::cout << "IsInfected returned " << inf_status  << std::endl;
    return Py_BuildValue("b", should_infect );
}

static PyObject*
getInfectionAge(PyObject* self, PyObject* args)
{
    // Get id. 
    int id;
    if( !PyArg_ParseTuple(args, "i", &id ) )
    {
        std::cout << "Failed to parse id for getInfectionAge (as int)." << std::endl;
    }
    float inf_age = population[ id ]->GetInfections().front()->GetDuration();
    return Py_BuildValue("f", inf_age );
}

static PyObject*
getStateChange(PyObject* self, PyObject* args)
{
    // Get id. 
    int id;
    if( !PyArg_ParseTuple(args, "i", &id ) )
    {
        std::cout << "Failed to parse id for getInfectionAge (as int)." << std::endl;
    }
    int state_change /*as int*/ = int(population[ id ]->GetStateChange());
    return Py_BuildValue("i", state_change );
}

static PyObject*
getInfectionStateAge(PyObject* self, PyObject* args)
{
    // Get id. 
    int id;
    if( !PyArg_ParseTuple(args, "i", &id ) )
    {
        std::cout << "Failed to parse id for getInfectionStateAge (as int)." << std::endl;
    }
    // Proposed logic: If latent, get infection age. Else get active stage duration.
    abort(); // not finished coding yet.
    float inf_age = population[ id ]->GetInfections().front()->GetDuration();
    return Py_BuildValue("f", inf_age );
}

static PyObject*
getInfectionState(PyObject* self, PyObject* args)
{
    // Get id. 
    int id;
    if( !PyArg_ParseTuple(args, "i", &id ) )
    {
        std::cout << "Failed to parse id for getInfectionStateAge (as int)." << std::endl;
    }
    int state = population[ id ]->GetTBInfectionState();
    return Py_BuildValue("i", state );
}

static PyObject*
getInfectionStateChange(PyObject* self, PyObject* args)
{
    // Get id. 
    int id;
    if( !PyArg_ParseTuple(args, "i", &id ) )
    {
        std::cout << "Failed to parse id for getInfectionStateAge (as int)." << std::endl;
    }
    int state_change = population[ id ]->GetNewInfectionState();
    return Py_BuildValue("i", state_change );
}

static PyObject*
getInfectiousness(PyObject* self, PyObject* args)
{
    // Get id. 
    int id;
    if( !PyArg_ParseTuple(args, "i", &id ) )
    {
        std::cout << "Failed to parse id for getInfectionStateAge (as int)." << std::endl;
    }
    int state_change = population[ id ]->GetInfectiousness();
    return Py_BuildValue("i", state_change );
}

static PyObject*
getCD4(PyObject* self, PyObject* args)
{
    // Get id. 
    int id;
    if( !PyArg_ParseTuple(args, "i", &id ) )
    {
        std::cout << "Failed to parse id for getCD4 (as int)." << std::endl;
    }
    float cd4 = population[ id ]->GetCD4();
    return Py_BuildValue("f", cd4 );
}

static PyObject*
hasActiveInfection(PyObject* self, PyObject* args)
{
    // Get id. 
    int id;
    if( !PyArg_ParseTuple(args, "i", &id ) )
    {
        std::cout << "Failed to parse id for hasActiveInfection (as int)." << std::endl;
    }
    bool ret = population[ id ]->HasActiveInfection();
    return Py_BuildValue("b", ret );
}

static PyObject*
hasLatentInfection(PyObject* self, PyObject* args)
{
    // Get id. 
    int id;
    if( !PyArg_ParseTuple(args, "i", &id ) )
    {
        std::cout << "Failed to parse id for hasLatentInfection (as int)." << std::endl;
    }
    bool ret = population[ id ]->HasLatentInfection();
    return Py_BuildValue("b", ret );
}

static PyObject*
hasActivePresymptomaticInfection(PyObject* self, PyObject* args)
{
    // Get id. 
    int id;
    if( !PyArg_ParseTuple(args, "i", &id ) )
    {
        std::cout << "Failed to parse id for hasLatentInfection (as int)." << std::endl;
    }
    bool ret = population[ id ]->HasActivePresymptomaticInfection();
    return Py_BuildValue("b", ret );
}

static PyObject*
hasHIV(PyObject* self, PyObject* args)
{
    // Get id. 
    int id;
    if( !PyArg_ParseTuple(args, "i", &id ) )
    {
        std::cout << "Failed to parse id for hasHIV (as int)." << std::endl;
    }
    bool ret = population[ id ]->HasHIV();
    return Py_BuildValue("b", ret );
}

static PyObject*
hasTB(PyObject* self, PyObject* args)
{
    // Get id. 
    int id;
    if( !PyArg_ParseTuple(args, "i", &id ) )
    {
        std::cout << "Failed to parse id for hasTB (as int)." << std::endl;
    }
    bool ret = population[ id ]->HasTB();
    return Py_BuildValue("b", ret );
}

static PyObject*
isFast(PyObject* self, PyObject* args)
{
    // Get id. 
    int id;
    if( !PyArg_ParseTuple(args, "i", &id ) )
    {
        std::cout << "Failed to parse id for isFast (as int)." << std::endl;
    }
    bool ret = population[ id ]->IsFastProgressor();
    PyObject * retObj = Py_BuildValue("b", ret );
    //Py_DECREF( retObj );
    return retObj;
}
        
static PyObject*
giveHIV(PyObject* self, PyObject* args)
{
    // Get id. 
    int id;
    if( !PyArg_ParseTuple(args, "i", &id ) )
    {
        std::cout << "Failed to parse id for giveHIV (as int)." << std::endl;
    }
    StrainIdentity strainIDs; // just going to use antigen=0
    population[ id ]->AcquireNewInfectionHIV(&strainIDs, 0);
    Py_RETURN_NONE;
}

static PyObject*
getIndividual(PyObject* self, PyObject* args)
{
    // Get id. 
    int id;
    if( !PyArg_ParseTuple(args, "i", &id ) )
    {
        std::cout << "Failed to parse id for getIndividual (as int)." << std::endl;
    }
    return PyCapsule_New((void*)population[ id ], nullptr, nullptr);
}

// This using PyCapsules for first time.
// Function expects 2 params: intervention (capsule) and individual id (integer)
// The tutorial used PyArg_UnpackTuple but couldn't get that to work with both params.
static PyObject*
giveIntervention(PyObject *self, PyObject *args)
{
    PyObject* pf = NULL;
    int id = 1;
    //if(!PyArg_UnpackTuple(args, "funk", 1, 2, &pf, &id)) return NULL; // does not work
    //if(!PyArg_UnpackTuple(args, "funk", 1, 1, &pf)) return NULL; // I don't understand what funk means here!
    if( !PyArg_ParseTuple(args, "(iO)", &id, &pf ) )
    {
        std::cout << "Failed to parse id and/or pointer for giveIV." << std::endl;
    }
    // This val thing is a little funny. It's from tutorial and I haven't dug into it.
    //IDistributableIntervention* f = (IDistributableIntervention*)PyCapsule_GetPointer(pf, "val");
    IDistributableIntervention* f = (IDistributableIntervention*)PyCapsule_GetPointer(pf, nullptr );
    //std::cout << "iv name = " << f->GetName() << std::endl;
    //f->Update(0);
    dynamic_cast<IInterventionConsumer*>(population[ id ]->GetInterventionsContext())->GiveIntervention( f );
    // The DTK usually distributes interventions by calling iv->Distribute( individual ) but here
    // I'm skipping that step and calling individual->GiveIV( iv )
    Py_RETURN_NONE;
}

static PyObject*
resetArtTimers(PyObject *self, PyObject *args)
{
    PyObject* pf = NULL;
    int id = 1;
    if( !PyArg_ParseTuple(args, "i", &id ) )
    {
        std::cout << "Failed to parse id and/or pointer for giveIV." << std::endl;
    }

    population[ id ]->LifeCourseLatencyUpdateAll();

    Py_RETURN_NONE;
}

// PyMod contract code below
static PyMethodDef TBHIVIndividualMethods[] =
{
     {"create", create, METH_VARARGS, "Create somebody."},
     {"update", update, METH_VARARGS, "Update somebody."},
     {"update1", updateShedding, METH_VARARGS, "Update somebody."},
     {"update2", updateMain, METH_VARARGS, "Update somebody."},
     {"get_age", getAge, METH_VARARGS, "Get age."},
     {"is_infected", isInfected, METH_VARARGS, "Has 1+ infections."},
     {"get_immunity", getImmunity, METH_VARARGS, "Returns acquisition immunity (product of immune system and interventions modifier)."},
     {"my_set_callback", my_set_callback, METH_VARARGS, "Set callback."},
     {"set_mortality_callback", set_mortality_callback, METH_VARARGS, "Set callback."},
     {"set_deposit_callback", set_deposit_callback, METH_VARARGS, "Set callback."},
     {"get_schema", getSchema, METH_VARARGS, "Update."},
     {"set_param", setParam, METH_VARARGS, "Setting a config.json-type param."},
     {"set_enum_param", setEnumParam, METH_VARARGS, "Setting a config.json-type param."},
     {"initiate_pregnancy", initiatePregnancy, METH_VARARGS, "Initiate pregnancy."},
     {"update_pregnancy", updatePregnancy, METH_VARARGS, "Test."},
     {"is_possible_mother", isPossibleMother, METH_VARARGS, "Is possible mother."},
     {"is_pregnant", isPregnant, METH_VARARGS, "Is pregnant."},
     {"is_dead", isDead, METH_VARARGS, "Is dead."},
     {"serialize", serialize, METH_VARARGS, "Serialize to JSON."},
     {"reset", reset, METH_VARARGS, "Resets variables for new run (force read config really)."},
     {"get_state_change", getStateChange, METH_VARARGS, "Get latest state change (e.g., KilledByInfection)."},
     {"get_infection_age", getInfectionAge, METH_VARARGS, "Get age of infection."},
     {"get_infection_state", getInfectionState, METH_VARARGS, "Get state of (TB) infection."},
     {"get_infection_state_age", getInfectionStateAge, METH_VARARGS, "Get age of infection _state_."},
     {"get_infection_state_change", getInfectionStateChange, METH_VARARGS, "Get infection state CHANGE (aka, NewlyCleared)."},
     {"get_infectiousness", getInfectiousness, METH_VARARGS, "Get infectiousness."},
     {"should_infect", shouldInfect, METH_VARARGS, "Use standard TBHIV logic to determine if infection should occur."},
     {"get_cd4", getCD4, METH_VARARGS, "Get CD4 count for a given individual."},
     {"has_active_infection", hasActiveInfection, METH_VARARGS, ""},
     {"has_latent_infection", hasLatentInfection, METH_VARARGS, ""},
     {"has_hiv", hasHIV, METH_VARARGS, "Has HIV (co-infection)"},
     {"hapi", hasActivePresymptomaticInfection, METH_VARARGS, "Has moved from latent to active but is still in presymptomatic state."},
     {"has_tb", hasTB, METH_VARARGS, "Has TB (primary infection)"},
     {"is_fast", isFast, METH_VARARGS, "Is fast progressor (from latent to active)"},
     {"infect_with_hiv", giveHIV, METH_VARARGS, "Give HIV (co-infection)"},
     {"give_intervention", giveIntervention, METH_VARARGS, "Invoke pointer by calling instantiating function."},
     {"get_individual_for_iv", getIndividual<Kernel::IndividualHumanCoInfection>, METH_VARARGS, "Get PyCapsule pointer to pass to intervention."},
     {"reset_art_timers", resetArtTimers, METH_VARARGS, "Get pointer to individual object as pycapsule to give to, say, intervention."},
     {NULL, NULL, 0, NULL}
};

static struct PyModuleDef moduledef = {
    PyModuleDef_HEAD_INIT,
    "dtk_tbhiv_intrahost",
    nullptr,
    0,
    TBHIVIndividualMethods,
    nullptr,
    nullptr,
    nullptr,
    nullptr
};

PyMODINIT_FUNC
PyInit_dtk_tbhiv_intrahost(void)
{
     PyObject* module = PyModule_Create( &moduledef );

     return module;
}
