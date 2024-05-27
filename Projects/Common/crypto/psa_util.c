/* Functions have been copied from :
 * mbedts/include/psa_util.h
 * mbedtls/library/pk_wrap.c
 */

/*
 *  Copyright The Mbed TLS Contributors
 *  SPDX-License-Identifier: Apache-2.0
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may
 *  not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 *  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include "tls_transport_config.h"

#ifdef MBEDTLS_TRANSPORT_PSA

#include <string.h>


#include "mbedtls/pk.h"
#include "mbedtls/error.h"
#include "mbedtls/oid.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/asn1write.h"

#include "psa/crypto.h"
#include "psa/client.h"
#include "tfm_ns_interface.h"
#include "tfm_crypto_defs.h"

/*-----------------------------------------------------------*/

int mbedtls_psa_err_translate_pk( psa_status_t status )
{
    switch( status )
    {
        case PSA_SUCCESS:
            return( 0 );

        case PSA_ERROR_NOT_SUPPORTED:
            return( MBEDTLS_ERR_PK_FEATURE_UNAVAILABLE );

        case PSA_ERROR_INSUFFICIENT_MEMORY:
            return( MBEDTLS_ERR_PK_ALLOC_FAILED );

        case PSA_ERROR_INSUFFICIENT_ENTROPY:
            return( MBEDTLS_ERR_ECP_RANDOM_FAILED );

        case PSA_ERROR_BAD_STATE:
            return( MBEDTLS_ERR_PK_BAD_INPUT_DATA );

        /* All other failures */
        case PSA_ERROR_COMMUNICATION_FAILURE:
        case PSA_ERROR_HARDWARE_FAILURE:
        case PSA_ERROR_CORRUPTION_DETECTED:
            return( MBEDTLS_ERR_PLATFORM_HW_ACCEL_FAILED );

        default: /* We return the same as for the 'other failures',
                  * but list them separately nonetheless to indicate
                  * which failure conditions we have considered. */
            return( MBEDTLS_ERR_PLATFORM_HW_ACCEL_FAILED );
    }
}

/*-----------------------------------------------------------*/

int mbedtls_psa_get_ecc_oid_from_id( psa_ecc_family_t curve,
                                     size_t bits,
                                     char const ** oid,
                                     size_t * oid_len )
{
    switch( curve )
    {
        case PSA_ECC_FAMILY_SECP_R1:

            switch( bits )
            {
#if defined( MBEDTLS_ECP_DP_SECP192R1_ENABLED )
                case 192:
                    *oid = MBEDTLS_OID_EC_GRP_SECP192R1;
                    *oid_len = MBEDTLS_OID_SIZE( MBEDTLS_OID_EC_GRP_SECP192R1 );
                    return( 0 );
#endif /* MBEDTLS_ECP_DP_SECP192R1_ENABLED */
#if defined( MBEDTLS_ECP_DP_SECP224R1_ENABLED )
                case 224:
                    *oid = MBEDTLS_OID_EC_GRP_SECP224R1;
                    *oid_len = MBEDTLS_OID_SIZE( MBEDTLS_OID_EC_GRP_SECP224R1 );
                    return( 0 );
#endif /* MBEDTLS_ECP_DP_SECP224R1_ENABLED */
#if defined( MBEDTLS_ECP_DP_SECP256R1_ENABLED )
                case 256:
                    *oid = MBEDTLS_OID_EC_GRP_SECP256R1;
                    *oid_len = MBEDTLS_OID_SIZE( MBEDTLS_OID_EC_GRP_SECP256R1 );
                    return( 0 );
#endif /* MBEDTLS_ECP_DP_SECP256R1_ENABLED */
#if defined( MBEDTLS_ECP_DP_SECP384R1_ENABLED )
                case 384:
                    *oid = MBEDTLS_OID_EC_GRP_SECP384R1;
                    *oid_len = MBEDTLS_OID_SIZE( MBEDTLS_OID_EC_GRP_SECP384R1 );
                    return( 0 );
#endif /* MBEDTLS_ECP_DP_SECP384R1_ENABLED */
#if defined( MBEDTLS_ECP_DP_SECP521R1_ENABLED )
                case 521:
                    *oid = MBEDTLS_OID_EC_GRP_SECP521R1;
                    *oid_len = MBEDTLS_OID_SIZE( MBEDTLS_OID_EC_GRP_SECP521R1 );
                    return( 0 );
#endif /* MBEDTLS_ECP_DP_SECP521R1_ENABLED */
            }

            break;

        case PSA_ECC_FAMILY_SECP_K1:

            switch( bits )
            {
#if defined( MBEDTLS_ECP_DP_SECP192K1_ENABLED )
                case 192:
                    *oid = MBEDTLS_OID_EC_GRP_SECP192K1;
                    *oid_len = MBEDTLS_OID_SIZE( MBEDTLS_OID_EC_GRP_SECP192K1 );
                    return( 0 );
#endif /* MBEDTLS_ECP_DP_SECP192K1_ENABLED */
#if defined( MBEDTLS_ECP_DP_SECP224K1_ENABLED )
                case 224:
                    *oid = MBEDTLS_OID_EC_GRP_SECP224K1;
                    *oid_len = MBEDTLS_OID_SIZE( MBEDTLS_OID_EC_GRP_SECP224K1 );
                    return( 0 );
#endif /* MBEDTLS_ECP_DP_SECP224K1_ENABLED */
#if defined( MBEDTLS_ECP_DP_SECP256K1_ENABLED )
                case 256:
                    *oid = MBEDTLS_OID_EC_GRP_SECP256K1;
                    *oid_len = MBEDTLS_OID_SIZE( MBEDTLS_OID_EC_GRP_SECP256K1 );
                    return( 0 );
#endif /* MBEDTLS_ECP_DP_SECP256K1_ENABLED */
            }

            break;

        case PSA_ECC_FAMILY_BRAINPOOL_P_R1:

            switch( bits )
            {
#if defined( MBEDTLS_ECP_DP_BP256R1_ENABLED )
                case 256:
                    *oid = MBEDTLS_OID_EC_GRP_BP256R1;
                    *oid_len = MBEDTLS_OID_SIZE( MBEDTLS_OID_EC_GRP_BP256R1 );
                    return( 0 );
#endif /* MBEDTLS_ECP_DP_BP256R1_ENABLED */
#if defined( MBEDTLS_ECP_DP_BP384R1_ENABLED )
                case 384:
                    *oid = MBEDTLS_OID_EC_GRP_BP384R1;
                    *oid_len = MBEDTLS_OID_SIZE( MBEDTLS_OID_EC_GRP_BP384R1 );
                    return( 0 );
#endif /* MBEDTLS_ECP_DP_BP384R1_ENABLED */
#if defined( MBEDTLS_ECP_DP_BP512R1_ENABLED )
                case 512:
                    *oid = MBEDTLS_OID_EC_GRP_BP512R1;
                    *oid_len = MBEDTLS_OID_SIZE( MBEDTLS_OID_EC_GRP_BP512R1 );
                    return( 0 );
#endif /* MBEDTLS_ECP_DP_BP512R1_ENABLED */
            }

            break;
    }

    ( void ) oid;
    ( void ) oid_len;
    return( -1 );
}

/*-----------------------------------------------------------*/

mbedtls_ecp_group_id xMbedtlsEccGroupIdFromPsaFamily( psa_ecc_family_t curve,
                                                      size_t bits )
{
    mbedtls_ecp_group_id xGroupID = MBEDTLS_ECP_DP_NONE;

    switch( curve )
    {
        case PSA_ECC_FAMILY_SECP_R1:

            switch( bits )
            {
                case 192:
                    xGroupID = MBEDTLS_ECP_DP_SECP192R1;
                    break;

                case 224:
                    xGroupID = MBEDTLS_ECP_DP_SECP224R1;
                    break;

                case 256:
                    xGroupID = MBEDTLS_ECP_DP_SECP256R1;
                    break;

                case 384:
                    xGroupID = MBEDTLS_ECP_DP_SECP384R1;
                    break;

                case 521:
                case 528:
                    xGroupID = MBEDTLS_ECP_DP_SECP521R1;
                    break;

                default:
                    break;
            }

            break;

        case PSA_ECC_FAMILY_BRAINPOOL_P_R1:

            switch( bits )
            {
                case 256:
                    xGroupID = MBEDTLS_ECP_DP_BP256R1;
                    break;

                case 384:
                    xGroupID = MBEDTLS_ECP_DP_BP384R1;
                    break;

                case 512:
                    xGroupID = MBEDTLS_ECP_DP_BP512R1;
                    break;

                default:
                    break;
            }

            break;

        case PSA_ECC_FAMILY_MONTGOMERY:

            switch( bits )
            {
                case 255:
                case 256:
                    xGroupID = MBEDTLS_ECP_DP_CURVE25519;
                    break;

                case 448:
                    xGroupID = MBEDTLS_ECP_DP_CURVE448;
                    break;

                default:
                    break;
            }

            break;

        case PSA_ECC_FAMILY_SECP_K1:

            switch( bits )
            {
                case 192:
                    xGroupID = MBEDTLS_ECP_DP_SECP192K1;
                    break;

                case 224:
                    xGroupID = MBEDTLS_ECP_DP_SECP224K1;
                    break;

                case 256:
                    xGroupID = MBEDTLS_ECP_DP_SECP256K1;
                    break;

                default:
                    break;
            }

            break;

        default:
            break;
    }

    return xGroupID;
}

/*-----------------------------------------------------------*/

psa_ecc_family_t xPsaFamilyFromMbedtlsEccGroupId( mbedtls_ecp_group_id xGroupId )
{
    size_t uxBits;
    psa_ecc_family_t xFamily;

    switch( xGroupId )
    {
        case MBEDTLS_ECP_DP_SECP192R1:
        case MBEDTLS_ECP_DP_SECP224R1:
        case MBEDTLS_ECP_DP_SECP256R1:
        case MBEDTLS_ECP_DP_SECP384R1:
        case MBEDTLS_ECP_DP_SECP521R1:
            xFamily = PSA_ECC_FAMILY_SECP_R1;
            break;

        case MBEDTLS_ECP_DP_BP256R1:
        case MBEDTLS_ECP_DP_BP384R1:
        case MBEDTLS_ECP_DP_BP512R1:
            xFamily = PSA_ECC_FAMILY_BRAINPOOL_P_R1;
            break;

        case MBEDTLS_ECP_DP_CURVE25519:
        case MBEDTLS_ECP_DP_CURVE448:
            xFamily = PSA_ECC_FAMILY_MONTGOMERY;
            break;

        case MBEDTLS_ECP_DP_SECP192K1:
        case MBEDTLS_ECP_DP_SECP224K1:
        case MBEDTLS_ECP_DP_SECP256K1:
            xFamily = PSA_ECC_FAMILY_SECP_K1;
            break;

        default:
            xFamily = 0;
            break;
    }

    return xFamily;
}

/*-----------------------------------------------------------*/

psa_status_t mbedtls_to_psa_error( int ret )
{
    /* Mbed TLS error codes can combine a high-level error code and a
     * low-level error code. The low-level error usually reflects the
     * root cause better, so dispatch on that preferably. */
    int low_level_ret = -( -ret & 0x007f );

    switch( ( low_level_ret != 0 ) ? low_level_ret : ret )
    {
        case 0:
            return( PSA_SUCCESS );

        case MBEDTLS_ERR_AES_INVALID_KEY_LENGTH:
        case MBEDTLS_ERR_AES_INVALID_INPUT_LENGTH:
            return( PSA_ERROR_NOT_SUPPORTED );

        case MBEDTLS_ERR_ASN1_OUT_OF_DATA:
        case MBEDTLS_ERR_ASN1_UNEXPECTED_TAG:
        case MBEDTLS_ERR_ASN1_INVALID_LENGTH:
        case MBEDTLS_ERR_ASN1_LENGTH_MISMATCH:
        case MBEDTLS_ERR_ASN1_INVALID_DATA:
            return( PSA_ERROR_INVALID_ARGUMENT );

        case MBEDTLS_ERR_ASN1_ALLOC_FAILED:
            return( PSA_ERROR_INSUFFICIENT_MEMORY );

        case MBEDTLS_ERR_ASN1_BUF_TOO_SMALL:
            return( PSA_ERROR_BUFFER_TOO_SMALL );

#if defined( MBEDTLS_ERR_CAMELLIA_BAD_INPUT_DATA )
        case MBEDTLS_ERR_CAMELLIA_BAD_INPUT_DATA:
#endif

        case MBEDTLS_ERR_CIPHER_FEATURE_UNAVAILABLE:
            return( PSA_ERROR_NOT_SUPPORTED );

        case MBEDTLS_ERR_CIPHER_BAD_INPUT_DATA:
            return( PSA_ERROR_INVALID_ARGUMENT );

        case MBEDTLS_ERR_CIPHER_ALLOC_FAILED:
            return( PSA_ERROR_INSUFFICIENT_MEMORY );

        case MBEDTLS_ERR_CIPHER_INVALID_PADDING:
            return( PSA_ERROR_INVALID_PADDING );

        case MBEDTLS_ERR_CIPHER_FULL_BLOCK_EXPECTED:
            return( PSA_ERROR_INVALID_ARGUMENT );

        case MBEDTLS_ERR_CIPHER_AUTH_FAILED:
            return( PSA_ERROR_INVALID_SIGNATURE );

        case MBEDTLS_ERR_CIPHER_INVALID_CONTEXT:
            return( PSA_ERROR_CORRUPTION_DETECTED );

#if !( defined( MBEDTLS_PSA_CRYPTO_EXTERNAL_RNG ) || defined( MBEDTLS_PSA_HMAC_DRBG_MD_TYPE ) )
        /* Only check CTR_DRBG error codes if underlying mbedtls_xxx
         * functions are passed a CTR_DRBG instance. */

        case MBEDTLS_ERR_CTR_DRBG_ENTROPY_SOURCE_FAILED:
            return( PSA_ERROR_INSUFFICIENT_ENTROPY );

        case MBEDTLS_ERR_CTR_DRBG_REQUEST_TOO_BIG:
        case MBEDTLS_ERR_CTR_DRBG_INPUT_TOO_BIG:
            return( PSA_ERROR_NOT_SUPPORTED );

        case MBEDTLS_ERR_CTR_DRBG_FILE_IO_ERROR:
            return( PSA_ERROR_INSUFFICIENT_ENTROPY );
#endif /* if !( defined( MBEDTLS_PSA_CRYPTO_EXTERNAL_RNG ) || defined( MBEDTLS_PSA_HMAC_DRBG_MD_TYPE ) ) */

        case MBEDTLS_ERR_ENTROPY_NO_SOURCES_DEFINED:
        case MBEDTLS_ERR_ENTROPY_NO_STRONG_SOURCE:
        case MBEDTLS_ERR_ENTROPY_SOURCE_FAILED:
            return( PSA_ERROR_INSUFFICIENT_ENTROPY );

#if !defined( MBEDTLS_PSA_CRYPTO_EXTERNAL_RNG ) && \
            defined( MBEDTLS_PSA_HMAC_DRBG_MD_TYPE )
        /* Only check HMAC_DRBG error codes if underlying mbedtls_xxx
         * functions are passed a HMAC_DRBG instance. */
        case MBEDTLS_ERR_HMAC_DRBG_ENTROPY_SOURCE_FAILED:
            return( PSA_ERROR_INSUFFICIENT_ENTROPY );

        case MBEDTLS_ERR_HMAC_DRBG_REQUEST_TOO_BIG:
        case MBEDTLS_ERR_HMAC_DRBG_INPUT_TOO_BIG:
            return( PSA_ERROR_NOT_SUPPORTED );

        case MBEDTLS_ERR_HMAC_DRBG_FILE_IO_ERROR:
            return( PSA_ERROR_INSUFFICIENT_ENTROPY );
#endif /* if !defined( MBEDTLS_PSA_CRYPTO_EXTERNAL_RNG ) && defined( MBEDTLS_PSA_HMAC_DRBG_MD_TYPE ) */

        case MBEDTLS_ERR_MD_FEATURE_UNAVAILABLE:
            return( PSA_ERROR_NOT_SUPPORTED );

        case MBEDTLS_ERR_MD_BAD_INPUT_DATA:
            return( PSA_ERROR_INVALID_ARGUMENT );

        case MBEDTLS_ERR_MD_ALLOC_FAILED:
            return( PSA_ERROR_INSUFFICIENT_MEMORY );

        case MBEDTLS_ERR_MD_FILE_IO_ERROR:
            return( PSA_ERROR_STORAGE_FAILURE );

        case MBEDTLS_ERR_MPI_FILE_IO_ERROR:
            return( PSA_ERROR_STORAGE_FAILURE );

        case MBEDTLS_ERR_MPI_BAD_INPUT_DATA:
            return( PSA_ERROR_INVALID_ARGUMENT );

        case MBEDTLS_ERR_MPI_INVALID_CHARACTER:
            return( PSA_ERROR_INVALID_ARGUMENT );

        case MBEDTLS_ERR_MPI_BUFFER_TOO_SMALL:
            return( PSA_ERROR_BUFFER_TOO_SMALL );

        case MBEDTLS_ERR_MPI_NEGATIVE_VALUE:
            return( PSA_ERROR_INVALID_ARGUMENT );

        case MBEDTLS_ERR_MPI_DIVISION_BY_ZERO:
            return( PSA_ERROR_INVALID_ARGUMENT );

        case MBEDTLS_ERR_MPI_NOT_ACCEPTABLE:
            return( PSA_ERROR_INVALID_ARGUMENT );

        case MBEDTLS_ERR_MPI_ALLOC_FAILED:
            return( PSA_ERROR_INSUFFICIENT_MEMORY );

        case MBEDTLS_ERR_PK_ALLOC_FAILED:
            return( PSA_ERROR_INSUFFICIENT_MEMORY );

        case MBEDTLS_ERR_PK_TYPE_MISMATCH:
        case MBEDTLS_ERR_PK_BAD_INPUT_DATA:
            return( PSA_ERROR_INVALID_ARGUMENT );

        case MBEDTLS_ERR_PK_FILE_IO_ERROR:
            return( PSA_ERROR_STORAGE_FAILURE );

        case MBEDTLS_ERR_PK_KEY_INVALID_VERSION:
        case MBEDTLS_ERR_PK_KEY_INVALID_FORMAT:
            return( PSA_ERROR_INVALID_ARGUMENT );

        case MBEDTLS_ERR_PK_UNKNOWN_PK_ALG:
            return( PSA_ERROR_NOT_SUPPORTED );

        case MBEDTLS_ERR_PK_PASSWORD_REQUIRED:
        case MBEDTLS_ERR_PK_PASSWORD_MISMATCH:
            return( PSA_ERROR_NOT_PERMITTED );

        case MBEDTLS_ERR_PK_INVALID_PUBKEY:
            return( PSA_ERROR_INVALID_ARGUMENT );

        case MBEDTLS_ERR_PK_INVALID_ALG:
        case MBEDTLS_ERR_PK_UNKNOWN_NAMED_CURVE:
        case MBEDTLS_ERR_PK_FEATURE_UNAVAILABLE:
            return( PSA_ERROR_NOT_SUPPORTED );

        case MBEDTLS_ERR_PK_SIG_LEN_MISMATCH:
            return( PSA_ERROR_INVALID_SIGNATURE );

        case MBEDTLS_ERR_PK_BUFFER_TOO_SMALL:
            return( PSA_ERROR_BUFFER_TOO_SMALL );

        case MBEDTLS_ERR_PLATFORM_HW_ACCEL_FAILED:
            return( PSA_ERROR_HARDWARE_FAILURE );

        case MBEDTLS_ERR_PLATFORM_FEATURE_UNSUPPORTED:
            return( PSA_ERROR_NOT_SUPPORTED );
#ifdef MBEDTLS_RSA_C
        case MBEDTLS_ERR_RSA_BAD_INPUT_DATA:
            return( PSA_ERROR_INVALID_ARGUMENT );

        case MBEDTLS_ERR_RSA_INVALID_PADDING:
            return( PSA_ERROR_INVALID_PADDING );

        case MBEDTLS_ERR_RSA_KEY_GEN_FAILED:
            return( PSA_ERROR_HARDWARE_FAILURE );

        case MBEDTLS_ERR_RSA_KEY_CHECK_FAILED:
            return( PSA_ERROR_INVALID_ARGUMENT );

        case MBEDTLS_ERR_RSA_PUBLIC_FAILED:
        case MBEDTLS_ERR_RSA_PRIVATE_FAILED:
            return( PSA_ERROR_CORRUPTION_DETECTED );

        case MBEDTLS_ERR_RSA_VERIFY_FAILED:
            return( PSA_ERROR_INVALID_SIGNATURE );

        case MBEDTLS_ERR_RSA_OUTPUT_TOO_LARGE:
            return( PSA_ERROR_BUFFER_TOO_SMALL );

        case MBEDTLS_ERR_RSA_RNG_FAILED:
            return( PSA_ERROR_INSUFFICIENT_ENTROPY );
#endif
        case MBEDTLS_ERR_ECP_BAD_INPUT_DATA:
        case MBEDTLS_ERR_ECP_INVALID_KEY:
            return( PSA_ERROR_INVALID_ARGUMENT );

        case MBEDTLS_ERR_ECP_BUFFER_TOO_SMALL:
            return( PSA_ERROR_BUFFER_TOO_SMALL );

        case MBEDTLS_ERR_ECP_FEATURE_UNAVAILABLE:
            return( PSA_ERROR_NOT_SUPPORTED );

        case MBEDTLS_ERR_ECP_SIG_LEN_MISMATCH:
        case MBEDTLS_ERR_ECP_VERIFY_FAILED:
            return( PSA_ERROR_INVALID_SIGNATURE );

        case MBEDTLS_ERR_ECP_ALLOC_FAILED:
            return( PSA_ERROR_INSUFFICIENT_MEMORY );

        case MBEDTLS_ERR_ECP_RANDOM_FAILED:
            return( PSA_ERROR_INSUFFICIENT_ENTROPY );

        case MBEDTLS_ERR_ERROR_CORRUPTION_DETECTED:
            return( PSA_ERROR_CORRUPTION_DETECTED );

        default:

            if( ret < 0 )
            {
                return( PSA_ERROR_GENERIC_ERROR );
            }
            else
            {
                return PSA_SUCCESS;
            }
    }
}

/*-----------------------------------------------------------*/

psa_algorithm_t mbedtls_psa_translate_md( mbedtls_md_type_t md_alg )
{
    switch( md_alg )
    {
#if defined( MBEDTLS_MD5_C )
        case MBEDTLS_MD_MD5:
            return( PSA_ALG_MD5 );
#endif
#if defined( MBEDTLS_SHA1_C )
        case MBEDTLS_MD_SHA1:
            return( PSA_ALG_SHA_1 );
#endif
#if defined( MBEDTLS_SHA224_C )
        case MBEDTLS_MD_SHA224:
            return( PSA_ALG_SHA_224 );
#endif
#if defined( MBEDTLS_SHA256_C )
        case MBEDTLS_MD_SHA256:
            return( PSA_ALG_SHA_256 );
#endif
#if defined( MBEDTLS_SHA384_C )
        case MBEDTLS_MD_SHA384:
            return( PSA_ALG_SHA_384 );
#endif
#if defined( MBEDTLS_SHA512_C )
        case MBEDTLS_MD_SHA512:
            return( PSA_ALG_SHA_512 );
#endif
#if defined( MBEDTLS_RIPEMD160_C )
        case MBEDTLS_MD_RIPEMD160:
            return( PSA_ALG_RIPEMD160 );
#endif
        case MBEDTLS_MD_NONE:
            return( 0 );

        default:
            return( 0 );
    }
}

/*-----------------------------------------------------------*/


/*
 * Simultaneously convert and move raw MPI from the beginning of a buffer
 * to an ASN.1 MPI at the end of the buffer.
 * See also mbedtls_asn1_write_mpi().
 *
 * p: pointer to the end of the output buffer
 * start: start of the output buffer, and also of the mpi to write at the end
 * n_len: length of the mpi to read from start
 */
static int asn1_write_mpibuf( unsigned char ** p,
                              unsigned char * start,
                              size_t n_len )
{
    int ret = MBEDTLS_ERR_ERROR_CORRUPTION_DETECTED;
    size_t len = 0;

    if( ( size_t ) ( *p - start ) < n_len )
    {
        return( MBEDTLS_ERR_ASN1_BUF_TOO_SMALL );
    }

    len = n_len;
    *p -= len;
    memmove( *p, start, len );

    /* ASN.1 DER encoding requires minimal length, so skip leading 0s.
     * Neither r nor s should be 0, but as a failsafe measure, still detect
     * that rather than overflowing the buffer in case of a PSA error. */
    while( len > 0 && **p == 0x00 )
    {
        ++( *p );
        --len;
    }

    /* this is only reached if the signature was invalid */
    if( len == 0 )
    {
        return( MBEDTLS_ERR_PLATFORM_HW_ACCEL_FAILED );
    }

    /* if the msb is 1, ASN.1 requires that we prepend a 0.
     * Neither r nor s can be 0, so we can assume len > 0 at all times. */
    if( **p & 0x80 )
    {
        if( *p - start < 1 )
        {
            return( MBEDTLS_ERR_ASN1_BUF_TOO_SMALL );
        }

        *--( *p ) = 0x00;
        len += 1;
    }

    MBEDTLS_ASN1_CHK_ADD( len, mbedtls_asn1_write_len( p, start, len ) );
    MBEDTLS_ASN1_CHK_ADD( len, mbedtls_asn1_write_tag( p, start,
                                                       MBEDTLS_ASN1_INTEGER ) );

    return( ( int ) len );
}

/* Transcode signature from PSA format to ASN.1 sequence.
 * See ecdsa_signature_to_asn1 in ecdsa.c, but with byte buffers instead of
 * MPIs, and in-place.
 * Copied from from mbedtls/library/pk_wrap.c.
 *
 * [in/out] sig: the signature pre- and post-transcoding
 * [in/out] sig_len: signature length pre- and post-transcoding
 * [int] buf_len: the available size the in/out buffer
 */
int pk_ecdsa_sig_asn1_from_psa( unsigned char * sig,
                                size_t * sig_len,
                                size_t buf_len )
{
    int ret = MBEDTLS_ERR_ERROR_CORRUPTION_DETECTED;
    size_t len = 0;
    const size_t rs_len = *sig_len / 2;
    unsigned char * p = sig + buf_len;

    MBEDTLS_ASN1_CHK_ADD( len, asn1_write_mpibuf( &p, sig + rs_len, rs_len ) );
    MBEDTLS_ASN1_CHK_ADD( len, asn1_write_mpibuf( &p, sig, rs_len ) );

    MBEDTLS_ASN1_CHK_ADD( len, mbedtls_asn1_write_len( &p, sig, len ) );
    MBEDTLS_ASN1_CHK_ADD( len, mbedtls_asn1_write_tag( &p, sig,
                                                       MBEDTLS_ASN1_CONSTRUCTED | MBEDTLS_ASN1_SEQUENCE ) );

    memmove( sig, p, len );
    *sig_len = len;

    return( 0 );
}


const char* get_psa_function_name(uint32_t xSignal, veneer_fn fn,
                                  uint32_t arg0, uint32_t arg1,
                                  uint32_t arg2, uint32_t arg3)
{
  const char * function_name = NULL;
  const uint32_t type = arg1 >> 16U;
  const uint32_t in_size = arg1 >> 8U & 0xFFU;
  (void) fn;
  (void) arg0;
  (void) arg3;

  if( arg2 != 0U )
  {
    // arg2 != 0 <=> called from veener_psa_call
    switch( xSignal )
    {
    case PSA_SIGNAL_CRYPTO:
      {
        switch( type )
        {
        case PSA_IPC_CALL:
          {
            const uint32_t method = ( in_size < 1 ) ? (TFM_CRYPTO_SID_MAX) : ( (struct tfm_crypto_pack_iovec*)( ((psa_invec*)arg2)[0].base ) )->sfn_id;
            switch(method)
            {
              case TFM_CRYPTO_GET_KEY_ATTRIBUTES_SID:
              {
                function_name = "psa_get_key_attributes";
              }
              break;
              case TFM_CRYPTO_RESET_KEY_ATTRIBUTES_SID:
              {
                function_name = "psa_reset_key_attributes";
              }
              break;
              case TFM_CRYPTO_OPEN_KEY_SID:
              {
                function_name = "psa_open_key";
              }
              break;
              case TFM_CRYPTO_CLOSE_KEY_SID:
              {
                function_name = "psa_close_key";
              }
              break;
              case TFM_CRYPTO_IMPORT_KEY_SID:
              {
                function_name = "psa_import_key";
              }
              break;
              case TFM_CRYPTO_DESTROY_KEY_SID:
              {
                function_name = "psa_destroy_key";
              }
              break;
              case TFM_CRYPTO_EXPORT_KEY_SID:
              {
                function_name = "psa_export_key";
              }
              break;
              case TFM_CRYPTO_EXPORT_PUBLIC_KEY_SID:
              {
                function_name = "psa_export_public_key";
              }
              break;
              case TFM_CRYPTO_PURGE_KEY_SID:
              {
                function_name = "psa_purge_key";
              }
              break;
              case TFM_CRYPTO_COPY_KEY_SID:
              {
                function_name = "psa_copy_key";
              }
              break;
              case TFM_CRYPTO_HASH_COMPUTE_SID:
              {
                function_name = "psa_hash_compute";
              }
              break;
              case TFM_CRYPTO_HASH_COMPARE_SID:
              {
                function_name = "psa_hash_compare";
              }
              break;
              case TFM_CRYPTO_HASH_SETUP_SID:
              {
                function_name = "psa_hash_setup";
              }
              break;
              case TFM_CRYPTO_HASH_UPDATE_SID:
              {
                function_name = "psa_hash_update";
              }
              break;
              case TFM_CRYPTO_HASH_FINISH_SID:
              {
                function_name = "psa_hash_finish";
              }
              break;
              case TFM_CRYPTO_HASH_VERIFY_SID:
              {
                function_name = "psa_hash_verify";
              }
              break;
              case TFM_CRYPTO_HASH_ABORT_SID:
              {
                function_name = "psa_hash_abort";
              }
              break;
              case TFM_CRYPTO_HASH_CLONE_SID:
              {
                function_name = "psa_hash_clone";
              }
              break;
              case TFM_CRYPTO_MAC_COMPUTE_SID:
              {
                function_name = "psa_mac_compute";
              }
              break;
              case TFM_CRYPTO_MAC_VERIFY_SID:
              {
                function_name = "psa_mac_verify";
              }
              break;
              case TFM_CRYPTO_MAC_SIGN_SETUP_SID:
              {
                function_name = "psa_mac_sign_setup";
              }
              break;
              case TFM_CRYPTO_MAC_VERIFY_SETUP_SID:
              {
                function_name = "psa_mac_verify_setup";
              }
              break;
              case TFM_CRYPTO_MAC_UPDATE_SID:
              {
                function_name = "psa_get_key_attributes";
              }
              break;
              case TFM_CRYPTO_MAC_SIGN_FINISH_SID:
              {
                function_name = "psa_mac_sign_finish";
              }
              break;
              case TFM_CRYPTO_MAC_VERIFY_FINISH_SID:
              {
                function_name = "psa_mac_verify_finish";
              }
              break;
              case TFM_CRYPTO_MAC_ABORT_SID:
              {
                function_name = "psa_mac_abort";
              }
              break;
              case TFM_CRYPTO_CIPHER_ENCRYPT_SID:
              {
                function_name = "psa_cipher_encrypt";
              }
              break;
              case TFM_CRYPTO_CIPHER_DECRYPT_SID:
              {
                function_name = "psa_cipher_decrypt";
              }
              break;
              case TFM_CRYPTO_CIPHER_ENCRYPT_SETUP_SID:
              {
                function_name = "psa_cipher_encrypt_setup";
              }
              break;
              case TFM_CRYPTO_CIPHER_DECRYPT_SETUP_SID:
              {
                function_name = "psa_cipher_decrypt_setup";
              }
              break;
              case TFM_CRYPTO_CIPHER_GENERATE_IV_SID:
              {
                function_name = "psa_cipher_generate_iv";
              }
              break;
              case TFM_CRYPTO_CIPHER_SET_IV_SID:
              {
                function_name = "psa_cipher_set_iv";
              }
              break;
              case TFM_CRYPTO_CIPHER_UPDATE_SID:
              {
                function_name = "psa_cipher_update";
              }
              break;
              case TFM_CRYPTO_CIPHER_FINISH_SID:
              {
                function_name = "psa_cipher_finish";
              }
              break;
              case TFM_CRYPTO_CIPHER_ABORT_SID:
              {
                function_name = "psa_cipher_abort";
              }
              break;
              case TFM_CRYPTO_AEAD_ENCRYPT_SID:
              {
                function_name = "psa_aead_encrypt";
              }
              break;
              case TFM_CRYPTO_AEAD_DECRYPT_SID:
              {
                function_name = "psa_aead_decrypt";
              }
              break;
              case TFM_CRYPTO_AEAD_ENCRYPT_SETUP_SID:
              {
                function_name = "psa_aead_encrypt_setup";
              }
              break;
              case TFM_CRYPTO_AEAD_DECRYPT_SETUP_SID:
              {
                function_name = "psa_aead_decrypt_setup";
              }
              break;
              case TFM_CRYPTO_AEAD_GENERATE_NONCE_SID:
              {
                function_name = "psa_aead_generate_nonce";
              }
              break;
              case TFM_CRYPTO_AEAD_SET_NONCE_SID:
              {
                function_name = "psa_aead_set_nonce";
              }
              break;
              case TFM_CRYPTO_AEAD_SET_LENGTHS_SID:
              {
                function_name = "psa_aead_set_length";
              }
              break;
              case TFM_CRYPTO_AEAD_UPDATE_AD_SID:
              {
                function_name = "psa_aead_update_ad";
              }
              break;
              case TFM_CRYPTO_AEAD_UPDATE_SID:
              {
                function_name = "psa_aead_update";
              }
              break;
              case TFM_CRYPTO_AEAD_FINISH_SID:
              {
                function_name = "psa_aead_finish";
              }
              break;
              case TFM_CRYPTO_AEAD_VERIFY_SID:
              {
                function_name = "psa_aead_verify";
              }
              break;
              case TFM_CRYPTO_AEAD_ABORT_SID:
              {
                function_name = "psa_aead_abort";
              }
              break;
              case TFM_CRYPTO_SIGN_MESSAGE_SID:
              {
                function_name = "psa_sign_message";
              }
              break;
              case TFM_CRYPTO_VERIFY_MESSAGE_SID:
              {
                function_name = "psa_verify_message";
              }
              break;
              case TFM_CRYPTO_SIGN_HASH_SID:
              {
                function_name = "psa_sign_hash";
              }
              break;
              case TFM_CRYPTO_VERIFY_HASH_SID:
              {
                function_name = "psa_verify_hash";
              }
              break;
              case TFM_CRYPTO_ASYMMETRIC_ENCRYPT_SID:
              {
                function_name = "psa_asymmetric_encrypt";
              }
              break;
              case TFM_CRYPTO_ASYMMETRIC_DECRYPT_SID:
              {
                function_name = "psa_asymmetric_decrypt";
              }
              break;
              case TFM_CRYPTO_KEY_DERIVATION_SETUP_SID:
              {
                function_name = "psa_key_derivation_setup";
              }
              break;
              case TFM_CRYPTO_KEY_DERIVATION_GET_CAPACITY_SID:
              {
                function_name = "psa_key_derivation_get_capacity";
              }
              break;
              case TFM_CRYPTO_KEY_DERIVATION_SET_CAPACITY_SID:
              {
                function_name = "psa_key_derivation_set_capacity";
              }
              break;
              case TFM_CRYPTO_KEY_DERIVATION_INPUT_BYTES_SID:
              {
                function_name = "psa_key_derivation_input_bytes";
              }
              break;
              case TFM_CRYPTO_KEY_DERIVATION_INPUT_KEY_SID:
              {
                function_name = "psa_key_derivation_input_key";
              }
              break;
              case TFM_CRYPTO_KEY_DERIVATION_KEY_AGREEMENT_SID:
              {
                function_name = "psa_key_derivation_key_agreement";
              }
              break;
              case TFM_CRYPTO_KEY_DERIVATION_OUTPUT_BYTES_SID:
              {
                function_name = "psa_key_derivation_output_bytes";
              }
              break;
              case TFM_CRYPTO_KEY_DERIVATION_OUTPUT_KEY_SID:
              {
                function_name = "psa_key_derivation_output_key";
              }
              break;
              case TFM_CRYPTO_KEY_DERIVATION_ABORT_SID:
              {
                function_name = "psa_key_derivation_abort";
              }
              break;
              case TFM_CRYPTO_RAW_KEY_AGREEMENT_SID:
              {
                function_name = "psa_raw_key_agreement";
              }
              break;
              case TFM_CRYPTO_GENERATE_RANDOM_SID:
              {
                function_name = "psa_generate_random";
              }
              break;
              case TFM_CRYPTO_GENERATE_KEY_SID:
              {
                function_name = "psa_generate_key";
              }
              break;
              case TFM_CRYPTO_SID_MAX:
              {
                function_name = "psa_crypto_???";
              }
              break;
              default:
              {
                function_name = "psa_crypto_???";
              }
              break;
            }
          }
          break;
        default:
        {
          function_name = "psa_crypto_???";
        }
        break;
        }
      }
      break;
    case PSA_SIGNAL_FW_UPG:
      {
        switch( type )
        {
          case PSA_TYPE_FWU_WRITE:
          {
            function_name = "psa_fwu_write";
          }
          break;
          case PSA_TYPE_FWU_INSTALL:
          {
            function_name = "psa_fwu_install";
          }
          break;
          case PSA_TYPE_FWU_ABORT:
          {
            function_name = "psa_fwu_abort";
          }
          break;
          case PSA_TYPE_FWU_QUERY:
          {
            function_name = "psa_fwu_query";
          }
          break;
          case PSA_TYPE_FWU_REQUEST_REBOOT:
          {
            function_name = "psa_fwu_request_reboot";
          }
          break;
          case PSA_TYPE_FWU_ACCEPT:
          {
            function_name = "psa_fwu_accept";
          }
          break;
          default:
          {
            function_name = "psa_fwu_???";
          }
          break;
        }
      }
      break;
    case PSA_SIGNAL_STORAGE:
      {
        switch( type )
        {
          case PSA_TYPE_ITS_SET_PRIVATE_DATA:
          {
            function_name = "psa_its_set";
          }
          break;
          case PSA_TYPE_ITS_GET_PRIVATE_DATA:
          {
            function_name = "psa_its_get";
          }
          break;
          case PSA_TYPE_ITS_REMOVE_DATA:
          {
            function_name = "psa_its_remove";
          }
          break;
          case PSA_TYPE_ITS_RETRIEVE_INFO:
          {
            function_name = "psa_its_get_info";
          }
          break;
          default:
          {
            function_name = "psa_its_???";
          }
          break;
        }
      }
      break;
      default:
      {
        function_name = "psa_???";
      }
      break;
    }
  }
  return function_name;
}

#endif /* MBEDTLS_TRANSPORT_PSA */
