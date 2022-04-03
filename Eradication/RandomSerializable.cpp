#include "stdafx.h"

#include "RandomSerializable.h"
#include "IArchive.h"

namespace Kernel
{
    // static function, only function in class
    void RANDOMBASE_SER::serialize( IArchive& ar, RANDOMBASE* obj )
    {
        RANDOMBASE& rb = *obj;
        ar.labelElement( "cache_count"   ) & rb.cache_count;

        if( ar.IsReader() )
        {
            free( rb.random_bits );
            free( rb.random_floats );

            rb.random_bits   = reinterpret_cast<uint32_t*>(malloc( rb.cache_count * sizeof( uint32_t ) ));
            rb.random_floats = reinterpret_cast<float*   >(malloc( rb.cache_count * sizeof( float    ) ));
        }

        ar.labelElement( "index"         ) & rb.index;
        ar.labelElement( "random_bits"   ); ar.serialize( rb.random_bits, rb.cache_count );
        ar.labelElement( "random_floats" ); ar.serialize( rb.random_floats, rb.cache_count );
        ar.labelElement( "bGauss"        ) & rb.bGauss;
        ar.labelElement( "eGauss_"       ) & rb.eGauss_;
    }

    // ----------------------------------------------------------------------------
    // --- LINEAR_CONGRUENTIAL
    // ----------------------------------------------------------------------------
    BEGIN_QUERY_INTERFACE_BODY( LINEAR_CONGRUENTIAL_SER )
    END_QUERY_INTERFACE_BODY( LINEAR_CONGRUENTIAL_SER )

    REGISTER_SERIALIZABLE( LINEAR_CONGRUENTIAL_SER );

    LINEAR_CONGRUENTIAL_SER::LINEAR_CONGRUENTIAL_SER( uint32_t iSequence, size_t nCache )
    : LINEAR_CONGRUENTIAL( iSequence, nCache )
    {
    }

    LINEAR_CONGRUENTIAL_SER::~LINEAR_CONGRUENTIAL_SER()
    {
    }

    void LINEAR_CONGRUENTIAL_SER::serialize( Kernel::IArchive& ar, LINEAR_CONGRUENTIAL_SER* obj )
    {
        RANDOMBASE_SER::serialize( ar, obj );
        LINEAR_CONGRUENTIAL_SER& lc = *obj;
        ar.labelElement( "iSeq" ) & lc.iSeq;
    }

    // ----------------------------------------------------------------------------
    // --- PSEUDO_DES
    // ----------------------------------------------------------------------------
    BEGIN_QUERY_INTERFACE_BODY( PSEUDO_DES_SER )
    END_QUERY_INTERFACE_BODY( PSEUDO_DES_SER )

    REGISTER_SERIALIZABLE( PSEUDO_DES_SER );

    PSEUDO_DES_SER::PSEUDO_DES_SER( uint64_t iSequence, size_t nCache )
    : PSEUDO_DES( iSequence, nCache )
    {
    }

    PSEUDO_DES_SER::~PSEUDO_DES_SER()
    {
    }

    void PSEUDO_DES_SER::serialize( Kernel::IArchive& ar, PSEUDO_DES_SER* obj )
    {
        RANDOMBASE_SER::serialize( ar, obj );
        PSEUDO_DES_SER& des = *obj;
        ar.labelElement( "iSeq" ) & des.iSeq;
        ar.labelElement( "iNum" ) & des.iNum;
    }

    // ----------------------------------------------------------------------------
    // --- AES_COUNTER
    // ----------------------------------------------------------------------------
    BEGIN_QUERY_INTERFACE_BODY( AES_COUNTER_SER )
    END_QUERY_INTERFACE_BODY( AES_COUNTER_SER )

    REGISTER_SERIALIZABLE( AES_COUNTER_SER );

    AES_COUNTER_SER::AES_COUNTER_SER( uint64_t iSequence, size_t nCache )
    : AES_COUNTER( iSequence, nCache )
    {
    }

    AES_COUNTER_SER::~AES_COUNTER_SER()
    {
    }

    void serialize_AES_KEY( Kernel::IArchive& ar, AES_KEY& key )
    {
        size_t num_keys = 2 * NUM_KEYS_IN_SCHEDULE;

        ar.startObject();
        ar.labelElement("KEY");
        ar.startArray( num_keys );
        uint64_t* vals = (uint64_t*)(key.KEY);
        for( size_t i = 0; i < num_keys; ++i )
        {
            ar & vals[i];
        }
        ar.endArray();
        ar.labelElement("nr") & key.nr;
        ar.endObject();
    }

    void AES_COUNTER_SER::serialize( Kernel::IArchive& ar, AES_COUNTER_SER* obj )
    {
        RANDOMBASE_SER::serialize( ar, obj );
        AES_COUNTER_SER& aes = *obj;
        ar.labelElement( "m_keySchedule" ); serialize_AES_KEY( ar, aes.m_keySchedule );
        ar.labelElement( "m_nonce"     ) & aes.m_nonce;
        ar.labelElement( "m_iteration" ) & aes.m_iteration;
    }

}
