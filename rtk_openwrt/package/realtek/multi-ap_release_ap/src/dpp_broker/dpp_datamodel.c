#include <stdint.h>
#include <string.h>
#include "dpp_datamodel.h"

struct dpp_enrollee_ctx *dpp_find_ctx_by_chirp(struct dpp_global *dpp, const uint8_t *hash)
{
	struct dpp_enrollee_ctx *ctx;

	if (!dpp)
		return NULL;

	dl_list_for_each(ctx, &dpp->enrollee, struct dpp_enrollee_ctx, list) {
		if (memcmp(ctx->pubkey_hash_chirp, hash, SHA256_MAC_LEN) == 0)
			return ctx;
	}

	return NULL;
}