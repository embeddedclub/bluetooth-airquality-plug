/*******************************************************************************
* File Name: flash_utils.h
*
* Description: This file is the public interface of flash_utils.c
*
* Related Document: See README.md
*
********************************************************************************
* $ Copyright 2021-YEAR Cypress Semiconductor $
*******************************************************************************/

/*******************************************************************************
 * Include guard
 ******************************************************************************/
#ifndef FLASH_UTILS_H_
#define FLASH_UTILS_H_

/*******************************************************************************
 * Header file includes
 ******************************************************************************/
#include "mtb_kvstore.h"

/*******************************************************************************
 * Macros
 ******************************************************************************/

/*******************************************************************************
 * Function Prototype
 ******************************************************************************/
cy_rslt_t flash_memory_init(void);
uint16_t flash_memory_write(uint16_t config_item_id, uint32_t len, uint8_t* buf, wiced_result_t *rslt);
uint16_t flash_memory_read(uint16_t config_item_id, uint32_t len, uint8_t* buf, wiced_result_t *rslt);
cy_rslt_t flash_memory_delete(uint16_t config_item_id);
cy_rslt_t flash_memory_reset(void);
/*******************************************************************************
 * External Function Prototype
 ******************************************************************************/
 extern cy_en_smif_status_t cybsp_smif_init(void);
/*******************************************************************************
 * Variable Definitions
 ******************************************************************************/

extern mtb_kvstore_t kv_store_obj;

/* SMIF configuration structure */
extern cy_stc_smif_mem_config_t* smifMemConfigs[];
extern cy_stc_smif_context_t cybsp_smif_context;

#endif /* FLASH_UTILS_H_ */
