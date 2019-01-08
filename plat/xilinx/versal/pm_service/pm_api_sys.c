/*
 * Copyright (c) 2018, Xilinx, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * Versal system level PM-API functions and communication with PMC via
 * IPI interrupts
 */

#include <pm_common.h>
#include <pm_ipi.h>
#include "pm_api_sys.h"
#include "pm_client.h"

/*********************************************************************
 * Target module IDs macros
 ********************************************************************/
#define PLM_MODULE_ID	0x2

/**
 * Assigning of argument values into array elements.
 */
#define PM_PACK_PAYLOAD1(pl, arg0) {	\
	pl[0] = (uint32_t)((uint32_t)((arg0) & 0xFF) | (PLM_MODULE_ID << 8)); \
}

/* PM API functions */

/**
 * pm_get_api_version() - Get version number of PMC PM firmware
 * @version	Returns 32-bit version number of PMC Power Management Firmware
 *
 * @return	Returns status, either success or error+reason
 */
enum pm_ret_status pm_get_api_version(unsigned int *version)
{
	uint32_t payload[PAYLOAD_ARG_CNT];

	/* Send request to the PMC */
	PM_PACK_PAYLOAD1(payload, PM_GET_API_VERSION);
	return pm_ipi_send_sync(primary_proc, payload, version, 1);
}
