#include "IdmDateTime.h"

// 
// We'll need a stub node object as the individual's parent. Most functions are empty.
//
class StubNode : public INodeContext
{
    public:
        StubNode()
        : INodeContext()
        , node_properties()
        , m_pRNG(nullptr)
        , m_RouteList()
        , m_NodeId(Kernel::suids::nil_suid())
        {
            //IPFactory::CreateFactory();
            pyMathFuncInit();
            
            unsigned int randomseed[2];
            randomseed[0] = 0;
            randomseed[1] = 0;
            m_pRNG = new Kernel::PSEUDO_DES(*reinterpret_cast<uint32_t*>(randomseed));
        }

        virtual int32_t AddRef() { return 0; }
        virtual int32_t Release() { return 0; }

        Kernel::QueryResult QueryInterface( iid_t iid, void **ppinstance )
        {
            assert(ppinstance);

            if ( !ppinstance )
                return e_NULL_POINTER;

            ISupports* foundInterface;

            /*if ( iid == GET_IID(ISporozoiteChallengeConsumer)) 
                foundInterface = static_cast<ISporozoiteChallengeConsumer*>(this);*/
            // -->> add support for other I*Consumer interfaces here <<--      
            //else 
            if ( iid == GET_IID(ISupports)) 
                foundInterface = static_cast<ISupports*>((this));
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

        
        virtual void SetRng(Kernel::RANDOMBASE *) override { std::cout << __FUNCTION__ << std::endl; }
        virtual RANDOMBASE* GetRng()
        {
            return m_pRNG;
        }
        
        // This is so we can pass a faux-node
        virtual void SortHumans(void) override { std::cout << __FUNCTION__ << std::endl; }
        virtual void VisitIndividuals( INodeEventContext::individual_visit_function_t func) { std::cout << __FUNCTION__ << std::endl; }
        virtual int VisitIndividuals(IVisitIndividual* pIndividualVisitImpl ) { std::cout << __FUNCTION__ << std::endl; return 0; } 
        //virtual const NodeDemographics& GetDemographics() { std::cout << __FUNCTION__ << std::endl; return &demographics; }
        virtual bool GetUrban() const { std::cout << __FUNCTION__ << std::endl; return false; }
        virtual const IdmDateTime& GetTime() const { std::cout << __FUNCTION__ << std::endl; static IdmDateTime time(0); return time; }
        virtual void UpdateInterventions(float = 0.0f) { std::cout << __FUNCTION__ << std::endl; } 
        virtual void RegisterTravelDistributionSource(ITravelLinkedDistributionSource *tles, INodeEventContext::TravelEventType type) { std::cout << __FUNCTION__ << std::endl; }
        virtual void UnregisterTravelDistributionSource(ITravelLinkedDistributionSource *tles, INodeEventContext::TravelEventType type) { std::cout << __FUNCTION__ << std::endl; } 
        virtual const suids::suid & GetId() const { std::cout << __FUNCTION__ << std::endl; return m_NodeId; }
        virtual void SetContextTo(INodeContext* context) { std::cout << __FUNCTION__ << std::endl; }
        virtual std::list<INodeDistributableIntervention*> GetInterventionsByType(const std::string& type_name) { std::cout << __FUNCTION__ << std::endl; return std::list<INodeDistributableIntervention*>(); }
        virtual void PurgeExisting( const std::string& iv_name ) { std::cout << __FUNCTION__ << std::endl; } 
        virtual bool IsInPolygon(float* vertex_coords, int numcoords) { std::cout << __FUNCTION__ << std::endl; return false; }
        virtual bool IsInExternalIdSet( const std::list<ExternalNodeId_t>& nodelist ) { std::cout << __FUNCTION__ << std::endl; return false; }
        virtual INodeContext* GetNodeContext() { std::cout << __FUNCTION__ << std::endl; return nullptr; } 
        virtual int GetIndividualHumanCount() const { std::cout << __FUNCTION__ << std::endl; return 0; }
        virtual ExternalNodeId_t GetExternalId() const { std::cout << __FUNCTION__ << std::endl; return 0; }

        virtual ISimulationContext* GetParent() override { std::cout << __FUNCTION__ << std::endl; return nullptr; }
        virtual suids::suid GetSuid() const override { Kernel::suids::nil_suid(); return Kernel::suids::nil_suid(); }
        virtual void SetContextTo( ISimulationContext* ) override { std::cout << __FUNCTION__ << std::endl; }
        virtual void SetParameters(NodeDemographicsFactory *demographics_factory, ClimateFactory *climate_factory, bool whitelistsomething ) override { std::cout << __FUNCTION__ << std::endl; }
        virtual void PopulateFromDemographics() override { std::cout << __FUNCTION__ << std::endl; }
        virtual suids::suid GetNextInfectionSuid() override {
            //std::cout << __FUNCTION__ << std::endl;
            return Kernel::suids::nil_suid();
        }
        virtual void Update(float dt) override { std::cout << __FUNCTION__ << std::endl; }
        virtual IIndividualHuman* processImmigratingIndividual( IIndividualHuman* ) override { return nullptr; }
        
        virtual void GetGroupMembershipForIndividual(const RouteList_t& route, const tProperties& properties, TransmissionGroupMembership_t& membershipOut ) override { std::cout << __FUNCTION__ << std::endl; }
        virtual void UpdateTransmissionGroupPopulation(const tProperties& properties, float size_changes,float mc_weight) override { std::cout << __FUNCTION__ << std::endl; }
        virtual std::map< std::string, float > GetContagionByRoute() const { std::cout << __FUNCTION__ << std::endl; return std::map< std::string, float >(); }
        virtual float GetTotalContagion( void ) { std::cout << __FUNCTION__ << std::endl; return 0; }
        virtual ITransmissionGroups* GetTransmissionGroups() const override { std::cout << __FUNCTION__ << std::endl; return nullptr; }
        virtual float GetContagionByRouteAndProperty( const std::string& route, const IPKeyValue& property_value ) override  { std::cout << __FUNCTION__ << std::endl; return 0; }


        virtual const RouteList_t& GetTransmissionRoutes( ) const override { std::cout << __FUNCTION__ << std::endl; return m_RouteList; }
        virtual float getSinusoidalCorrection(float sinusoidal_amplitude, float sinusoidal_phase) const override { std::cout << __FUNCTION__ << std::endl; return 0; }
        virtual float getBoxcarCorrection(float boxcar_amplitude, float boxcar_start_time, float boxcar_end_time) const override { std::cout << __FUNCTION__ << std::endl; return 0; }
        virtual act_prob_vec_t DiscreteGetTotalContagion( void ) override { std::cout << __FUNCTION__ << std::endl; return act_prob_vec_t(); }
        virtual IMigrationInfo* GetMigrationInfo() override { std::cout << __FUNCTION__ << std::endl; return nullptr; }
        virtual const NodeDemographics* GetDemographics() const override { return nullptr; }
        //virtual const NodeDemographicsDistribution* GetDemographicsDistribution(std::string) const override { return nullptr; }
        virtual float GetNonDiseaseMortalityRateByAgeAndSex( float age, Gender::Enum sex ) const override
        {
            int sexAsInt = int(sex==Gender::FEMALE);
            PyObject *arglist = Py_BuildValue("(f,i)", age, sexAsInt );
            float mortality_rate = 0.0f;
            if( mortality_callback != nullptr )
            {
                PyObject *retVal = PyObject_CallObject(mortality_callback, arglist);
                mortality_rate = PyFloat_AsDouble( retVal );
                Py_DECREF(arglist);
            }
            return mortality_rate; 
        }

        virtual void DepositFromIndividual( const IStrainIdentity& strain_IDs, float contagion_quantity, TransmissionGroupMembership_t individual, TransmissionRoute::Enum route = TransmissionRoute::TRANSMISSIONROUTE_CONTACT) override
        {
            // Let's call into a python callback here to let python layer do transmission
            if( deposit_callback != nullptr )
            {
                int individual_id = strain_IDs.GetGeneticID();
                //std::cout << individual_id  << " depositing contagion." << std::endl;
                PyObject *arglist = Py_BuildValue("f,i", contagion_quantity, individual_id ); // TBD: Need to get more individual info?
                //PyObject *arglist = Py_BuildValue("f,i", 2.444, 55 ); // TBD: Need to get more individual info?
                PyObject *retVal = PyObject_CallObject(deposit_callback, arglist);
                Py_DECREF(arglist);
            }
            return;
        }

        virtual float       GetInfected()      const override { std::cout << __FUNCTION__ << std::endl; return 0.0f; }
        virtual float       GetSymptomatic()   const override { std::cout << __FUNCTION__ << std::endl; return 0.0f; }
        virtual float       GetNewlySymptomatic()      const override { std::cout << __FUNCTION__ << std::endl; return 0.0f; }
        virtual float       GetStatPop()       const override { std::cout << __FUNCTION__ << std::endl; return 0.0f; }
        virtual float       GetBirths()        const override { std::cout << __FUNCTION__ << std::endl; return 0.0f; }
        virtual float       GetCampaignCost()  const override { std::cout << __FUNCTION__ << std::endl; return 0.0f; }
        virtual float       GetInfectivity()   const override { std::cout << __FUNCTION__ << std::endl; return 0.0f; }
        virtual float       GetInfectionRate() const override { std::cout << __FUNCTION__ << std::endl; return 0.0f; }
        virtual float       GetSusceptDynamicScaling() const override { std::cout << __FUNCTION__ << std::endl; return 0.0f; }
        virtual const Climate* GetLocalWeather() const override { std::cout << __FUNCTION__ << std::endl; return nullptr; }
        virtual long int GetPossibleMothers()  const override { std::cout << __FUNCTION__ << std::endl; return 0; }
        virtual float GetMeanAgeInfection()    const override { std::cout << __FUNCTION__ << std::endl; return 0; }
        virtual float GetLatitudeDegrees() override { std::cout << __FUNCTION__ << std::endl; return 0; }
        virtual float GetLongitudeDegrees() override { std::cout << __FUNCTION__ << std::endl; return 0; }
        virtual ExternalNodeId_t GetExternalID() const override { std::cout << __FUNCTION__ << std::endl; return 0; }
        virtual INodeEventContext* GetEventContext() override { return nullptr; }
        virtual void AddEventsFromOtherNodes( const std::vector<EventTrigger::Enum>& rTriggerList ) override { std::cout << __FUNCTION__ << std::endl; }
        virtual bool IsEveryoneHome() const override { std::cout << __FUNCTION__ << std::endl; return false; }
        virtual float GetBasePopulationScaleFactor() const override { std::cout << __FUNCTION__ << std::endl; return 0; }
        virtual ProbabilityNumber GetProbMaternalTransmission() const override { std::cout << __FUNCTION__ << std::endl; return 0; }
        virtual void SetupMigration( IMigrationInfoFactory * migration_factory, MigrationStructure::Enum ms, const boost::bimap<ExternalNodeId_t, suids::suid>& rNodeIdSuidMap ) override { std::cout << __FUNCTION__ << std::endl; }
        virtual std::vector<bool> GetMigrationTypeEnabledFromDemographics() const override { std::cout << __FUNCTION__ << std::endl; return std::vector<bool>(); }
        virtual void SetWaitingForFamilyTrip( suids::suid migrationDestination, MigrationType::Enum migrationType, float timeUntilTrip, float timeAtDestination, bool isDestinationNewHome ) override { std::cout << __FUNCTION__ << std::endl; }
        virtual void InitializeTransmissionGroupPopulations() {}
        virtual const NodeDemographicsDistribution* GetImmunityDistribution()        const { return nullptr; }
        virtual const NodeDemographicsDistribution* GetFertilityDistribution()       const { return nullptr; }
        virtual const NodeDemographicsDistribution* GetMortalityDistribution()       const { return nullptr; }
        virtual const NodeDemographicsDistribution* GetMortalityDistributionMale()   const { return nullptr; }
        virtual const NodeDemographicsDistribution* GetMortalityDistributionFemale() const { return nullptr; }
        virtual const NodeDemographicsDistribution* GetAgeDistribution()             const { return nullptr; }
        virtual NPKeyValueContainer& GetNodeProperties() { return node_properties; }
        virtual float GetMaxInfectionProb( TransmissionRoute::Enum tx_route )        const { return 0.0f; } 
        //virtual const std::vector<IIndividualHuman*>& GetHumans() const override { return std::vector<IIndividualHuman*>(); }
        virtual const std::vector<IIndividualHuman*>& GetHumans() const override { throw NotYetImplementedException( __FILE__, __LINE__, __FUNCTION__, "" ); }
        //virtual const std::map<std::string, int>&                 GetStrainClades()   const override { return std::map<std::string, int>(); }
        virtual const std::map<std::string, int>&                 GetStrainClades()   const override { throw NotYetImplementedException( __FILE__, __LINE__, __FUNCTION__, "" ); }
        //virtual const std::map<std::string, int>&                 GetStrainGenomes()  const override { return std::map<std::string, int>(); }
        virtual const std::map<std::string, int>&                 GetStrainGenomes()  const override { throw NotYetImplementedException( __FILE__, __LINE__, __FUNCTION__, "" ); }
        //virtual const std::map<std::string, std::vector<float>>&  GetStrainData()     const override { return std::map<std::string, std::vector<float>>(); }
        virtual const std::map<std::string, std::vector<float>>&  GetStrainData()     const override { throw NotYetImplementedException( __FILE__, __LINE__, __FUNCTION__, "" ); }

        // We want to be able to infect someone but not with existing Tx groups. Call into py layer here and get a bool back
        // True to infect, False to leave alone.
        virtual void ExposeIndividual(IInfectable* candidate, TransmissionGroupMembership_t individual, float dt) override
        {
            //transmissionGroups->ExposeToContagion(candidate, individual, dt);
            if( my_callback != nullptr )
            {
                PyObject *arglist = Py_BuildValue("(s,f,i)", "expose", 1.0, dynamic_cast<IIndividualHuman*>(candidate)->GetSuid().data );
                PyObject *retVal = PyObject_CallObject(my_callback, arglist);
                bool infect = false;
                infect = bool(PyLong_AsLong( retVal ));
                if( infect )
                {
                    IInfectionAcquirable* ind = nullptr;
                    candidate->QueryInterface( GET_IID( IInfectionAcquirable ), reinterpret_cast<void**>(&ind) );
                    ind->AcquireNewInfection();
                }
                Py_DECREF(arglist);
            }
        }

        virtual float initiatePregnancyForIndividual( int individual_id, float dt ) override
        {
            std::cout << "StubNode::initiatePregnancyForIndividual: individual_id = " << individual_id << std::endl;
            abort(); // ???
        }
        virtual bool updatePregnancyForIndividual( int individual_id, float duration ) override
        {
            std::cout << "StubNode::updatePregnancyForIndividual: individual_id = " << individual_id << std::endl;
            abort(); // ???
        }
        virtual void populateNewIndividualsByBirth(int count_new_individuals = 100) override
        {
            std::cout << "StubNode::populateNewIndividualsByBirth: num_individuals = " << count_new_individuals  << std::endl;
            abort(); // ???
        }
        
        static PyObject *my_callback;
        static PyObject *mortality_callback;
        static PyObject *deposit_callback;

    private:
        NPKeyValueContainer node_properties;
        RANDOMBASE* m_pRNG;
        RouteList_t m_RouteList;
        Kernel::suids::suid m_NodeId;
};
