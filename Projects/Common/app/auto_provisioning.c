/**
  ******************************************************************************
  * @file           : auto_provisioning.c
  * @brief          : Import User Device Unique Authentication credentials
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

#include "logging_levels.h"
/* define LOG_LEVEL here if you want to modify the logging level from the default */

//#define LOG_LEVEL    LOG_ERROR

#include "logging.h"

#include "mbedtls_transport.h"
#include "mbedtls/oid.h"
#include "mbedtls/asn1.h"
#include "kvstore.h"
#include <string.h>
#include <stdbool.h>

#include "stm32_cert.h"
#include "psa/internal_trusted_storage.h"

#define SUBJECT_CN_LEN 40

void vPopulateDeviceIdentity(void);
static BaseType_t prvSetThingNameFromCert( uint8_t *p_certificate, uint32_t certificate_size );

static size_t prvGetCertCNFromName( unsigned char ** ppucCommonName,
                                   mbedtls_x509_name * pxCertName )
{
    size_t uxCommonNameLen = 0;

    configASSERT( ppucCommonName != NULL );
    configASSERT( pxCertName != NULL );

    *ppucCommonName = NULL;

    for( ; pxCertName != NULL; pxCertName = pxCertName->next )
    {
        if( MBEDTLS_OID_CMP( MBEDTLS_OID_AT_CN, &( pxCertName->oid ) ) == 0 )
        {
            *ppucCommonName = pxCertName->val.p;
            uxCommonNameLen = pxCertName->val.len;
            break;
        }
    }

    return( uxCommonNameLen );
}


/* Retrieve the DUA User certificate from system flash, and write it to PSA ITS if needed.
   Set the thing name accordingly in KVStore. */
void vPopulateDeviceIdentity()
{
  psa_status_t psa_status = PSA_ERROR_DOES_NOT_EXIST;
  CERT_Status_t cert_status;
  psa_storage_uid_t cert_id = PSA_TLS_CERT_ID;
  uint32_t dua_certificate_size = 0;
  uint8_t * p_dua_certificate;
  uint8_t * p_its_certificate;
  struct psa_storage_info_t psa_s_info;
  bool b_its_needs_provisioning = false;

  cert_status = UTIL_CERT_GetCertificateSize(DUA_USER, &dua_certificate_size);
  if( cert_status == CERT_OK )
  {
    p_dua_certificate = pvPortMalloc( dua_certificate_size );
    if( p_dua_certificate != NULL )
    {
      cert_status = UTIL_CERT_GetCertificate(DUA_USER, p_dua_certificate);
      if( cert_status == CERT_OK )
      {
        /* Set the thing name in KVStore */
        (void) prvSetThingNameFromCert( p_dua_certificate, dua_certificate_size );

        /* Write cert to ITS only if it is different */
        psa_status = psa_its_get_info( cert_id, &psa_s_info );
        if( psa_status == PSA_SUCCESS )
        {
          p_its_certificate = pvPortMalloc( psa_s_info.size );
          if( p_its_certificate != NULL )
          {
            size_t read_length;
            psa_status = psa_its_get( cert_id, 0, psa_s_info.size, p_its_certificate, &read_length );
            if( psa_status == PSA_SUCCESS )
            {
              /* The comparison assumes that both certificates are stored in DER format */
              if( memcmp(p_dua_certificate, p_its_certificate, dua_certificate_size) == 0 )
              {
                LogInfo("DUA User certificate is already available in ITS. No need to write it again.");
              }
              else
              {
                b_its_needs_provisioning = true;
              }
            }
            vPortFree( p_its_certificate );
          }
          else
          {
            LogError("malloc failed when reading device certificate from ITS");
          }
        } else if( psa_status == PSA_ERROR_DOES_NOT_EXIST ) {
          b_its_needs_provisioning = true;
        }

        if( b_its_needs_provisioning == true )
        {
          psa_status = psa_its_set(cert_id, dua_certificate_size, p_dua_certificate, 0);
          if( psa_status == PSA_SUCCESS )
          {
            LogInfo("Copied DUA User certificate from system flash to ITS.");
          }
          else
          {
            LogError("psa_its_set(id = %u) failed error = %d\r\n", cert_id, psa_status);
          }
        }

      }
      else
      {
        LogError("UTIL_CERT_GetCertificate(id = %u) failed error = %d\r\n", cert_id, cert_status);
      }
      vPortFree(p_dua_certificate);
    }
    else
    {
      LogError("malloc failed when reading device certificate from system flash");
    }
  }
  else
  {
    LogError("UTIL_CERT_GetCertificateSize(id = %u) failed error = %d\r\n", cert_id, cert_status);
  }

  return ;
}


/* Set the thing name in the KVStore according to the DUA User certificate */
static BaseType_t prvSetThingNameFromCert( uint8_t *p_certificate, uint32_t certificate_size )
{
  BaseType_t status = pdFALSE;
  mbedtls_x509_crt xClientCert;
  unsigned char * pString;
  char subjectCN[SUBJECT_CN_LEN];

  mbedtls_x509_crt_init( &xClientCert );
  if ( mbedtls_x509_crt_parse_der( &xClientCert, p_certificate, certificate_size ) < 0 )
  {
    LogError("Failed to parse DUA certificate from buffer");
  }
  else
  {
    size_t size = prvGetCertCNFromName( &pString, &( xClientCert.subject ) );
    if( size > 0 )
    {
      snprintf( subjectCN, sizeof( subjectCN ), "%.*s", size, pString );
      status = KVStore_setString( CS_CORE_THING_NAME, subjectCN );
      if( status != pdTRUE )
      {
        LogError( "Failed to store subject CN into KVstore at thing name key." );
      }
      else
      {
          memset( subjectCN, 0, sizeof( subjectCN ) );
          (void) KVStore_getString( CS_CORE_THING_NAME, subjectCN, sizeof( subjectCN ) );
      }
    }
    else
    {
      LogError( "prvGetCertCNFromName error" );
    }
  }
  return status;
}

