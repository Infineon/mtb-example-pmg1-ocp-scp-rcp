/*******************************************************************************
* File Name: fault.h
* Version 1.0
*
* Description:
*  This file contains the macro declaration and function prototypes used in
*  the fault source code
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
#ifndef SRC_FAULT_H_
#define SRC_FAULT_H_

/*******************************************************************************
 * Macro declarations
 ******************************************************************************/
/* CY Assert Failure */
#define CY_ASSERT_FAILED                (0u)

/* Base current for OCP in units of 10 mA */
#define OCP_BASE_CUR                    (200u)

#if (!defined(CY_DEVICE_SERIES_PMG1S2))
/* RCP Threshold */
#define RCP_THRESHOLD                   (5000u)
#endif

/* Debug print macro to enable the UART print */
#define DEBUG_PRINT                     (0u)

/******************************************************************************
 * Global function declaration
 ******************************************************************************/
void rcp_cb(void *context, bool compOut);
void enable_rcp(cy_stc_usbpd_context_t *context, uint16_t volt);
void scp_cb(void *context, bool compOut);
void enable_scp(cy_stc_usbpd_context_t *context, uint32_t cur);
void ocp_cb(void *context, bool compOut);
void enable_ocp(cy_stc_usbpd_context_t *context, uint32_t cur);
bool comp_status (cy_stc_usbpd_context_t *context);

#endif /* SRC_FAULT_H_ */
