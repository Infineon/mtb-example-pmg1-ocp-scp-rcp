/******************************************************************************
* File Name: main.c
*
* Description: This is the source code for the PMG1 MCU: Using CSA block
*              for OCP, SCP & RCP Example for ModusToolbox.
*
* Related Document: See README.md
*
*******************************************************************************
* Copyright 2021-2023, Cypress Semiconductor Corporation (an Infineon company) or
* an affiliate of Cypress Semiconductor Corporation.  All rights reserved.
*
* This software, including source code, documentation and related
* materials ("Software") is owned by Cypress Semiconductor Corporation
* or one of its affiliates ("Cypress") and is protected by and subject to
* worldwide patent protection (United States and foreign),
* United States copyright laws and international treaty provisions.
* Therefore, you may use this Software only as provided in the license
* agreement accompanying the software package from which you
* obtained this Software ("EULA").
* If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
* non-transferable license to copy, modify, and compile the Software
* source code solely for use in connection with Cypress's
* integrated circuit products.  Any reproduction, modification, translation,
* compilation, or representation of this Software except as specified
* above is prohibited without the express written permission of Cypress.
*
* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
* reserves the right to make changes to the Software without notice. Cypress
* does not assume any liability arising out of the application or use of the
* Software or any product or circuit described in the Software. Cypress does
* not authorize its products for use in any products where a malfunction or
* failure of the Cypress product may reasonably be expected to result in
* significant property damage, injury or death ("High Risk Product"). By
* including Cypress's product in a High Risk Product, the manufacturer
* of such system or application assumes all risk of such use and in doing
* so agrees to indemnify Cypress against all liability.
*******************************************************************************/

/*******************************************************************************
 * Include header files
 ******************************************************************************/
#include "cy_pdl.h"
#include "cybsp.h"
#include <stdio.h>
#include <inttypes.h>
#include "fault.h"

/*******************************************************************************
* Global Variables
*******************************************************************************/
/* USB PD context */
cy_stc_usbpd_context_t gl_UsbPdPort0Ctx;

/* Flag to detect OCP event */
volatile bool ocp_flag = false;

/* Flag to detect SCP event */
volatile bool scp_flag = false;

/* Flag to detect RCP event */
volatile bool rcp_flag = false;

/*******************************************************************************
* Function Prototypes
********************************************************************************/
#if DEBUG_PRINT
/* Structure for UART context */
cy_stc_scb_uart_context_t CYBSP_UART_context;

/* Variable used for tracking the print status */
volatile bool ENTER_LOOP = true;

/*******************************************************************************
* Function Name: check_status
********************************************************************************
* Summary:
*  Prints the error message.
*
* Parameters:
*  error_msg - message to print if any error encountered.
*  status - status obtained after evaluation.
*
* Return:
*  void
*
*******************************************************************************/
void check_status(char *message, cy_rslt_t status)
{
    char error_msg[100];

    sprintf(error_msg, "Error Code: 0x%08" PRIX32 " n", status);

    Cy_SCB_UART_PutString(CYBSP_UART_HW, "\r\n=====================================================\r\n");
    Cy_SCB_UART_PutString(CYBSP_UART_HW, "\nFAIL: ");
    Cy_SCB_UART_PutString(CYBSP_UART_HW, message);
    Cy_SCB_UART_PutString(CYBSP_UART_HW, "\r\n");
    Cy_SCB_UART_PutString(CYBSP_UART_HW, error_msg);
    Cy_SCB_UART_PutString(CYBSP_UART_HW, "\r\n=====================================================\r\n");
}
#endif

/* USBPD driver initialization */
cy_stc_pd_dpm_config_t* get_dpm_connect_stat(void)
{
    return NULL;
}

/* Interrupt configuration for USB PD Port 0 */
const cy_stc_sysint_t usbpd_port0_intr1_config =
{
    .intrSrc = (IRQn_Type)mtb_usbpd_port0_DS_IRQ,
    .intrPriority = 0U,
};

/* Interrupt handler for USB PD Port 0 */
static void cy_usbpd0_intr1_handler(void)
{
    Cy_USBPD_Intr1Handler(&gl_UsbPdPort0Ctx);
}

/*******************************************************************************
* Function Name: main
********************************************************************************
* Summary:
*  System entrance point. This function performs
*  - Initial setup of device
*  - Initialize USBPD interrupt
*  - Initialize USBPD
*  - Enable OCP for PMG1-S2
*  - Enable OCP SCP & RCP for PMG1-S1 and PMG1-S3
*
* Parameters:
*  None
*
* Return:
*  int
*
*******************************************************************************/
int main(void)
{
    cy_rslt_t result;
    cy_en_usbpd_status_t usbpd_result;
    cy_en_sysint_status_t sysint_result;

#if (!defined(CY_DEVICE_SERIES_PMG1S2))
    uint16_t scp_cur = 0u;
#endif

    /* Initialize the device and board peripherals */
    result = cybsp_init();

    /* Board init failed. Stop program execution */
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(CY_ASSERT_FAILED);
    }

#if DEBUG_PRINT
    /* Configure and enable the UART peripheral */
    Cy_SCB_UART_Init(CYBSP_UART_HW, &CYBSP_UART_config, &CYBSP_UART_context);
    Cy_SCB_UART_Enable(CYBSP_UART_HW);

    /* Sequence to clear screen */
    Cy_SCB_UART_PutString(CYBSP_UART_HW, "\x1b[2J\x1b[;H");

    /* Print "Using CSA block for OCP, SCP & RCP" for PMG1-S1 and PMG1-S3 or
     * "Using CSA block for OCP" for PMG1-S2 */
    Cy_SCB_UART_PutString(CYBSP_UART_HW, "****************** ");
#if (!defined(CY_DEVICE_SERIES_PMG1S2))
    Cy_SCB_UART_PutString(CYBSP_UART_HW, "EZ-PDTM PMG1: Using CSA block for OCP, SCP & RCP");
#else
    Cy_SCB_UART_PutString(CYBSP_UART_HW, "EZ-PDTM PMG1: Using CSA block for OCP");
#endif /* (!defined(CY_DEVICE_SERIES_PMG1S2)) */
    Cy_SCB_UART_PutString(CYBSP_UART_HW, "****************** \r\n\n");
#endif

    /* Enable global interrupts */
    __enable_irq();

    /* To set data field in USBPD context structure to NULL.
     * Required for uninterrupted USBPD driver initialization */
    memset((void *)&gl_UsbPdPort0Ctx, 0, sizeof (cy_stc_usbpd_context_t));

    /* Configure and enable the USBPD interrupts */
    sysint_result = Cy_SysInt_Init(&usbpd_port0_intr1_config, &cy_usbpd0_intr1_handler);
    /* System Interrupt init failed. Stop program execution */
    if (sysint_result != CY_SYSINT_SUCCESS)
    {
#if DEBUG_PRINT
        check_status("API Cy_SysInt_Init failed with error code", sysint_result);
#endif
        CY_ASSERT(CY_ASSERT_FAILED);
    }
    NVIC_EnableIRQ(usbpd_port0_intr1_config.intrSrc);

    /* Initialize the USBPD driver */
#if defined(CY_DEVICE_SERIES_PMG1S2)
    usbpd_result = Cy_USBPD_Init(&gl_UsbPdPort0Ctx, 0, mtb_usbpd_port0_HW, NULL,
            (cy_stc_usbpd_config_t *)&mtb_usbpd_port0_config, get_dpm_connect_stat);
#else
    usbpd_result = Cy_USBPD_Init(&gl_UsbPdPort0Ctx, 0, mtb_usbpd_port0_HW, mtb_usbpd_port0_HW_TRIM,
            (cy_stc_usbpd_config_t *)&mtb_usbpd_port0_config, get_dpm_connect_stat);
#endif /* defined(CY_DEVICE_SERIES_PMG1S2) */

    /* USBPD driver init failed. Stop program execution */
    if (usbpd_result != CY_USBPD_STAT_SUCCESS)
    {
#if DEBUG_PRINT
        check_status("API Cy_USBPD_Init failed with error code", usbpd_result);
#endif
        CY_ASSERT(CY_ASSERT_FAILED);
    }

    /* Enable and configure the OCP block if the USB PD OCP feature is enabled */
    cy_stc_fault_vbus_ocp_cfg_t * ocp_config = (cy_stc_fault_vbus_ocp_cfg_t *)gl_UsbPdPort0Ctx.usbpdConfig->vbusOcpConfig;
    if (ocp_config->enable)
    {
        enable_ocp(&gl_UsbPdPort0Ctx, OCP_BASE_CUR);
    }

#if (!defined(CY_DEVICE_SERIES_PMG1S2))
    /* Getting the threshold value from USB PD Peripheral (SCP current in units of 10 mA) */
    scp_cur = (gl_UsbPdPort0Ctx.usbpdConfig->vbusScpConfig->threshold * 100);

    /* Enable and configure the SCP block if the USB PD SCP feature is enabled */
    cy_stc_fault_vbus_scp_cfg_t * scp_config = (cy_stc_fault_vbus_scp_cfg_t *)gl_UsbPdPort0Ctx.usbpdConfig->vbusScpConfig;
    if (scp_config->enable)
    {
        enable_scp(&gl_UsbPdPort0Ctx, scp_cur);
    }

    /* Enable and configure the RCP block if the USB PD RCP feature is enabled  */
    cy_stc_fault_vbus_rcp_cfg_t * rcp_config = (cy_stc_fault_vbus_rcp_cfg_t *)gl_UsbPdPort0Ctx.usbpdConfig->vbusRcpConfig;
    if (rcp_config->enable)
    {
        enable_rcp(&gl_UsbPdPort0Ctx, RCP_THRESHOLD);
    }
#endif /* (!defined(CY_DEVICE_SERIES_PMG1S2)) */

    /* Set User LED to ON to indicate board is in Idle state */
    Cy_GPIO_Clr(CYBSP_USER_LED_PORT, CYBSP_USER_LED_PIN);

    for(;;)
    {
        /* Check if OCP flag is set */
        if(ocp_flag == true)
        {
#if (!defined(CY_DEVICE_SERIES_PMG1S2))
            while (comp_status (&gl_UsbPdPort0Ctx) && (scp_flag != true) && (rcp_flag != true))
#else
            while (comp_status (&gl_UsbPdPort0Ctx))
#endif /* (!defined(CY_DEVICE_SERIES_PMG1S2)) */
            {
                /* Toggle the user LED state */
                Cy_GPIO_Inv(CYBSP_USER_LED_PORT, CYBSP_USER_LED_PIN);

                /* Delay */
                Cy_SysLib_Delay(125);
            }

            ocp_flag = false;

            /* Set User LED to ON to indicate board is in Idle state */
            Cy_GPIO_Clr(CYBSP_USER_LED_PORT, CYBSP_USER_LED_PIN);

            /* Enable OCP */
            Cy_USBPD_Fault_Vbus_OcpEnable(&gl_UsbPdPort0Ctx, OCP_BASE_CUR, (cy_cb_vbus_fault_t) ocp_cb);
        }

#if (!defined(CY_DEVICE_SERIES_PMG1S2))
        /* Check if SCP flag is set */
        if(scp_flag)
        {
#if DEBUG_PRINT
            Cy_SCB_UART_PutString(CYBSP_UART_HW, "SCP fault detected\r\n\n");
            Cy_SCB_UART_PutString(CYBSP_UART_HW, "Remove or Reduce current and Reset the board\r\n\n");
#endif
            /* Toggle the LED 2 times per second to indicate SCP is triggered */
            while (true)
            {
                /* Toggle the user LED state */
                Cy_GPIO_Inv(CYBSP_USER_LED_PORT, CYBSP_USER_LED_PIN);

                /* Delay */
                Cy_SysLib_Delay(500);
            }
        }

        /* Check if RCP flag is set */
        if(rcp_flag)
        {
#if DEBUG_PRINT
            Cy_SCB_UART_PutString(CYBSP_UART_HW, "RCP fault detected\r\n\n");
            Cy_SCB_UART_PutString(CYBSP_UART_HW, "Remove or Reduce current and Reset the board\r\n\n");
#endif
            /* Toggle the LED 2 times per second to indicate RCP is triggered */
            while (true)
            {
                /* Toggle the user LED state */
                Cy_GPIO_Inv(CYBSP_USER_LED_PORT, CYBSP_USER_LED_PIN);

                /* Delay */
                Cy_SysLib_Delay(500);
            }
        }
#endif /* (!defined (CY_DEVICE_SERIES_PMG1S2)) */

#if DEBUG_PRINT
        if (ENTER_LOOP)
        {
            Cy_SCB_UART_PutString(CYBSP_UART_HW, "Entered for loop\r\n");
            ENTER_LOOP = false;
        }
#endif
    }
}

/* [] END OF FILE */

