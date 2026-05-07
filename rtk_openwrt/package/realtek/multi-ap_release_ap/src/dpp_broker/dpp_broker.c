#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dpp_datamodel.h"
#include "dpp_relay.h"
#include "eloop.h"
#include "easymesh_relay.h"
#include "dpp_utils.h"

static void handle_term(int sig, void *signal_ctx)
{
	dpp_printf("Signal %d received - terminating\n", sig);
	eloop_terminate();
}


struct dpp_global * dpp_global_init()
{
	struct dpp_global *dpp;

	dpp = malloc(sizeof(*dpp));
	memset(dpp, 0, sizeof(*dpp));

	if (!dpp)
		return NULL;

	dl_list_init(&dpp->enrollee);

	return dpp;
}

int main(void)
{
	if (eloop_init())
		return -1;

	struct dpp_global *dpp = dpp_global_init();

	eloop_register_signal_terminate(handle_term, NULL);

	dpp_relay_init(dpp);

	easymesh_relay_init(dpp);

	eloop_run();

	eloop_destroy();
	return 0;
}