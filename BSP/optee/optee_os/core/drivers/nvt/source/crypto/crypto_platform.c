#include <platform_config.h>
#include <kernel/mutex.h>
#include "crypto_platform.h"

static int is_create = 0;
static struct mutex SEMID_CRYPTO;

void crypto_platform_create_resource(void)
{
	if (!is_create) {
		mutex_init(&SEMID_CRYPTO);
		is_create = 1;
	}
}

void crypto_platform_release_resource(void)
{
	if (is_create) {
		mutex_destroy(&SEMID_CRYPTO);
		is_create = 0;
	}
}

ER crypto_platform_sem_wait(void)
{
	mutex_lock(&SEMID_CRYPTO);
	return E_OK;
}

ER crypto_platform_sem_signal(void)
{
	mutex_unlock(&SEMID_CRYPTO);
	return E_OK;
}
