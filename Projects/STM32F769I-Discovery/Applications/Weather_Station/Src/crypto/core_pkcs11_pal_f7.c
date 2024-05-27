
/**
 * @file pkcs11_pal_f7.c
 * @brief Limited STM32F7 internal flash file save and read implementation
 * for PKCS #11 based on mbedTLS with for software keys. This
 * file deviates from the FreeRTOS style standard for some function names and
 * data types in order to maintain compliance with the PKCS #11 standard.
 */
/*-----------------------------------------------------------*/

#include "FreeRTOS.h"
#include "atomic.h"

/* PKCS 11 includes. */
#include "core_pkcs11_config.h"
#include "core_pkcs11_config_defaults.h"
#include "core_pkcs11.h"
#include "core_pkcs11_pal_utils.h"

#include "mbedtls/asn1.h"

#include "flash.h"
#include <string.h>

#include "pkcs11_user_config.h"
//#include "data_saes_encryption.h"
//#include "appli_flash_layout.h"
//#include "low_level_ext_flash.h"

#ifndef PKCS11_PAL_EXT_FLASH_ENCRYPTION
#define PKCS11_PAL_EXT_FLASH_ENCRYPTION 1
#endif /* PKCS11_PAL_EXT_FLASH_ENCRYPTION */

#define USER_CONF_TLS_OBJECT_MAX_SIZE   2048
#define USER_CONF_MAGIC                 0x0123456789ABCDEFuLL

/** Static user configuration data which must survive reboot and firmware update.
  */
typedef struct {
  char tls_root_ca_cert[USER_CONF_TLS_OBJECT_MAX_SIZE];
  char tls_device_cert[USER_CONF_TLS_OBJECT_MAX_SIZE];
  char tls_device_privkey[USER_CONF_TLS_OBJECT_MAX_SIZE];
  char tls_device_pubkey[USER_CONF_TLS_OBJECT_MAX_SIZE];
  char ota_signer_pubkey[USER_CONF_TLS_OBJECT_MAX_SIZE];
  /* The USER_CONF_MAGIC magic word signals that the object is present in flash. */
  uint64_t tls_root_ca_cert_magic;
  uint64_t tls_device_cert_magic;
  uint64_t tls_device_privkey_magic;
  uint64_t tls_device_pubkey_magic;
  uint64_t ota_signer_pubkey_magic;
  int32_t tls_root_ca_cert_size;
  int32_t tls_device_cert_size;
  int32_t tls_device_privkey_size;
  int32_t tls_device_pubkey_size;
  int32_t ota_signer_pubkey_size;
} user_config_t;

/*-----------------------------------------------------------*/

/** Do not zero-initialize the static user configuration.
 *  Otherwise, it must be entered manually each time the device FW is updated by STLink.
 */

#if defined (__GNUC__) /* GNU compiler */
user_config_t __uninited_region_start__ __attribute__((section("UNINIT_FIXED_LOC")));
user_config_t *lUserConfigPtr = &__uninited_region_start__;
#elif defined ( __ICCARM__ ) /* IAR Compiler */
user_config_t __attribute__((section("UNINIT_FIXED_LOC"))) uninited_region_start;
user_config_t *lUserConfigPtr = &uninited_region_start;
#endif

user_config_t lUserConfig_encrypted;

uint32_t config_erase_data(void);
uint32_t config_set_default(void);

/*-----------------------------------------------------------*/

/**
 * @brief Checks to see if a file exists
 *
 * @param[in] pcFileName         The name of the file to check for existence.
 *
 * @returns CKR_OK if the file exists, CKR_OBJECT_HANDLE_INVALID if not.
 */
static CK_RV prvFileExists( const char * pcFileName )
{
  CK_RV xReturn = CKR_OK;

  if( pcFileName == NULL )
  {
    xReturn = CKR_OBJECT_HANDLE_INVALID;
  }
  else
  {
    if( ( (strncmp(pkcs11palFILE_NAME_CA_CERTIFICATE, pcFileName, strlen(pkcs11palFILE_NAME_CA_CERTIFICATE)) == 0) && (lUserConfigPtr->tls_root_ca_cert_magic == USER_CONF_MAGIC) )
      || ( (strncmp(pkcs11palFILE_NAME_CLIENT_CERTIFICATE, pcFileName, strlen(pkcs11palFILE_NAME_CLIENT_CERTIFICATE)) == 0) && (lUserConfigPtr->tls_device_cert_magic == USER_CONF_MAGIC) )
      || ( (strncmp(pkcs11palFILE_NAME_KEY, pcFileName, strlen(pkcs11palFILE_NAME_KEY)) == 0) && (lUserConfigPtr->tls_device_privkey_magic == USER_CONF_MAGIC) )
      || ( (strncmp(pkcs11palFILE_NAME_PUBLIC_KEY, pcFileName, strlen(pkcs11palFILE_NAME_PUBLIC_KEY)) == 0) && (lUserConfigPtr->tls_device_pubkey_magic == USER_CONF_MAGIC) )
      || ( (strncmp(pkcs11palFILE_CODE_SIGN_PUBLIC_KEY, pcFileName, strlen(pkcs11palFILE_CODE_SIGN_PUBLIC_KEY)) == 0) && (lUserConfigPtr->ota_signer_pubkey_magic == USER_CONF_MAGIC) )
      )
    {
      xReturn = CKR_OK;
      LogDebug( ( "Found file %s", pcFileName ) );
    }
    else
    {
      xReturn = CKR_OBJECT_HANDLE_INVALID;
      LogInfo( ( "Could not open %s for reading.", pcFileName ) );
    }
  }

  return xReturn;
}

/**
 * @brief Reads object value from file system.
 *
 * @param[in] pcFileName         The name of the file to read
 * @param[in] pcLabel            The PKCS #11 label to convert to a file name
 * @param[out] pHandle           The type of the PKCS #11 object.
 *
 */
static CK_RV prvReadData( const char * pcFileName,
                          CK_BYTE_PTR * ppucData,
                          CK_ULONG_PTR pulDataSize )
{
    CK_RV xReturn = CKR_OK;

    /* Initialize return vars */
    *ppucData = NULL;
    *pulDataSize = 0;

    if( prvFileExists(pcFileName) == CKR_OK )
    {
      if( strncmp(pkcs11palFILE_NAME_CA_CERTIFICATE, pcFileName, strlen(pkcs11palFILE_NAME_CA_CERTIFICATE)) == 0 )
      {
        *ppucData = (CK_BYTE_PTR) lUserConfigPtr->tls_root_ca_cert;
        *pulDataSize = lUserConfigPtr->tls_root_ca_cert_size;
      }
      else if( strncmp(pkcs11palFILE_NAME_CLIENT_CERTIFICATE, pcFileName, strlen(pkcs11palFILE_NAME_CLIENT_CERTIFICATE)) == 0 )
      {
        *ppucData = (CK_BYTE_PTR) lUserConfigPtr->tls_device_cert;
        *pulDataSize = lUserConfigPtr->tls_device_cert_size;
      }
      else if( strncmp(pkcs11palFILE_NAME_KEY, pcFileName, strlen(pkcs11palFILE_NAME_KEY)) == 0 )
      {
        *ppucData = (CK_BYTE_PTR) lUserConfigPtr->tls_device_privkey;
        *pulDataSize = lUserConfigPtr->tls_device_privkey_size;
      }
      else if( strncmp(pkcs11palFILE_NAME_PUBLIC_KEY, pcFileName, strlen(pkcs11palFILE_NAME_PUBLIC_KEY)) == 0 )
      {
        *ppucData = (CK_BYTE_PTR) lUserConfigPtr->tls_device_pubkey;
        *pulDataSize = lUserConfigPtr->tls_device_pubkey_size;
      }
      else if( strncmp(pkcs11palFILE_CODE_SIGN_PUBLIC_KEY, pcFileName, strlen(pkcs11palFILE_CODE_SIGN_PUBLIC_KEY)) == 0 )
      {
        *ppucData = (CK_BYTE_PTR) lUserConfigPtr->ota_signer_pubkey;
        *pulDataSize = lUserConfigPtr->ota_signer_pubkey_size;
      }
    }
    else
    {
      LogError( ( "PKCS #11 PAL failed to get object value. "
                   "Could not locate file named %s for reading.",
                   pcFileName ) );
      xReturn = CKR_FUNCTION_FAILED;
    }

    return xReturn;
}

/*-----------------------------------------------------------*/

CK_RV PKCS11_PAL_Initialize( void )
{
//  FLASH_Init_User_Config((void *)lUserConfigPtr, sizeof(user_config_t));

  return CKR_OK;
}

CK_OBJECT_HANDLE PKCS11_PAL_SaveObject( CK_ATTRIBUTE_PTR pxLabel,
                                        CK_BYTE_PTR pucData,
                                        CK_ULONG ulDataSize )
{
  CK_OBJECT_HANDLE xHandle = ( CK_OBJECT_HANDLE ) eInvalidHandle;
  const char * pcFileName = NULL;
  int ret = 0;
  uint64_t magic = USER_CONF_MAGIC;

  if( ( pxLabel != NULL ) && ( pucData != NULL ) )
  {
      /* Converts a label to its respective filename and handle. */
      PAL_UTILS_LabelToFilenameHandle( pxLabel->pValue,
                                       &pcFileName,
                                       &xHandle );
  }
  else
  {
      LogError( ( "Could not save object. Received invalid parameters." ) );
  }

  if( pcFileName != NULL )
  {
    /* Overwrite the file every time it is saved. */
    if( strncmp(pkcs11palFILE_NAME_CA_CERTIFICATE, pcFileName, strlen(pkcs11palFILE_NAME_CA_CERTIFICATE)) == 0 )
    {
      memcpy((void *) &lUserConfigPtr->tls_root_ca_cert, pucData, ulDataSize);
      memcpy((void *) &lUserConfigPtr->tls_root_ca_cert_size, &ulDataSize, sizeof(int32_t));
      memcpy((void *) &lUserConfigPtr->tls_root_ca_cert_magic, &magic, sizeof(uint64_t));
    }
    else if( strncmp(pkcs11palFILE_NAME_CLIENT_CERTIFICATE, pcFileName, strlen(pkcs11palFILE_NAME_CLIENT_CERTIFICATE)) == 0 )
    {
      memcpy((void *) &lUserConfigPtr->tls_device_cert, pucData, ulDataSize);
      memcpy((void *) &lUserConfigPtr->tls_device_cert_size, &ulDataSize, sizeof(int32_t));
      memcpy((void *) &lUserConfigPtr->tls_device_cert_magic, &magic, sizeof(uint64_t));
    }
    else if( strncmp(pkcs11palFILE_NAME_KEY, pcFileName, strlen(pkcs11palFILE_NAME_KEY)) == 0 )
    {
      memcpy((void *) &lUserConfigPtr->tls_device_privkey, pucData, ulDataSize);
      memcpy((void *) &lUserConfigPtr->tls_device_privkey_size, &ulDataSize, sizeof(int32_t));
      memcpy((void *) &lUserConfigPtr->tls_device_privkey_magic, &magic, sizeof(uint64_t));
    }
    else if( strncmp(pkcs11palFILE_NAME_PUBLIC_KEY, pcFileName, strlen(pkcs11palFILE_NAME_PUBLIC_KEY)) == 0 )
    {          
      memcpy((void *) &lUserConfigPtr->tls_device_pubkey, pucData, ulDataSize);
      memcpy((void *) &lUserConfigPtr->tls_device_pubkey_size, &ulDataSize, sizeof(int32_t));
      memcpy((void *) &lUserConfigPtr->tls_device_pubkey_magic, &magic, sizeof(uint64_t));
    }
    else if( strncmp(pkcs11palFILE_CODE_SIGN_PUBLIC_KEY, pcFileName, strlen(pkcs11palFILE_CODE_SIGN_PUBLIC_KEY)) == 0 )
    {          
      memcpy((void *) &lUserConfigPtr->ota_signer_pubkey, pucData, ulDataSize);
      memcpy((void *) &lUserConfigPtr->ota_signer_pubkey_size, &ulDataSize, sizeof(int32_t));
      memcpy((void *) &lUserConfigPtr->ota_signer_pubkey_magic, &magic, sizeof(uint64_t));
    }
    else
    {
      LogError( ( "PKCS #11 PAL was unable to save object to file. "
                  "The PAL was unable to open a file with name %s in write mode.", pcFileName ) );
      xHandle = ( CK_OBJECT_HANDLE ) eInvalidHandle;
    }

//    if(config_set_data() != -1)
//    {
//        ret = 0;
//    }
//    else
//    {
//        ret = -1;
//    }
    
    if( ret < 0 )
    {
        LogError( ( "PKCS #11 PAL was unable to save object to file. "
                    "Failed to commit changes to flash. rc: %d", ret ) );
        xHandle = ( CK_OBJECT_HANDLE ) eInvalidHandle;
    }
    else
    {
      LogDebug( ( "Successfully wrote %lu to %s", lBytesWritten, pcFileName ) );
    }
  }

  return xHandle;
}

/*-----------------------------------------------------------*/


CK_OBJECT_HANDLE PKCS11_PAL_FindObject( CK_BYTE_PTR pxLabel,
                                        CK_ULONG usLength )
{
    const char * pcFileName = NULL;
    CK_OBJECT_HANDLE xHandle = ( CK_OBJECT_HANDLE ) eInvalidHandle;

    ( void ) usLength;

    pxLabel[usLength] = 0x00;
    
    if( pxLabel != NULL )
    {
        PAL_UTILS_LabelToFilenameHandle( ( const char * ) pxLabel,
                                         &pcFileName,
                                         &xHandle );

        if( ( pcFileName == NULL ) || ( CKR_OK != prvFileExists( pcFileName ) ) )
        {
            xHandle = ( CK_OBJECT_HANDLE ) eInvalidHandle;
        }
    }
    else
    {
        LogError( ( "Could not find object. Received a NULL label." ) );
    }

    return xHandle;
}
/*-----------------------------------------------------------*/

CK_RV PKCS11_PAL_GetObjectValue( CK_OBJECT_HANDLE xHandle,
                                 CK_BYTE_PTR * ppucData,
                                 CK_ULONG_PTR pulDataSize,
                                 CK_BBOOL * pIsPrivate )
{
    CK_RV xReturn = CKR_OK;
    const char * pcFileName = NULL;


    if( ( ppucData == NULL ) || ( pulDataSize == NULL ) || ( pIsPrivate == NULL ) )
    {
        xReturn = CKR_ARGUMENTS_BAD;
        LogError( ( "Could not get object value. Received a NULL argument." ) );
    }
    else
    {
        xReturn = PAL_UTILS_HandleToFilename( xHandle, &pcFileName, pIsPrivate );
    }

    if( xReturn == CKR_OK )
    {
        xReturn = prvReadData( pcFileName, ppucData, pulDataSize );
    }

    return xReturn;
}

/*-----------------------------------------------------------*/

void PKCS11_PAL_GetObjectValueCleanup( CK_BYTE_PTR pucData,
                                       CK_ULONG ulDataSize )
{
    /* Unused parameters. */
    ( void ) pucData;
    ( void ) ulDataSize;
}

/*-----------------------------------------------------------*/

CK_RV PKCS11_PAL_DestroyObject( CK_OBJECT_HANDLE xHandle )
{
    const char * pcFileName = NULL;
    CK_BBOOL xIsPrivate = CK_TRUE;
    CK_RV xResult = CKR_OBJECT_HANDLE_INVALID;

    xResult = PAL_UTILS_HandleToFilename( xHandle,
                                          &pcFileName,
                                          &xIsPrivate );

    if( ( xResult == CKR_OK ) &&
        ( prvFileExists( pcFileName ) == CKR_OK ) )
    {
      if( strncmp(pkcs11palFILE_NAME_CA_CERTIFICATE, pcFileName, strlen(pkcs11palFILE_NAME_CA_CERTIFICATE)) == 0 )
      {
          lUserConfigPtr->tls_root_ca_cert_magic = 0;
      }
      else if( strncmp(pkcs11palFILE_NAME_CLIENT_CERTIFICATE, pcFileName, strlen(pkcs11palFILE_NAME_CLIENT_CERTIFICATE)) == 0 )
      {
          lUserConfigPtr->tls_device_cert_magic = 0;
      }
      else if( strncmp(pkcs11palFILE_NAME_KEY, pcFileName, strlen(pkcs11palFILE_NAME_KEY)) == 0 )
      {
          lUserConfigPtr->tls_device_privkey_magic = 0;
      }
      else if( strncmp(pkcs11palFILE_NAME_PUBLIC_KEY, pcFileName, strlen(pkcs11palFILE_NAME_PUBLIC_KEY)) == 0 )
      {
          lUserConfigPtr->tls_device_pubkey_magic = 0;
      }
      else if( strncmp(pkcs11palFILE_CODE_SIGN_PUBLIC_KEY, pcFileName, strlen(pkcs11palFILE_CODE_SIGN_PUBLIC_KEY)) == 0 )
      {
          lUserConfigPtr->ota_signer_pubkey_magic = 0;
      }
      else
      {
        xResult = CKR_FUNCTION_FAILED;
      }
    }

    return xResult;
}

/*-----------------------------------------------------------*/

/**
  * @brief  Set configuration in secure storage
  * @param  none
  * @retval status
  */
uint32_t config_set_data(void)
{
//  int32_t status = ARM_DRIVER_ERROR;
//
//  // Erase flash sectors
//
//  for(uint32_t addr = FLASH_CREDENTIALS_AREA_BEGIN_OFFSET ; addr <= (FLASH_CREDENTIALS_AREA_BEGIN_OFFSET + sizeof(user_config_t)); addr+= EXT_FLASH_SECTOR_SIZE)
//  {
//    Ext_Flash_EraseSector(addr);
//  }
//
//#ifdef PKCS11_PAL_EXT_FLASH_ENCRYPTION
//
//  SAES_EncryptData((uint32_t *) lUserConfigPtr, (sizeof(user_config_t)/sizeof(uint32_t)), (uint32_t *) &lUserConfig_encrypted, (sizeof(user_config_t)/sizeof(uint32_t)));
//
//  status = FLASH_update((void *) &lUserConfig_encrypted, sizeof(user_config_t));
//
//  memset( &lUserConfig_encrypted, 0x00, sizeof( user_config_t ) );
//#else
//  status = FLASH_update((void *) lUserConfigPtr, sizeof(user_config_t));
//#endif /* PKCS11_PAL_EXT_FLASH_ENCRYPTION */
//
//  if (status != ARM_DRIVER_OK)
//  {
//    LogError("config_set_data failure (%d)\r\n", status);
//    return -1;
//  }

  return 0;
}

/**
  * @brief  Read the saved config
  * @param  none
  * @retval status
  */
uint32_t config_get_data(void)
{
//  int32_t status = ARM_DRIVER_ERROR;
//
//  status = Ext_Flash_ReadData(FLASH_CREDENTIALS_AREA_BEGIN_OFFSET,(void*) &lUserConfig_encrypted, sizeof(user_config_t));
//
//  if (status != ARM_DRIVER_OK)
//  {
//    LogError("config_get_data failed with status %d\r\n", status);
//    return -1;
//  }
//
//#ifdef PKCS11_PAL_EXT_FLASH_ENCRYPTION
//  SAES_DecryptData((uint32_t *) &lUserConfig_encrypted, (sizeof(user_config_t)/sizeof(uint32_t)), (uint32_t *) lUserConfigPtr, (sizeof(user_config_t)/sizeof(uint32_t)));
//#else
//  memcpy(lUserConfigPtr, &lUserConfig_encrypted, sizeof(user_config_t));
//#endif
//
//  memset( &lUserConfig_encrypted, 0x00, sizeof( user_config_t ) );
//
  return 0;
}
