/*******************************************************************************
* File Name: bt_app.h
*
* Description: This file is the public interface of bt_app.c source file
*
* Related Document: README.md
*
********************************************************************************
* $ Copyright 2023-YEAR Cypress Semiconductor $
*******************************************************************************/

/*******************************************************************************
 * Include guard
 ******************************************************************************/
#ifndef BT_APP_H
#define BT_APP_H

/*******************************************************************************
 * Header file includes
 ******************************************************************************/
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "wiced_bt_dev.h"
#include "wiced_bt_ble.h"
#include "wiced_bt_gatt.h"
#include "wiced_bt_uuid.h"
#include "wiced_result.h"
#include "wiced_bt_stack.h"
#include "wiced_memory.h"
#include "cybt_platform_config.h"
#include "cycfg_gatt_db.h"


#define BT_TASK_PRIORITY             (2u)
#define BT_TASK_STACK_SIZE           (1024)

/*******************************************************************************
* Global constants
*******************************************************************************/
/* Notification parameters */
enum
{
    NOTIFIY_OFF,
    NOTIFIY_ON,
};

/*******************************************************************************
 * Extern Variables
 ******************************************************************************/
extern uint8_t notify_enabled;
/*******************************************************************************
 * Function prototype
 ******************************************************************************/
void bt_app_send_notification(uint8_t index);
wiced_result_t bt_app_management_cb(wiced_bt_management_evt_t event,
                                    wiced_bt_management_evt_data_t *p_event_data);
void bt_task(void* param);
#endif /* BT_APP_H */
