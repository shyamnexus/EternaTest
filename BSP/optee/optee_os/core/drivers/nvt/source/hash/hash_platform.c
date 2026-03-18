#include <platform_config.h>
#include <kernel/mutex.h>
#include "hash_platform.h"

static int is_create = 0;
static struct mutex SEMID_HASH;

void hash_platform_create_resource(void)
{
	if (!is_create) {
		mutex_init(&SEMID_HASH);
		is_create = 1;
	}
}

void hash_platform_release_resource(void)
{
	if (is_create) {
		mutex_destroy(&SEMID_HASH);
		is_create = 0;
	}
}

ER hash_platform_sem_wait(void)
{
	mutex_lock(&SEMID_HASH);
	return E_OK;
}

ER hash_platform_sem_signal(void)
{
	mutex_unlock(&SEMID_HASH);
	return E_OK;
}
