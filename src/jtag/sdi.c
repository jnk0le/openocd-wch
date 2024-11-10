/* SPDX-License-Identifier: GPL-2.0-or-later */

/*
 
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "interface.h"

#include <helper/command.h>
#include <transport/transport.h>

extern struct adapter_driver *adapter_driver;

int transfer(unsigned long iIndex, unsigned char iAddr,unsigned long iData,unsigned char iOP,unsigned char *oAddr,unsigned long *oData,unsigned char *oOP)
{
	assert(adapter_driver->sdi_ops);

	return adapter_driver->sdi_ops->transfer(iIndex,iAddr,iData,iOP,oAddr,oData,oOP);
}

COMMAND_HANDLER(handle_sdi_newtap_command)
{
	struct jtag_tap *tap;

	/*
	 * only need "basename" and "tap_type", but for backward compatibility
	 * ignore extra parameters
	 */
	if (CMD_ARGC < 2)
		return ERROR_COMMAND_SYNTAX_ERROR;

	tap = calloc(1, sizeof(*tap));
	if (!tap) {
		LOG_ERROR("Out of memory");
		return ERROR_FAIL;
	}

	tap->chip = strdup(CMD_ARGV[0]);
	tap->tapname = strdup(CMD_ARGV[1]);
	tap->dotted_name = alloc_printf("%s.%s", CMD_ARGV[0], CMD_ARGV[1]);
	if (!tap->chip || !tap->tapname || !tap->dotted_name) {
		LOG_ERROR("Out of memory");
		free(tap->dotted_name);
		free(tap->tapname);
		free(tap->chip);
		free(tap);
		return ERROR_FAIL;
	}

	LOG_DEBUG("Creating new sdi \"tap\", Chip: %s, Tap: %s, Dotted: %s",
			  tap->chip, tap->tapname, tap->dotted_name);

	/* default is enabled-after-reset */
	tap->enabled = true;

	jtag_tap_init(tap);
	return ERROR_OK;
}

static const struct command_registration sdi_transport_subcommand_handlers[] = {
	{
		.name = "newtap",
		.handler = handle_sdi_newtap_command,
		.mode = COMMAND_CONFIG,
		.help = "Create a new TAP instance named basename.tap_type, "
				"and appends it to the scan chain.",
		.usage = "basename tap_type",
	},
	COMMAND_REGISTRATION_DONE
};

static const struct command_registration sdi_transport_command_handlers[] = {
	{
		.name = "sdi",
		.mode = COMMAND_ANY,
		.help = "perform sdi adapter actions",
		.usage = "",
		.chain = sdi_transport_subcommand_handlers,
	},
	COMMAND_REGISTRATION_DONE
};

static int sdi_transport_select(struct command_context *cmd_ctx)
{
	LOG_DEBUG(__func__);

	return register_commands(cmd_ctx, NULL, sdi_transport_command_handlers);
}

static int sdi_transport_init(struct command_context *cmd_ctx)
{
	// enum reset_types jtag_reset_config = jtag_get_reset_config();

	// LOG_DEBUG(__func__);

	// if (jtag_reset_config & RESET_CNCT_UNDER_SRST) {
	// 	if (jtag_reset_config & RESET_SRST_NO_GATING)
	// 		adapter_assert_reset();
	// 	else
	// 		LOG_WARNING("\'srst_nogate\' reset_config option is required");
	// } else
	// 	adapter_deassert_reset();

	return ERROR_OK;
}

static struct transport sdi_transport = {
	.name = "sdi",
	.select = sdi_transport_select,
	.init = sdi_transport_init,
};
const char *sdi_transports[] = { "sdi", NULL };
static void sdi_constructor(void) __attribute__ ((constructor));
static void sdi_constructor(void)
{
	transport_register(&sdi_transport);
}

bool transport_is_sdi(void)
{
	return get_current_transport() == &sdi_transport;
}
