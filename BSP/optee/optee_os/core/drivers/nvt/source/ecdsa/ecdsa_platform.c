#include <kwrap/semaphore.h>
#include <kwrap/flag.h>
#include <kwrap/spinlock.h>
#include <kwrap/nvt_type.h>
#include <kwrap/error_no.h>

#include "ecdsa_platform.h"

static int is_create = 0;
static struct mutex SEMID_ECDSA;

void ecdsa_platform_create_resource(void)
{
	if (!is_create) {
		mutex_init(&SEMID_ECDSA);
		is_create = 1;
	}
}

void ecdsa_platform_release_resource(void)
{
	if (is_create) {
		mutex_destroy(&SEMID_ECDSA);
		is_create = 0;
	}
}

ER ecdsa_platform_sem_wait(void)
{
	mutex_lock(&SEMID_ECDSA);
	return E_OK;
}

ER ecdsa_platform_sem_signal(void)
{
	mutex_unlock(&SEMID_ECDSA);
	return E_OK;
}
