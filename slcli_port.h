#ifndef __SLCLI_PORT_H__
#define __SLCLI_PORT_H__

#include "stdint.h"
#include "slcli_cfg.h"
#include "slcli_comm.h"


extern void* slcli_malloc(uint32_t size);
extern void slcli_free(void* ptr);

extern int slcli_lock_init(void **lock);
extern int slcli_lock(void **lock);
extern int slcli_lock_timeout(void **lock, uint32_t timeout);
extern int slcli_unlock(void **lock);
extern int slcli_lock_deinit(void **lock);


extern int slcli_sem_init(void **sem);
extern int slcli_get_sem_timeout(void **sem,uint32_t timeout);
extern int slcli_set_sem(void **sem);
extern int slcli_set_sem_from_isr(void **sem);
extern int slcli_sem_deinit(void **sem);

#endif

