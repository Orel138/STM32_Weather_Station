/*
 * Copyright (C) 2021 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * https://www.FreeRTOS.org
 * https://github.com/FreeRTOS
 *
 */

#if defined( STM32H7S7xx )
#include "appli_flash_layout.h"
#include "low_level_ext_flash.h"

#include "data_saes_encryption.h"
#endif /* STM32H7S7xx */

#include "logging_levels.h"
#include "logging.h"
#include "kvstore_prv.h"
#include <string.h>

#if KV_STORE_NVIMPL_EXT_FLASH

#define KV_KEY_MAX_LENGTH      (32)
#define KV_VALUE_MAX_LENGTH    (256)

#define KV_STORE_ENTRY_MAGIC    (0xAFAFAFAF)

typedef struct {
  uint32_t            magic;
  char                key[KV_KEY_MAX_LENGTH];
  KVStoreValueType_t  type;
  size_t              length;
  union
  {
      uint8_t     value[KV_VALUE_MAX_LENGTH];
      char        str[KV_VALUE_MAX_LENGTH];
      UBaseType_t uxData;
      BaseType_t  xData;
      uint32_t    ulData;
      int32_t     lData;
  };
  uint16_t crc16;
} KVStoreEntry_t;

 
__ALIGN_BEGIN KVStoreEntry_t kv_store[CS_NUM_KEYS] __ALIGN_END;
KVStoreEntry_t kv_store_encrypted[CS_NUM_KEYS];

static bool isEntryValid(const KVStoreKey_t xKey )
{
#if KV_STORE_NVIMPL_EXT_FLASH_ENCRYPTION == 1
  KVStoreEntry_t *pxEntry = &kv_store[xKey];
#else
  KVStoreEntry_t *pxEntry = (KVStoreEntry_t*) (EXT_FLASH_BASE_ADDRESS + FLASH_KVSTORE_AREA_BEGIN_OFFSET + ((uint32_t)xKey * sizeof(KVStoreEntry_t)) );
#endif /* KV_STORE_NVIMPL_EXT_FLASH_ENCRYPTION */
  
  const char* pcKey= kvKeyToString(xKey);
  configASSERT(pcKey);
  
  if(pxEntry->magic != KV_STORE_ENTRY_MAGIC) return false;
  if(pxEntry->type != KVStore_getType(xKey)) return false;
  if(strcmp(pcKey, pxEntry->key) != 0) return false;
  return true;
}


/*
 * @brief Get the length of a value stored in the KVStore implementation
 * @param[in] xKey Key to lookup
 * @return length of the value stored in the KVStore or 0 if not found.
 */
size_t xprvGetValueLengthFromImpl( const KVStoreKey_t xKey )
{
#if KV_STORE_NVIMPL_EXT_FLASH_ENCRYPTION == 1
  KVStoreEntry_t *pxEntry = &kv_store[xKey];
#else
  KVStoreEntry_t *pxEntry = (KVStoreEntry_t*) (EXT_FLASH_BASE_ADDRESS + FLASH_KVSTORE_AREA_BEGIN_OFFSET + ((uint32_t)xKey * sizeof(KVStoreEntry_t)) );
#endif /* KV_STORE_NVIMPL_EXT_FLASH_ENCRYPTION */

  if(!isEntryValid(xKey)) return 0;
  
  return pxEntry->length;
}

/*
 * @brief Read the value for the given key into a given buffer.
 * @param[in] xKey The key to lookup
 * @param[out] pxType The type of the value returned.
 * @param[out] pxLength Pointer to store the length of the read value in.
 * @param[out] pvBuffer The buffer to copy the value to.
 * @param[in] xBufferSize The length of the provided buffer.
 * @return pdTRUE on success, otherwise pdFALSE.
 */
BaseType_t xprvReadValueFromImpl( const KVStoreKey_t xKey,
                                  KVStoreValueType_t * pxType,
                                  size_t * pxLength,
                                  void * pvBuffer,
                                  size_t xBufferSize )
{
#if KV_STORE_NVIMPL_EXT_FLASH_ENCRYPTION == 1
  KVStoreEntry_t *pxEntry = &kv_store[xKey];
#else
  KVStoreEntry_t *pxEntry = (KVStoreEntry_t*) (EXT_FLASH_BASE_ADDRESS + FLASH_KVSTORE_AREA_BEGIN_OFFSET + ((uint32_t)xKey * sizeof(KVStoreEntry_t)) );
#endif /* KV_STORE_NVIMPL_EXT_FLASH_ENCRYPTION */

  if(!isEntryValid(xKey)) return pdFALSE;

  if(xBufferSize<pxEntry->length) return pdFALSE;

  *pxType = pxEntry->type;
  *pxLength = pxEntry->length;

  memcpy(pvBuffer, (void*) (&pxEntry->value[0]), pxEntry->length);
  
  return pdTRUE;
}

/*
 * @brief Write a value for a given key to non-volatile storage.
 * @param[in] xKey Key to store the given value in.
 * @param[in] xType Type of value to record.
 * @param[in] xLength length of the value given in pxDataUnion.
 * @param[in] pxData Pointer to a buffer containing the value to be stored.
 * The caller must free any heap allocated buffers passed into this function.
 */
BaseType_t xprvWriteValueToImpl( const KVStoreKey_t xKey,
                                 const KVStoreValueType_t xType,
                                 const size_t xLength,
                                 const void * pvData )
{

  if(xLength>KV_VALUE_MAX_LENGTH) return pdFALSE;

  const char* pcKey= kvKeyToString(xKey);
  configASSERT(pcKey);
  configASSERT(strlen(pcKey)<(KV_KEY_MAX_LENGTH-1));
  configASSERT(xLength<KV_VALUE_MAX_LENGTH);

#if KV_STORE_NVIMPL_EXT_FLASH_ENCRYPTION == 1
  Ext_Flash_ReadData(FLASH_KVSTORE_AREA_BEGIN_OFFSET, (void*) &kv_store_encrypted, sizeof(kv_store));

  SAES_DecryptData((uint32_t *) kv_store_encrypted, sizeof(kv_store_encrypted)/sizeof(uint32_t), (uint32_t *) kv_store, sizeof(kv_store)/sizeof(uint32_t));
#else
  Ext_Flash_ReadData(FLASH_KVSTORE_AREA_BEGIN_OFFSET, (void*) &kv_store_encrypted, sizeof(kv_store));

#endif /*KV_STORE_NVIMPL_EXT_FLASH_ENCRYPTION == 1 */

  strncpy(&kv_store[xKey].key[0], pcKey, (KV_KEY_MAX_LENGTH-1));
  kv_store[xKey].key[(KV_KEY_MAX_LENGTH-1)]=0;
  
  kv_store[xKey].type = xType;
  kv_store[xKey].length = xLength;
  
  memcpy((void*)&kv_store[xKey].value[0], pvData, xLength);

  kv_store[xKey].magic = KV_STORE_ENTRY_MAGIC;

  BaseType_t xRes = pdTRUE;
  for(int i=0 ; i<4 && xRes; i++)
  {
    xRes &= (Ext_Flash_EraseSector(FLASH_KVSTORE_AREA_BEGIN_OFFSET + (i * EXT_FLASH_SECTOR_SIZE)) == ARM_DRIVER_OK);
  }
  
#if KV_STORE_NVIMPL_EXT_FLASH_ENCRYPTION == 1
  SAES_EncryptData((uint32_t *) kv_store, sizeof(kv_store)/sizeof(uint32_t), (uint32_t *) kv_store_encrypted, sizeof(kv_store)/sizeof(uint32_t));
  xRes |= (Ext_Flash_ProgramData(FLASH_KVSTORE_AREA_BEGIN_OFFSET, &kv_store_encrypted, sizeof(kv_store)) == ARM_DRIVER_OK);
#else

  xRes |= (Ext_Flash_ProgramData(FLASH_KVSTORE_AREA_BEGIN_OFFSET, &kv_store, sizeof(kv_store)) == ARM_DRIVER_OK);

#endif /* KV_STORE_NVIMPL_EXT_FLASH_ENCRYPTION == 1 */



  /*
   * Clear any sensitive data stored in ram temporarily
   * Free heap allocated buffer
   */
#if defined ( __ICCARM__ ) /* IAR Compiler */
  memset( kv_store, 0x00, sizeof( kv_store ) );
  memset( kv_store_encrypted, 0x00, sizeof( kv_store ) );
#else
  explicit_bzero( kv_store, sizeof( kv_store ) );
  explicit_bzero( kv_store_encrypted, sizeof( kv_store ) );
#endif /* IAR Compiler */
  
  return xRes;
}

void vprvNvImplInit( void )
{
#if KV_STORE_NVIMPL_EXT_FLASH_ENCRYPTION == 1
  
  Ext_Flash_ReadData(FLASH_KVSTORE_AREA_BEGIN_OFFSET, (void*) &kv_store_encrypted, sizeof(kv_store));
  
  SAES_DecryptData((uint32_t *) kv_store_encrypted, sizeof(kv_store)/sizeof(uint32_t), (uint32_t *) kv_store, sizeof(kv_store)/sizeof(uint32_t));
  
#if defined ( __ICCARM__ ) /* IAR Compiler */
  memset( kv_store_encrypted, 0x00, sizeof( kv_store ) );
#else
  explicit_bzero( kv_store_encrypted, sizeof( kv_store ) );
#endif /* IAR Compiler */
#else

  Ext_Flash_ReadData(FLASH_KVSTORE_AREA_BEGIN_OFFSET, (void*) &kv_store, sizeof(kv_store));

#endif /* KV_STORE_NVIMPL_EXT_FLASH_ENCRYPTION */
}

#endif /* KV_STORE_NVIMPL_EXT_FLASH */
