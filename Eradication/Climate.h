/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include "stdafx.h"
#include <fstream>
#include <math.h>
#include <unordered_map>

#include "Sugar.h"
#include "SimulationEnums.h"
#include "Configure.h"
#include "ExternalNodeId.h"

#ifdef __GNUC__
namespace std
{
     using namespace __gnu_cxx;
}
#endif

#define PI 3.141593

namespace Kernel
{
    class RANDOMBASE;
    struct ClimateParams;
    struct INodeContext;
    struct ISimulationContext;

    class Climate
    {
    public:
        inline float airtemperature()       const { return m_airtemperature; }
        inline float landtemperature()      const { return m_landtemperature; }
        inline float accumulated_rainfall() const { return m_accumulated_rainfall; }
        inline float humidity()             const { return m_humidity; }

    protected:
        float m_airtemperature;
        float m_landtemperature;
        float m_accumulated_rainfall;
        float m_humidity;
        float resolution_correction;

        static const float min_airtemp;
        static const float max_airtemp;
        static const float min_landtemp;
        static const float max_landtemp;
        static const float max_rainfall;

        INodeContext * parent;

    public:
        virtual ~Climate();
        void SetContextTo(INodeContext* _parent);

        //  Updates weather based on the time step.  If the time step is long, then it adjusts.
        //  For instance, is rainfall over an hour or over a week?
        virtual void UpdateWeather( float time, float dt, RANDOMBASE* pRNG );

    protected:
        friend class ClimateFactory;

        Climate(ClimateUpdateResolution::Enum update_resolution = ClimateUpdateResolution::CLIMATE_UPDATE_DAY, INodeContext * _parent = nullptr);

        virtual void AddStochasticity( RANDOMBASE* pRNG );

        virtual bool IsPlausible() = 0;
    };


    class ClimateFactory
    {
    public:

        static ClimateFactory* CreateClimateFactory(const std::string idreference, ISimulationContext* parent_sim);
        ~ClimateFactory();

        const ClimateParams* GetParams();

        Climate* CreateClimate( INodeContext *parent_node, float altitude, float latitude, RANDOMBASE* pRNG );

    private:
        ClimateFactory(ISimulationContext* parent_sim);
        bool Initialize(const std::string idreference);
        bool ParseMetadataForFile(std::string data_filepath, std::string idreference, int * const num_datavalues, int * const num_nodes, std::unordered_map<uint32_t, uint32_t> &node_offsets);
        bool OpenClimateFile(std::string filepath, uint32_t expected_size, std::ifstream &file);

        ISimulationContext* parent;

        // data file stream handles for ClimateByData
        std::ifstream climate_landtemperature_file;
        std::ifstream climate_airtemperature_file;
        std::ifstream climate_rainfall_file;
        std::ifstream climate_humidity_file;

        // data file stream handle for ClimateKoppen
        std::ifstream climate_koppentype_file;

        // node offsets in ClimateByData files
        std::unordered_map<uint32_t, uint32_t> landtemperature_offsets;
        std::unordered_map<uint32_t, uint32_t> airtemperature_offsets;
        std::unordered_map<uint32_t, uint32_t> rainfall_offsets;
        std::unordered_map<uint32_t, uint32_t> humidity_offsets;

        // node offsets in ClimateKoppen file
        std::unordered_map<uint32_t, uint32_t> koppentype_offsets;

        // metadata info
        int num_datavalues;
        int num_nodes;

        int num_badnodes;
    };
}
