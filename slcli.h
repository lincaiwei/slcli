#ifndef __SLCLI__H_____
#define __SLCLI__H_____

#include "slcli_cfg.h"
#include "slcli_comm.h"



extern int slcli_schedule(struct slcli_st* slcli,uint32_t timeout);

extern struct slcli_st* slcli_init(char*name,int *status);
extern int slcli_deinit(struct slcli_st* slcli);

extern int slcli_register_port_api(struct slcli_st* slcli,Fslcli_getchar _getchar,Fslcli_put_array _put_array);
extern int slcli_set_work_status(struct slcli_st* slcli, uint16_t status);
extern uint16_t slcli_get_work_status(struct slcli_st* slcli);

extern int slcli_set_name(struct slcli_st* slcli, char* name);
extern int slcli_set_alloc_name(struct slcli_st* slcli, char* name);
extern char* slcli_get_name(struct slcli_st* slcli);

extern int slcli_set_semaphore(struct slcli_st* slcli);

extern void slcli_printf(struct slcli_st *slcli,uint32_t buf_len,const char *fmt, ...);
extern void slcli_output_array(struct slcli_st *slcli,uint32_t buf_len,char *msg,uint32_t msg_len);

#endif
