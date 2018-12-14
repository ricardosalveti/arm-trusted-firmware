/*
 * Copyright (c) 2018, Xilinx, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * Versal IPI agent registers access management
 */

#include <bakery_lock.h>
#include <debug.h>
#include <errno.h>
#include <ipi.h>
#include <mmio.h>
#include <plat_ipi.h>
#include <plat_private.h>
#include <runtime_svc.h>
#include <string.h>

/* versal ipi configuration table */
const static struct ipi_config versal_ipi_table[] = {
	/* A72 IPI */
	[IPI_ID_APU] = {
		.ipi_bit_mask = IPI0_TRIG_BIT,
		.ipi_reg_base = IPI0_REG_BASE,
		.secure_only = 0,
	},

	/* PMC IPI */
	[IPI_ID_PMC] = {
		.ipi_bit_mask = PMC_IPI_TRIG_BIT,
		.ipi_reg_base = IPI0_REG_BASE,
		.secure_only = IPI_SECURE_MASK,
	},
};

/* versal_ipi_config_table_init() - Initialize versal IPI configuration data
 *
 * @ipi_config_table  - IPI configuration table
 * @ipi_total - Total number of IPI available
 *
 */
void versal_ipi_config_table_init(void)
{
	ipi_config_table_init(versal_ipi_table, ARRAY_SIZE(versal_ipi_table));
}
