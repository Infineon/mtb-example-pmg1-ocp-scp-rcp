/******************************************************************************
* File Name: fault.c
* Version 1.0
*
* Description:
*  This file provides the source code for enabling faults and their
*  respective callback.
*
* Related Document: See README.md
*
*******************************************************************************
* Copyright 2023-2024, Cypress Semiconductor Corporation (an Infineon company) or
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
#include "cybsp.h"
#include "cy_pdl.h"
#include "fault.h"

/*******************************************************************************
* Global Variables
*******************************************************************************/
/* Flag to detect OCP event */
extern volatile bool ocp_flag;

#if (!defined(CY_DEVICE_SERIES_PMG1S2))
/* Flag to detect RCP event */
extern volatile bool rcp_flag;

/* Flag to detect SCP event */
extern volatile bool scp_flag;
#endif /* (!defined(CY_DEVICE_SERIES_PMG1S2)) */

/* Global variable to read the OCP status bit */
uint32_t ocp_status = 0;

/* USB PD context */
extern cy_stc_usbpd_context_t gl_UsbPdPort0Ctx;

#if (!defined(CY_DEVICE_SERIES_PMG1S2))
/*******************************************************************************
* Function Name: rcp_cb
********************************************************************************
* Summary:
*  Callback function for RCP
*
* Parameters:
*  context - USBPD context
*  compOut - Comparator output
*
* Return:
*  void
*
*******************************************************************************/
void rcp_cb(void *context, bool compOut)
{
    (void)context;
    (void)compOut;

    /* Set the RCP flag */
    rcp_flag = true;
}

/*******************************************************************************
* Function Name: enable_rcp
********************************************************************************
* Summary:
*  Calls the API to enable Reverse current protection (RCP)
*
* Parameters:
*  context - USBPD context
*  volt - voltage used to calculate the RCP threshold (in mV)
*
* Return:
*  void
*
*******************************************************************************/
void enable_rcp(cy_stc_usbpd_context_t *context, uint16_t volt)
{
    /* Enter Critical Section */
    uint8_t intr_state = Cy_SysLib_EnterCriticalSection();

    /* Enables RCP */
    Cy_USBPD_Fault_Vbus_RcpEnable(context, volt, (cy_cb_vbus_fault_t) rcp_cb);

    /* Exits critical section */
    Cy_SysLib_ExitCriticalSection(intr_state);
}

/*******************************************************************************
* Function Name: scp_cb
********************************************************************************
* Summary:
*  Callback function for SCP
*
* Parameters:
*  context - USBPD context
*  compOut - Comparator output
*
* Return:
*  void
*
*******************************************************************************/
void scp_cb(void *context, bool compOut)
{
    (void)context;
    (void)compOut;

    /* Set the SCP flag */
    scp_flag = true;
}

/*******************************************************************************
* Function Name: enable_scp
********************************************************************************
* Summary:
*  Calls the API to enable Short Circuit Protection (SCP)
*
* Parameters:
*  context - USBPD context
*  cur - current used to calculate the SCP threshold (in mA)
*
* Return:
*  void
*
*******************************************************************************/
void enable_scp(cy_stc_usbpd_context_t *context, uint32_t cur)
{
    /* Enter critical section */
    uint8_t intr_state = Cy_SysLib_EnterCriticalSection();

    /* Enables SCP */
    Cy_USBPD_Fault_Vbus_ScpEnable(context, cur, (cy_cb_vbus_fault_t) scp_cb);

    /* Exits critical section */
    Cy_SysLib_ExitCriticalSection(intr_state);
}
#endif /* (!defined(CY_DEVICE_SERIES_PMG1S2)) */

/*******************************************************************************
* Function Name: ocp_cb
********************************************************************************
* Summary:
*  Callback function for OCP
*
* Parameters:
*  context - USBPD context
*  compOut - Comparator output
*
* Return:
*  void
*
*******************************************************************************/
void ocp_cb(void *context, bool compOut)
{
    (void)context;
    (void)compOut;

#if (!defined(CY_DEVICE_SERIES_PMG1S2))
    ocp_status = ((gl_UsbPdPort0Ctx.base->intr13_status >> 8) & 0x01);
#else
    ocp_status = ((gl_UsbPdPort0Ctx.base->ncell_status >> 1) & 0x01);
#endif /* (!defined(CY_DEVICE_SERIES_PMG1S2)) */

    if (ocp_status == 1)
    {
        ocp_flag = true;
    }
}

/*******************************************************************************
* Function Name: enable_ocp
********************************************************************************
* Summary:
*  Calls the API to enable Over Current Protection (OCP)
*
* Parameters:
*  context - USBPD context
*  cur - current used to calculate the OCP threshold (in mA)
*
* Return:
*  void
*
*******************************************************************************/
void enable_ocp(cy_stc_usbpd_context_t *context, uint32_t cur)
{
    /* Enter critical section */
    uint8_t intr_state = Cy_SysLib_EnterCriticalSection();

    /* Enable OCP */
    Cy_USBPD_Fault_Vbus_OcpEnable(context, cur, (cy_cb_vbus_fault_t) ocp_cb);

    /* Exit critical section */
    Cy_SysLib_ExitCriticalSection(intr_state);
}

/*******************************************************************************
* Function Name: comp_status
********************************************************************************
* Summary:
*  - Parses the OCP Comparator bit
*  - If OCP Comparator is set, returns true
*  - If OCP Comparator is zero, returns false
*
* Parameters:
*  context - USBPD context
*
* Return:
*  bool
*
*******************************************************************************/
bool comp_status (cy_stc_usbpd_context_t *context)
{
#if (!defined(CY_DEVICE_SERIES_PMG1S2))
    ocp_status = ((gl_UsbPdPort0Ctx.base->intr13_status >> 8) & 0x01);
#else
    ocp_status = ((gl_UsbPdPort0Ctx.base->ncell_status >> 1) & 0x01);
#endif /* (!defined(CY_DEVICE_SERIES_PMG1S2)) */

    if (ocp_status == 1)
    {
#if DEBUG_PRINT
        Cy_SCB_UART_PutString(CYBSP_UART_HW, "OCP fault detected\r\n\n");
#endif
        return true;
    }
    else
    {
        return false;
    }
}

/* [] END OF FILE */

