/******************************************************************************
* File Name:   main.c
*
* Description: This is the source code for the Bluetooth Air Quality Checker
*
* Related Document: See README.md
*
*******************************************************************************
* $ Copyright 2023-YEAR Cypress Semiconductor $
*******************************************************************************/

/*******************************************************************************
* Header Files
*******************************************************************************/
#include <stdio.h>
#include <string.h>
#include "cyhal.h"
#include "cy_pdl.h"
#include "cybsp.h"
#include "cy_retarget_io.h"

#include "wiced_bt_stack.h"
#include "cybsp_bt_config.h"
#include "cycfg_bt_settings.h"
#include "cybt_platform_config.h"

#include "bt_app.h"
#include "flash_utils.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "xensiv_pasco2_mtb.h"
#include "ws2812.h"

/*******************************************************************************
* Macros
*******************************************************************************/
#define GPIO_INTERRUPT_PRIORITY (7u)

#define PIN_XENSIV_PASCO2_I2C_SDA   	CYBSP_I2C_SDA
#define PIN_XENSIV_PASCO2_I2C_SCL   	CYBSP_I2C_SCL

#define I2C_MASTER_FREQUENCY        	(400000U)

/* Wait time for sensor ready (milliseconds) */
#define WAIT_SENSOR_RDY_MS          	(200)

#define DI_TASK_PRIORITY				(2u)
#define DI_TASK_STACK_SIZE				(512u)

#define CO2_GREEN   (0x0000FF00)
#define CO2_LGREEN  (0x00D0FF00)
#define CO2_ORANGE  (0x00FFA500)
#define CO2_RED		(0x00FF0000)


//#define BTTEST

/*******************************************************************************
* Global Variables
*******************************************************************************/
/* Variable which holds the button pressed status */
volatile bool co2_check_flag = false;
TaskHandle_t  dis_task_handle;
cyhal_gpio_callback_data_t gpio_btn_callback_data;

cyhal_i2c_t cyhal_i2c;
xensiv_pasco2_t xensiv_pasco2;

/*******************************************************************************
* Function Prototypes
*******************************************************************************/

/* button press interrupt handler */
static void gpio_interrupt_handler(void *handler_arg, cyhal_gpio_event_t event);
/* handler for general errors */
void handle_error(uint32_t status);

void display_task(void* param);
/*******************************************************************************
* Function Definitions
*******************************************************************************/

/*******************************************************************************
* Function Name: main
********************************************************************************
* Summary:
* This is the main function. It initializes the CAN-FD channel and interrupt.
* User button and User LED are also initialized. The main loop checks for the
* button pressed interrupt flag and when it is set, a CAN-FD frame is sent.
* Whenever a CAN-FD frame is received from other nodes, the user LED toggles and
* the received data is logged over serial terminal.
*
* Parameters:
*  none
*
* Return:
*  int
*
*******************************************************************************/
int main(void)
{
    cy_rslt_t result;
    BaseType_t rtos_result;
    /* Initialize I2C */
    cyhal_i2c_cfg_t i2c_master_config = {CYHAL_I2C_MODE_MASTER,
                                         0,
                                         I2C_MASTER_FREQUENCY};

    /* Initialize the device and board peripherals */
    result = cybsp_init();
    /* Board init failed. Stop program execution */
    handle_error(result);
    /* Initialize retarget-io for uart logging */
    result = cy_retarget_io_init(CYBSP_DEBUG_UART_TX, CYBSP_DEBUG_UART_RX,
                                CY_RETARGET_IO_BAUDRATE);
    /* Retarget-io init failed. Stop program execution */
    handle_error(result);


    /* Enable global interrupts */
    __enable_irq();

    printf("===========================================================\r\n");
    printf("Bluetooth Co2 Sensor example\r\n");
    printf("===========================================================\r\n\n");


    result = cyhal_gpio_init(CYBSP_USER_LED2, CYHAL_GPIO_DIR_OUTPUT,
                    CYHAL_GPIO_DRIVE_STRONG, CYBSP_LED_STATE_OFF);
    /* User LED init failed. Stop program execution */
    handle_error(result);

    /* Initialize the user button */
    result = cyhal_gpio_init(CYBSP_USER_BTN, CYHAL_GPIO_DIR_INPUT,
                    CYBSP_USER_BTN_DRIVE, CYBSP_BTN_OFF);
    /* User button init failed. Stop program execution */
    handle_error(result);


    /* Configure GPIO interrupt */
    gpio_btn_callback_data.callback = gpio_interrupt_handler;
    cyhal_gpio_register_callback(CYBSP_USER_BTN,
                                 &gpio_btn_callback_data);
    cyhal_gpio_enable_event(CYBSP_USER_BTN, CYHAL_GPIO_IRQ_FALL,
                                     GPIO_INTERRUPT_PRIORITY, true);

    result = cyhal_i2c_init(&cyhal_i2c, PIN_XENSIV_PASCO2_I2C_SDA, PIN_XENSIV_PASCO2_I2C_SCL, NULL);
    CY_ASSERT(result == CY_RSLT_SUCCESS);

    result = cyhal_i2c_configure(&cyhal_i2c, &i2c_master_config);
    CY_ASSERT(result == CY_RSLT_SUCCESS);


    /* Initialize the external flash */
    if(CY_RSLT_SUCCESS == flash_memory_init())
    {
        printf("Flash memory initialized! \r\n");
    }

    /* Configure platform specific settings for the BT device */
    cybt_platform_config_init(&cybsp_bt_platform_cfg);

    /* Register call back and configuration with stack */
    result = wiced_bt_stack_init(bt_app_management_cb, &wiced_bt_cfg_settings);

    /* Check if stack initialization was successful */
    if(WICED_BT_SUCCESS == result)
    {
        printf("Bluetooth stack initialization successful!\r\n");
    }
    else
    {
        printf("Bluetooth stack initialization failed!\r\n");
        CY_ASSERT(0);
    }


    if(pdPASS != xTaskCreate(bt_task, "BT Task", BT_TASK_STACK_SIZE,
                                                NULL, BT_TASK_PRIORITY, NULL))
    {
        CY_ASSERT(0u);
    }

    if(pdPASS != xTaskCreate(display_task, "Display Task", DI_TASK_STACK_SIZE,
                                                NULL, DI_TASK_PRIORITY, &dis_task_handle))
    {
        CY_ASSERT(0u);
    }


    /* Start the FreeRTOS scheduler */
    vTaskStartScheduler();


}

/*******************************************************************************
* Function Name: gpio_interrupt_handler
********************************************************************************
* Summary:
*   GPIO interrupt handler.
*
* Parameters:
*  void *handler_arg (unused)
*  cyhal_gpio_event_t (unused)
*
*******************************************************************************/
static void gpio_interrupt_handler(void *handler_arg, cyhal_gpio_event_t event)
{
	co2_check_flag = 1 - co2_check_flag;
}


/*******************************************************************************
* Function Name: display_task
********************************************************************************
* Summary:
*  Task that handles Bluetooth initialization and updates GATT notification data.
*
* Parameters:
*  void *param : Task parameter defined during task creation (unused)
*
* Return:
*  None
*
*******************************************************************************/
void display_task(void* param)
{
	 cy_rslt_t result = CY_RSLT_SUCCESS;
uint32_t ppm;

	result = StripLights_Init();

	if (CY_RSLT_SUCCESS != result)
	{
	   printf("PixLED init failed! \r\n");
	  CY_ASSERT(0);
	}

	printf("PixLED init done! \r\n");
	StripLights_Intensity(1);	// <<< Set intensity to maximum

	StripLights_Pixel(0, WS2812_WHITE);
	StripLights_Trigger(1);


    /* Repeatedly running part of the task */
    for(;;)
    {

        /* Block till a notification is received. */
        xTaskNotifyWait(0, 0, &ppm, portMAX_DELAY);

    	if(ppm <= 600 && ppm > 400)
		{
			printf("CO2 level is Very Good!\r\n");
			StripLights_Pixel(0, CO2_GREEN);
			StripLights_Trigger(1);
		}
		else if(ppm <= 800 && ppm > 600)
		{
			printf("CO2 level is Good!\r\n");
			StripLights_Pixel(0, CO2_GREEN);
			StripLights_Trigger(1);
		}
		else if(ppm <= 1000 && ppm > 800)
		{
			printf("CO2 level is Fair!\r\n");
			StripLights_Pixel(0, CO2_LGREEN);
			StripLights_Trigger(1);
		}
		else if(ppm <= 1400 && ppm > 1000)
		{
			printf("CO2 level is Bad!\r\n");
			StripLights_Pixel(0, CO2_ORANGE);
			StripLights_Trigger(1);
		}
		else if(ppm > 1400)
		{
			printf("CO2 level is Very Bad!\r\n");
			StripLights_Pixel(0, CO2_RED);
			StripLights_Trigger(1);
		}


    }


}
/*******************************************************************************
* Function Name: handle_error
********************************************************************************
*
* Summary:
* User defined error handling function. This function processes unrecoverable
* errors such as any initialization errors etc. In case of such error the system
* will enter into assert.
*
* Parameters:
*  uint32_t status - status indicates success or failure
*
* Return:
*  void
*
*******************************************************************************/
void handle_error(uint32_t status)
{
    if (status != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }
}

/* [] END OF FILE */
