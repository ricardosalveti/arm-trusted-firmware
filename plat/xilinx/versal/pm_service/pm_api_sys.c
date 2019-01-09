/*
 * Copyright (c) 2018, Xilinx, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * Versal system level PM-API functions and communication with PMC via
 * IPI interrupts
 */

#include <platform.h>
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

#define PM_PACK_PAYLOAD2(pl, arg0, arg1) {	\
	pl[1] = (uint32_t)(arg1);		\
	PM_PACK_PAYLOAD1(pl, arg0);		\
}

#define PM_PACK_PAYLOAD3(pl, arg0, arg1, arg2) {	\
	pl[2] = (uint32_t)(arg2);			\
	PM_PACK_PAYLOAD2(pl, arg0, arg1);		\
}

#define PM_PACK_PAYLOAD4(pl, arg0, arg1, arg2, arg3) {	\
	pl[3] = (uint32_t)(arg3);			\
	PM_PACK_PAYLOAD3(pl, arg0, arg1, arg2);		\
}

#define PM_PACK_PAYLOAD5(pl, arg0, arg1, arg2, arg3, arg4) {	\
	pl[4] = (uint32_t)(arg4);				\
	PM_PACK_PAYLOAD4(pl, arg0, arg1, arg2, arg3);		\
}

#define PM_PACK_PAYLOAD6(pl, arg0, arg1, arg2, arg3, arg4, arg5) {	\
	pl[5] = (uint32_t)(arg5);					\
	PM_PACK_PAYLOAD5(pl, arg0, arg1, arg2, arg3, arg4);		\
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

/**
 * pm_self_suspend() - PM call for processor to suspend itself
 * @nid		Node id of the processor or subsystem
 * @latency	Requested maximum wakeup latency (not supported)
 * @state	Requested state
 * @address	Resume address
 *
 * This is a blocking call, it will return only once PMU has responded.
 * On a wakeup, resume address will be automatically set by PMU.
 *
 * @return	Returns status, either success or error+reason
 */
enum pm_ret_status pm_self_suspend(uint32_t nid,
				   unsigned int latency,
				   unsigned int state,
				   uintptr_t address)
{
	uint32_t payload[PAYLOAD_ARG_CNT];
	unsigned int cpuid = plat_my_core_pos();
	const struct pm_proc *proc = pm_get_proc(cpuid);

	if (!proc) {
		WARN("Failed to get proc %d\n", cpuid);
		return PM_RET_ERROR_INTERNAL;
	}

	/*
	 * Do client specific suspend operations
	 * (e.g. set powerdown request bit)
	 */
	pm_client_suspend(proc, state);

	/* Send request to the PLM */
	PM_PACK_PAYLOAD6(payload, PM_SELF_SUSPEND, proc->node_id, latency,
			 state, address, (address >> 32));
	return pm_ipi_send_sync(proc, payload, NULL, 0);
}

/**
 * pm_abort_suspend() - PM call to announce that a prior suspend request
 *			is to be aborted.
 * @reason	Reason for the abort
 *
 * Calling PU expects the PMU to abort the initiated suspend procedure.
 * This is a non-blocking call without any acknowledge.
 *
 * @return	Returns status, either success or error+reason
 */
enum pm_ret_status pm_abort_suspend(enum pm_abort_reason reason)
{
	uint32_t payload[PAYLOAD_ARG_CNT];

	/*
	 * Do client specific abort suspend operations
	 * (e.g. enable interrupts and clear powerdown request bit)
	 */
	pm_client_abort_suspend();

	/* Send request to the PLM */
	PM_PACK_PAYLOAD3(payload, PM_ABORT_SUSPEND, reason,
			 primary_proc->node_id);
	return pm_ipi_send(primary_proc, payload);
}

/**
 * pm_req_suspend() - PM call to request for another PU or subsystem to
 *		      be suspended gracefully.
 * @target	Node id of the targeted PU or subsystem
 * @ack		Flag to specify whether acknowledge is requested
 * @latency	Requested wakeup latency (not supported)
 * @state	Requested state (not supported)
 *
 * @return	Returns status, either success or error+reason
 */
enum pm_ret_status pm_req_suspend(uint32_t target, uint8_t ack,
				  unsigned int latency, unsigned int state)
{
	uint32_t payload[PAYLOAD_ARG_CNT];

	/* Send request to the PMU */
	PM_PACK_PAYLOAD4(payload, PM_REQ_SUSPEND, target, latency, state);
	if (ack == IPI_BLOCKING)
		return pm_ipi_send_sync(primary_proc, payload, NULL, 0);
	else
		return pm_ipi_send(primary_proc, payload);
}

/**
 * pm_request_device() - Request a device
 * @device_id		Device ID
 * @capabilities	Requested capabilities for the device
 * @latency		Requested maximum latency
 * @qos			Required Quality of Service
 *
 * @return	Returns status, either success or error+reason
 */
enum pm_ret_status pm_request_device(uint32_t device_id, uint32_t capabilities,
				     uint32_t latency, uint32_t qos)
{
	uint32_t payload[PAYLOAD_ARG_CNT];

	/* Send request to the PMC */
	PM_PACK_PAYLOAD6(payload, PM_REQUEST_DEVICE, XPM_SUBSYSID_APU,
			 device_id, capabilities, latency, qos);

	return pm_ipi_send_sync(primary_proc, payload, NULL, 0);
}

/**
 * pm_release_device() - Release a device
 * @device_id		Device ID
 *
 * @return	Returns status, either success or error+reason
 */
enum pm_ret_status pm_release_device(uint32_t device_id)
{
	uint32_t payload[PAYLOAD_ARG_CNT];

	/* Send request to the PMC */
	PM_PACK_PAYLOAD2(payload, PM_RELEASE_DEVICE, device_id);

	return pm_ipi_send_sync(primary_proc, payload, NULL, 0);
}

/**
 * pm_set_requirement() - Set requirement for the device
 * @device_id		Device ID
 * @capabilities	Requested capabilities for the device
 * @latency		Requested maximum latency
 * @qos			Required Quality of Service
 *
 * @return	Returns status, either success or error+reason
 */
enum pm_ret_status pm_set_requirement(uint32_t device_id, uint32_t capabilities,
				      uint32_t latency, uint32_t qos)
{
	uint32_t payload[PAYLOAD_ARG_CNT];

	/* Send request to the PMC */
	PM_PACK_PAYLOAD5(payload, PM_SET_REQUIREMENT, device_id, capabilities,
			 latency, qos);

	return pm_ipi_send_sync(primary_proc, payload, NULL, 0);
}

/**
 * pm_get_device_status() - Get device's status
 * @device_id		Device ID
 * @response		Buffer to store device status response
 *
 * @return	Returns status, either success or error+reason
 */
enum pm_ret_status pm_get_device_status(uint32_t device_id, uint32_t *response)
{
	uint32_t payload[PAYLOAD_ARG_CNT];

	/* Send request to the PMC */
	PM_PACK_PAYLOAD2(payload, PM_GET_DEVICE_STATUS, device_id);

	return pm_ipi_send_sync(primary_proc, payload, response, 3);
}

/**
 * pm_reset_assert() - Assert/De-assert reset
 * @reset	Reset ID
 * @assert	Assert (1) or de-assert (0)
 *
 * @return	Returns status, either success or error+reason
 */
enum pm_ret_status pm_reset_assert(uint32_t reset, bool assert)
{
	uint32_t payload[PAYLOAD_ARG_CNT];

	/* Send request to the PMC */
	PM_PACK_PAYLOAD3(payload, PM_RESET_ASSERT, reset, assert);

	return pm_ipi_send_sync(primary_proc, payload, NULL, 0);
}

/**
 * pm_reset_get_status() - Get current status of a reset line
 * @reset	Reset ID
 * @status	Returns current status of selected reset ID
 *
 * @return	Returns status, either success or error+reason
 */
enum pm_ret_status pm_reset_get_status(uint32_t reset, uint32_t *status)
{
	uint32_t payload[PAYLOAD_ARG_CNT];

	/* Send request to the PMC */
	PM_PACK_PAYLOAD2(payload, PM_RESET_ASSERT, reset);

	return pm_ipi_send_sync(primary_proc, payload, status, 1);
}

/**
 * pm_pinctrl_request() - Request a pin
 * @pin		Pin ID
 *
 * @return	Returns status, either success or error+reason
 */
enum pm_ret_status pm_pinctrl_request(uint32_t pin)
{
	uint32_t payload[PAYLOAD_ARG_CNT];

	/* Send request to the PMC */
	PM_PACK_PAYLOAD2(payload, PM_PINCTRL_REQUEST, pin);

	return pm_ipi_send_sync(primary_proc, payload, NULL, 0);
}

/**
 * pm_pinctrl_release() - Release a pin
 * @pin		Pin ID
 *
 * @return	Returns status, either success or error+reason
 */
enum pm_ret_status pm_pinctrl_release(uint32_t pin)
{
	uint32_t payload[PAYLOAD_ARG_CNT];

	/* Send request to the PMC */
	PM_PACK_PAYLOAD2(payload, PM_PINCTRL_RELEASE, pin);

	return pm_ipi_send_sync(primary_proc, payload, NULL, 0);
}

/**
 * pm_pinctrl_set_function() - Set pin function
 * @pin		Pin ID
 * @function	Function ID
 *
 * @return	Returns status, either success or error+reason
 */
enum pm_ret_status pm_pinctrl_set_function(uint32_t pin, uint32_t function)
{
	uint32_t payload[PAYLOAD_ARG_CNT];

	/* Send request to the PMC */
	PM_PACK_PAYLOAD3(payload, PM_PINCTRL_SET_FUNCTION, pin, function);

	return pm_ipi_send_sync(primary_proc, payload, NULL, 0);
}

/**
 * pm_pinctrl_get_function() - Get function set on the pin
 * @pin		Pin ID
 * @function	Function set on the pin
 *
 * @return	Returns status, either success or error+reason
 */
enum pm_ret_status pm_pinctrl_get_function(uint32_t pin, uint32_t *function)
{
	uint32_t payload[PAYLOAD_ARG_CNT];

	/* Send request to the PMC */
	PM_PACK_PAYLOAD2(payload, PM_PINCTRL_SET_FUNCTION, pin);

	return pm_ipi_send_sync(primary_proc, payload, function, 1);
}

/**
 * pm_pinctrl_set_pin_param() - Set configuration parameter for the pin
 * @pin		Pin ID
 * @param	Parameter ID
 * @value	Parameter value
 *
 * @return	Returns status, either success or error+reason
 */
enum pm_ret_status pm_pinctrl_set_pin_param(uint32_t pin, uint32_t param,
					    uint32_t value)
{
	uint32_t payload[PAYLOAD_ARG_CNT];

	/* Send request to the PMC */
	PM_PACK_PAYLOAD4(payload, PM_PINCTRL_CONFIG_PARAM_SET,
			 pin, param, value);

	return pm_ipi_send_sync(primary_proc, payload, NULL, 0);
}

/**
 * pm_pinctrl_get_pin_param() - Get configuration parameter value for the pin
 * @pin		Pin ID
 * @param	Parameter ID
 * @value	Buffer to store parameter value
 *
 * @return	Returns status, either success or error+reason
 */
enum pm_ret_status pm_pinctrl_get_pin_param(uint32_t pin, uint32_t param,
					    uint32_t *value)
{
	uint32_t payload[PAYLOAD_ARG_CNT];

	/* Send request to the PMC */
	PM_PACK_PAYLOAD3(payload, PM_PINCTRL_CONFIG_PARAM_GET, pin, param);

	return pm_ipi_send_sync(primary_proc, payload, value, 1);
}

/**
 * pm_clock_enable() - Enable the clock
 * @clk_id	Clock ID
 *
 * @return	Returns status, either success or error+reason
 */
enum pm_ret_status pm_clock_enable(uint32_t clk_id)
{
	uint32_t payload[PAYLOAD_ARG_CNT];

	/* Send request to the PMC */
	PM_PACK_PAYLOAD2(payload, PM_CLOCK_ENABLE, clk_id);

	return pm_ipi_send_sync(primary_proc, payload, NULL, 0);
}

/**
 * pm_clock_disable() - Disable the clock
 * @clk_id	Clock ID
 *
 * @return	Returns status, either success or error+reason
 */
enum pm_ret_status pm_clock_disable(uint32_t clk_id)
{
	uint32_t payload[PAYLOAD_ARG_CNT];

	/* Send request to the PMC */
	PM_PACK_PAYLOAD2(payload, PM_CLOCK_DISABLE, clk_id);

	return pm_ipi_send_sync(primary_proc, payload, NULL, 0);
}

/**
 * pm_clock_get_state() - Get clock status
 * @clk_id	Clock ID
 * @state:	Buffer to store clock status (1: Enabled, 0:Disabled)
 *
 * @return	Returns status, either success or error+reason
 */
enum pm_ret_status pm_clock_get_state(uint32_t clk_id, uint32_t *state)
{
	uint32_t payload[PAYLOAD_ARG_CNT];

	/* Send request to the PMC */
	PM_PACK_PAYLOAD2(payload, PM_CLOCK_GETSTATE, clk_id);

	return pm_ipi_send_sync(primary_proc, payload, state, 1);
}

/**
 * pm_clock_set_divider() - Set divider for the clock
 * @clk_id	Clock ID
 * @divider	Divider value
 *
 * @return	Returns status, either success or error+reason
 */
enum pm_ret_status pm_clock_set_divider(uint32_t clk_id, uint32_t divider)
{
	uint32_t payload[PAYLOAD_ARG_CNT];

	/* Send request to the PMC */
	PM_PACK_PAYLOAD3(payload, PM_CLOCK_SETDIVIDER, clk_id, divider);

	return pm_ipi_send_sync(primary_proc, payload, NULL, 0);
}

/**
 * pm_clock_get_divider() - Get divider value for the clock
 * @clk_id	Clock ID
 * @divider:	Buffer to store clock divider value
 *
 * @return	Returns status, either success or error+reason
 */
enum pm_ret_status pm_clock_get_divider(uint32_t clk_id, uint32_t *divider)
{
	uint32_t payload[PAYLOAD_ARG_CNT];

	/* Send request to the PMC */
	PM_PACK_PAYLOAD2(payload, PM_CLOCK_GETDIVIDER, clk_id);

	return pm_ipi_send_sync(primary_proc, payload, divider, 1);
}

/**
 * pm_clock_set_parent() - Set parent for the clock
 * @clk_id	Clock ID
 * @parent	Parent ID
 *
 * @return	Returns status, either success or error+reason
 */
enum pm_ret_status pm_clock_set_parent(uint32_t clk_id, uint32_t parent)
{
	uint32_t payload[PAYLOAD_ARG_CNT];

	/* Send request to the PMC */
	PM_PACK_PAYLOAD3(payload, PM_CLOCK_SETPARENT, clk_id, parent);

	return pm_ipi_send_sync(primary_proc, payload, NULL, 0);
}

/**
 * pm_clock_get_parent() - Get parent value for the clock
 * @clk_id	Clock ID
 * @parent:	Buffer to store clock parent value
 *
 * @return	Returns status, either success or error+reason
 */
enum pm_ret_status pm_clock_get_parent(uint32_t clk_id, uint32_t *parent)
{
	uint32_t payload[PAYLOAD_ARG_CNT];

	/* Send request to the PMC */
	PM_PACK_PAYLOAD2(payload, PM_CLOCK_GETPARENT, clk_id);

	return pm_ipi_send_sync(primary_proc, payload, parent, 1);
}

/**
 * pm_pll_set_param() - Set PLL parameter
 * @clk_id	PLL clock ID
 * @param	PLL parameter ID
 * @value	Value to set for PLL parameter
 *
 * @return	Returns status, either success or error+reason
 */
enum pm_ret_status pm_pll_set_param(uint32_t clk_id, uint32_t param,
				    uint32_t value)
{
	uint32_t payload[PAYLOAD_ARG_CNT];

	/* Send request to the PMC */
	PM_PACK_PAYLOAD4(payload, PM_PLL_SET_PARAMETER, clk_id, param, value);

	return pm_ipi_send_sync(primary_proc, payload, NULL, 0);
}

/**
 * pm_pll_get_param() - Get PLL parameter value
 * @clk_id	PLL clock ID
 * @param	PLL parameter ID
 * @value:	Buffer to store PLL parameter value
 *
 * @return	Returns status, either success or error+reason
 */
enum pm_ret_status pm_pll_get_param(uint32_t clk_id, uint32_t param,
				    uint32_t *value)
{
	uint32_t payload[PAYLOAD_ARG_CNT];

	/* Send request to the PMC */
	PM_PACK_PAYLOAD3(payload, PM_PLL_GET_PARAMETER, clk_id, param);

	return pm_ipi_send_sync(primary_proc, payload, value, 1);
}

/**
 * pm_pll_set_mode() - Set PLL mode
 * @clk_id	PLL clock ID
 * @mode	PLL mode
 *
 * @return	Returns status, either success or error+reason
 */
enum pm_ret_status pm_pll_set_mode(uint32_t clk_id, uint32_t mode)
{
	uint32_t payload[PAYLOAD_ARG_CNT];

	/* Send request to the PMC */
	PM_PACK_PAYLOAD3(payload, PM_PLL_SET_MODE, clk_id, mode);

	return pm_ipi_send_sync(primary_proc, payload, NULL, 0);
}

/**
 * pm_pll_get_mode() - Get PLL mode
 * @clk_id	PLL clock ID
 * @mode:	Buffer to store PLL mode
 *
 * @return	Returns status, either success or error+reason
 */
enum pm_ret_status pm_pll_get_mode(uint32_t clk_id, uint32_t *mode)
{
	uint32_t payload[PAYLOAD_ARG_CNT];

	/* Send request to the PMC */
	PM_PACK_PAYLOAD2(payload, PM_PLL_GET_MODE, clk_id);

	return pm_ipi_send_sync(primary_proc, payload, mode, 1);
}