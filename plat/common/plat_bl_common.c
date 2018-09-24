/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <assert.h>
#include <bl_common.h>
#include <debug.h>
#include <errno.h>
#if TRUSTED_BOARD_BOOT
#include <mbedtls_config.h>
#endif
#include <platform.h>

/*
 * The following platform functions are weakly defined. The Platforms
 * may redefine with strong definition.
 */
#pragma weak bl2_el3_plat_prepare_exit
#pragma weak plat_error_handler
#pragma weak bl2_plat_preload_setup
#pragma weak bl2_plat_handle_pre_image_load
#pragma weak bl2_plat_handle_post_image_load
#pragma weak plat_try_next_boot_source
#pragma weak plat_get_mbedtls_heap

void bl2_el3_plat_prepare_exit(void)
{
}

void __dead2 plat_error_handler(int err)
{
	while (1)
		wfi();
}

void bl2_plat_preload_setup(void)
{
}

int bl2_plat_handle_pre_image_load(unsigned int image_id)
{
	return 0;
}

int bl2_plat_handle_post_image_load(unsigned int image_id)
{
	return 0;
}

int plat_try_next_boot_source(void)
{
	return 0;
}

#if !ERROR_DEPRECATED
#pragma weak bl2_early_platform_setup2

/*
 * The following platform API implementation that allow compatibility for
 * the older platform APIs.
 */
void bl2_early_platform_setup2(u_register_t arg0, u_register_t arg1,
			u_register_t arg2, u_register_t arg3)
{
	bl2_early_platform_setup((void *)arg1);
}
#endif


#if TRUSTED_BOARD_BOOT
/*
 * The following default implementation of the function simply returns the
 * by-default allocated heap.
 */
int plat_get_mbedtls_heap(void **heap_addr, size_t *heap_size)
{
	static unsigned char heap[TF_MBEDTLS_HEAP_SIZE];

	assert(heap_addr != NULL);
	assert(heap_size != NULL);

	*heap_addr = heap;
	*heap_size = sizeof(heap);
	return 0;
}
#endif /* TRUSTED_BOARD_BOOT */
