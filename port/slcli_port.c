#include "slcli_port.h"
#include "rtos_pub.h"
#include "string.h"

#ifndef SLCLI_WEAK
#define SLCLI_WEAK __attribute__((weak))
#endif


int SLCLI_WEAK slcli_lock_init(void **lock)
{
	sl_mutex_t mutex;

	memset(&mutex, 0, sizeof(sl_mutex_t));
	if(rtos_init_mutex(&mutex) != knNoErr){
		return SLCLI_ERROR;
	}
	*lock = mutex;

	return SLCLI_SUCCEED;
}

int SLCLI_WEAK slcli_lock(void **lock)
{
	sl_mutex_t mutex = *lock;

	if(rtos_lock_mutex(&mutex) != knNoErr){
		return SLCLI_ERROR;
	}

	return SLCLI_SUCCEED;
}

int SLCLI_WEAK slcli_lock_timeout(void **lock, uint32_t timeout)
{
	sl_mutex_t mutex = *lock;

	if(rtos_lock_mutex_times(&mutex,timeout) != knNoErr){
		return SLCLI_ERROR;
	}

	return SLCLI_SUCCEED;
}

int SLCLI_WEAK slcli_lock_deinit(void **lock)
{
	sl_mutex_t mutex = *lock;

	if(rtos_deinit_mutex(&mutex) != knNoErr){
		return SLCLI_ERROR;
	}
	*lock = NULL;

	return SLCLI_SUCCEED;
}


int SLCLI_WEAK slcli_unlock(void **lock)
{
	sl_mutex_t mutex = *lock;

	if(rtos_unlock_mutex(&mutex) != knNoErr){
		return SLCLI_ERROR;
	}

	return SLCLI_SUCCEED;
}



void* SLCLI_WEAK slcli_malloc(uint32_t size)
{
	return rtos_malloc(size);
}

void SLCLI_WEAK slcli_free(void* ptr)
{
	rtos_free(ptr);
}

int SLCLI_WEAK slcli_sem_init(void **sem)
{
	sl_semaphore_t _sem;
	int ret;
	memset(&_sem, 0, sizeof(sl_semaphore_t));

	ret = rtos_init_semaphore(&_sem, SLCLI_SEM_NUM);
	if(ret != knNoErr){
		return SLCLI_ERROR;
	}

	if(sem) {
		*sem = _sem;
	}else{
		rtos_deinit_semaphore(&_sem);
		return SLCLI_ERROR;
	}

	return SLCLI_SUCCEED;
}

int SLCLI_WEAK slcli_set_sem(void **sem)
{
	sl_semaphore_t _sem = *sem;
	int ret;

	ret = rtos_set_semaphore(&_sem);
	if(ret != knNoErr){
		return SLCLI_ERROR;
	}

	return SLCLI_SUCCEED;
}

int SLCLI_WEAK slcli_set_sem_from_isr(void **sem)
{
	sl_semaphore_t _sem = *sem;
	int ret;

	ret = rtos_set_semaphore_flag(&_sem, RTOS_INIT);
	if(ret != knNoErr){
		return SLCLI_ERROR;
	}

	return SLCLI_SUCCEED;
}


int SLCLI_WEAK slcli_get_sem_timeout(void **sem,uint32_t timeout)
{
	sl_semaphore_t _sem = *sem;
	int ret;

	ret = rtos_get_semaphore(&_sem, timeout);
	if(ret != knNoErr){
		return SLCLI_ERROR;
	}

	return SLCLI_SUCCEED;
}

int SLCLI_WEAK slcli_sem_deinit(void **sem)
{
	sl_semaphore_t _sem = *sem;
	int ret;

	ret = rtos_deinit_semaphore(&_sem);
	if(ret != knNoErr){
		return SLCLI_ERROR;
	}

	return SLCLI_SUCCEED;
}


int SLCLI_WEAK slcli_getchar(char *inbuf)
{
    return 0;
}

int SLCLI_WEAK slcli_put_array(char *out,uint32_t len)
{
    return 0;
}

