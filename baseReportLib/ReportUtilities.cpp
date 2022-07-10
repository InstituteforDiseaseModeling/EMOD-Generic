/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"

#include "Log.h"
#include "IIndividualHuman.h"
#include "Interventions.h"
#include "ReportUtilities.h"
#include "Environment.h"
#include "IdmMpi.h"

SETUP_LOGGING( "ReportUtilities" )

using namespace Kernel;

namespace ReportUtilities
{
    // ----------------------
    // --- Local access only
    // ----------------------
    void InternalDeserializeVector( const json::QuickInterpreter& q_int, bool isSettingValuesInVector, std::vector<std::string>& rData )
    {
        const json::Array& json_array = q_int.As<json::Array>();
        if( isSettingValuesInVector && (rData.size() != json_array.Size()) )
        {
            std::stringstream ss;
            ss << "Cannot deserialize json into 1D-array because they are not the same size.  vector.size=" << rData.size() << "  json_size=" << json_array.Size();
            throw IllegalOperationException( __FILE__, __LINE__, __FUNCTION__, ss.str().c_str() );
        }

        for( unsigned int i = 0; i < json_array.Size(); ++i )
        {
            std::string value = (q_int[i]).As<json::String>();
            if( isSettingValuesInVector )
            {
                rData[ i ] = value;
            }
            else
            {
                rData.push_back( value );
            }
        }
    }

    void InternalDeserializeVector( const json::QuickInterpreter&& q_int, bool isSettingValuesInVector, std::vector<double>& rData )
    {
        const json::Array& json_array = q_int.As<json::Array>();
        if( isSettingValuesInVector && (rData.size() != json_array.Size()) )
        {
            std::stringstream ss;
            ss << "Cannot deserialize json into 1D-array because they are not the same size.  vector.size=" << rData.size() << "  json_size=" << json_array.Size();
            throw IllegalOperationException( __FILE__, __LINE__, __FUNCTION__, ss.str().c_str() );
        }

        for( unsigned int i = 0; i < json_array.Size(); ++i )
        {
            double value = static_cast<double>((q_int[i]).As<json::Number>());
            if( isSettingValuesInVector )
            {
                rData[ i ] = value;
            }
            else
            {
                rData.push_back( value );
            }
        }
    }

    void InternalDeserializeVector( const json::QuickInterpreter&& q_int, bool isSettingValuesInVector, std::vector<std::vector<double>>& rData )
    {
        const json::Array& json_array = q_int.As<json::Array>();
        if( isSettingValuesInVector && (rData.size() != json_array.Size()) )
        {
            std::stringstream ss;
            ss << "Cannot deserialize json into 2D-array because they are not the same size.  vector.size=" << rData.size() << "  json_size=" << json_array.Size();
            throw IllegalOperationException( __FILE__, __LINE__, __FUNCTION__, ss.str().c_str() );
        }

        for( unsigned int i = 0; i < json_array.Size(); ++i )
        {
            InternalDeserializeVector( json_array[i], isSettingValuesInVector, rData[ i ] );
        }
    }

    void InternalDeserializeVector( const json::QuickInterpreter&& q_int, bool isSettingValuesInVector, std::vector<std::vector<std::vector<double>>>& rData )
    {
        const json::Array& json_array = q_int.As<json::Array>();
        if( isSettingValuesInVector && (rData.size() != json_array.Size()) )
        {
            std::stringstream ss;
            ss << "Cannot deserialize json into 3D-array because they are not the same size.  vector.size=" << rData.size() << "  json_size=" << json_array.Size();
            throw IllegalOperationException( __FILE__, __LINE__, __FUNCTION__, ss.str().c_str() );
        }

        for( unsigned int i = 0; i < json_array.Size(); ++i )
        {
            InternalDeserializeVector( json_array[i], isSettingValuesInVector, rData[ i ] );
        }
    }

    void InternalDeserializeVector( const json::QuickInterpreter& q_int, bool isSettingValuesInVector, std::vector<std::vector<std::vector<std::vector<double>>>>& rData )
    {
        const json::Array& json_array = q_int.As<json::Array>();
        if( isSettingValuesInVector && (rData.size() != json_array.Size()) )
        {
            std::stringstream ss;
            ss << "Cannot deserialize json into 4D-array because they are not the same size.  vector.size=" << rData.size() << "  json_size=" << json_array.Size();
            throw IllegalOperationException( __FILE__, __LINE__, __FUNCTION__, ss.str().c_str() );
        }

        for( unsigned int i = 0; i < json_array.Size(); ++i )
        {
            InternalDeserializeVector( json_array[i], isSettingValuesInVector, rData[ i ] );
        }
    }

    // ---------------------
    // --- Public namespace
    // ---------------------
    int GetBinIndex( float val, std::vector<float>& rValues )
    {
        if( val > rValues.back() )
        {
            return rValues.size() - 1;
        }
        else
        {
            vector<float>::const_iterator it;
            it = std::lower_bound( rValues.begin(), rValues.end(), val );
            int index = it - rValues.begin();
            return index;
        }
    }

    int GetAgeBin( float age, std::vector<float>& rAges )
    {
        float age_years = age / DAYSPERYEAR ;
        return GetBinIndex( age_years, rAges );
    }

    void SendData( const std::string& rToSend )
    {
        uint32_t size = rToSend.size();

        IdmMpi::Request size_request;
        EnvPtr->MPI.p_idm_mpi->SendIntegers( &size, 1, 0, &size_request );


        IdmMpi::Request data_request;
        EnvPtr->MPI.p_idm_mpi->SendChars( rToSend.c_str(), size, 0, &data_request );

        IdmMpi::RequestList request_list;
        request_list.Add( size_request );
        request_list.Add( data_request );

        EnvPtr->MPI.p_idm_mpi->WaitAll( request_list );
    }

    void GetData( int fromRank, std::vector<char>& rReceive )
    {
        int32_t size = 0;
        EnvPtr->MPI.p_idm_mpi->ReceiveIntegers( &size, 1, fromRank );

        rReceive.resize( size + 1 );

        EnvPtr->MPI.p_idm_mpi->ReceiveChars( rReceive.data(), size, fromRank );
        rReceive[size] = '\0';
    }

    void SerializeVector(json::Object& root, const char* pName, std::vector<std::string>& rData )
    {
        json::QuickBuilder json_root(root);

        json::Array        arr_dim1;
        for(size_t k1 = 0; k1 < rData.size(); k1++)
        {
            arr_dim1.Insert(json::String(rData[k1]));
        }
        json_root[pName] = arr_dim1;
    }

    void SerializeVector(json::Object& root, const char* pName, std::vector<float>& rData )
    {
        json::QuickBuilder json_root(root);

        json::Array        arr_dim1;
        for(size_t k1 = 0; k1 < rData.size(); k1++)
        {
            arr_dim1.Insert(json::Number(rData[k1]));
        }
        json_root[pName] = arr_dim1;
    }

    void SerializeVector(json::Object& root, const char* pName, std::vector<double>& rData )
    {
        json::QuickBuilder json_root(root);

        json::Array        arr_dim1;
        for(size_t k1 = 0; k1 < rData.size(); k1++)
        {
            arr_dim1.Insert(json::Number(rData[k1]));
        }
        json_root[pName] = arr_dim1;
    }

    void SerializeVector(json::Object& root, const char* pName, std::vector<std::vector<double>>& rData )
    {
        json::QuickBuilder json_root(root);

        json::Array        arr_dim1;
        for(size_t k1 = 0; k1 < rData.size(); k1++)
        {
            json::Array        arr_dim2;
            for(size_t k2 = 0; k2 < rData[k1].size(); k2++)
            {
                arr_dim2.Insert(json::Number(rData[k1][k2]));
            }
            arr_dim1.Insert(arr_dim2);
        }
        json_root[pName] = arr_dim1;
    }

    void SerializeVector(json::Object& root, const char* pName, std::vector<std::vector<std::vector<double>>>& rData )
    {
        json::QuickBuilder json_root(root);

        json::Array        arr_dim1;
        for(size_t k1 = 0; k1 < rData.size(); k1++)
        {
            json::Array        arr_dim2;
            for(size_t k2 = 0; k2 < rData[k1].size(); k2++)
            {
                json::Array        arr_dim3;
                for(size_t k3 = 0; k3 < rData[k1][k2].size(); k3++)
                {
                    arr_dim3.Insert(json::Number(rData[k1][k2][k3]));
                }
                arr_dim2.Insert(arr_dim3);
            }
            arr_dim1.Insert(arr_dim2);
        }
        json_root[pName] = arr_dim1;
    }

    void SerializeVector( json::Object& root, const char* pName, std::vector<std::vector<std::vector<std::vector<double>>>>& rData )
    {
        json::QuickBuilder json_root(root);

        json::Array        arr_dim1;
        for(size_t k1 = 0; k1 < rData.size(); k1++)
        {
            json::Array        arr_dim2;
            for(size_t k2 = 0; k2 < rData[k1].size(); k2++)
            {
                json::Array        arr_dim3;
                for(size_t k3 = 0; k3 < rData[k1][k2].size(); k3++)
                {
                    json::Array        arr_dim4;
                    for(size_t k4 = 0; k4 < rData[k1][k2][k3].size(); k4++)
                    {
                        arr_dim4.Insert(json::Number(rData[k1][k2][k3][k4]));
                    }
                    arr_dim3.Insert(arr_dim4);
                }
                arr_dim2.Insert(arr_dim3);
            }
            arr_dim1.Insert(arr_dim2);
        }
        json_root[pName] = arr_dim1;
    }

    void DeserializeVector( json::Object& root, bool isSettingValuesInVector, const char* pName, std::vector<std::string>& rData )
    {
        json::QuickInterpreter q_int(root);
        InternalDeserializeVector( q_int[pName], isSettingValuesInVector, rData );
    }

    void DeserializeVector( json::Object& root, bool isSettingValuesInVector, const char* pName, std::vector<double>& rData )
    {
        json::QuickInterpreter q_int(root);
        InternalDeserializeVector( q_int[pName], isSettingValuesInVector, rData );
    }

    void DeserializeVector( json::Object& root, bool isSettingValuesInVector, const char* pName, std::vector<std::vector<double>>& rData )
    {
        json::QuickInterpreter q_int(root);
        InternalDeserializeVector( q_int[pName], isSettingValuesInVector, rData );
    }

    void DeserializeVector( json::Object& root, bool isSettingValuesInVector, const char* pName, std::vector<std::vector<std::vector<double>>>& rData )
    {
        json::QuickInterpreter q_int(root);
        InternalDeserializeVector( q_int[pName], isSettingValuesInVector, rData );
    }

    void DeserializeVector( json::Object& root, bool isSettingValuesInVector, const char* pName, std::vector<std::vector<std::vector<std::vector<double>>>>& rData )
    {
        json::QuickInterpreter q_int(root);
        InternalDeserializeVector( q_int[pName], isSettingValuesInVector, rData );
    }

    void AddVector( std::vector<double>& rThis, const std::vector<double>& rThat )
    {
        release_assert( rThis.size() ==rThat.size() );

        for( int i = 0 ; i < rThis.size(); ++i )
        {
            rThis[i] += rThat.at(i);
        }
    }

    void AddVector( std::vector<std::vector<double>>& rThis, const std::vector<std::vector<double>>& rThat )
    {
        release_assert( rThis.size() == rThat.size() );

        for( int i = 0; i < rThis.size(); ++i )
        {
            AddVector( rThis[ i ], rThat[ i ] );
        }
    }

    void AddVector( std::vector<std::vector<std::vector<double>>>& rThis, const std::vector<std::vector<std::vector<double>>>& rThat)
    {
        release_assert(rThis.size() == rThat.size());

        for (int i = 0; i < rThis.size(); ++i)
        {
            AddVector( rThis[ i ], rThat[ i ] );
        }
    }

    void AddVector( std::vector<std::vector<std::vector<std::vector<double>>>>& rThis, const std::vector<std::vector<std::vector<std::vector<double>>>>& rThat )
    {
        release_assert( rThis.size() == rThat.size() );

        for( int i = 0; i < rThis.size(); ++i )
        {
            AddVector( rThis[ i ], rThat[ i ] );
        }
    }

    std::string GetIPColumnHeader( const char* pColumnPrefix,
                                   const Kernel::jsonConfigurable::tDynamicStringSet& rPropertiesToReport )
    {
        std::stringstream header;
        for( const auto& prop : rPropertiesToReport )
        {
            header << "," << pColumnPrefix << prop;
        }
        return header.str();
    }

    std::string GetIPData( const Kernel::IPKeyValueContainer& rContainer,
                           const std::vector<Kernel::IPKey>& rKeysToReport )
    {
        std::stringstream header;
        for( const auto& key : rKeysToReport )
        {
            header << "," << rContainer.Get( key ).GetValueAsString();
        }
        return header.str();
    }

    std::vector<Kernel::IPKey> GetKeys( const Kernel::jsonConfigurable::tDynamicStringSet& rPropertiesToReport,
                                        const char* pParamName )
    {
        std::vector<IPKey> keys_to_report;
        for( auto key_name : rPropertiesToReport )
        {
            IndividualProperty* p_ip = IPFactory::GetInstance()->GetIP( key_name, pParamName, false );
            if( p_ip == nullptr )
            {
                std::stringstream ss;
                ss << "The IP Key (" << key_name << ") specified in '" << pParamName << "' is unknown.\n"
                    << "Valid values are: " << IPFactory::GetInstance()->GetKeysAsString();
                throw GeneralConfigurationException( __FILE__, __LINE__, __FUNCTION__, ss.str().c_str() );
            }
            keys_to_report.push_back( p_ip->GetKey<IPKey>() );
        }
        return keys_to_report;
    }
}