#ifndef __SLCLI_CMD_H_____
#define __SLCLI_CMD_H_____

#include "slcli_cfg.h"
#include "stdint.h"
#include "slcli_comm.h"

extern int slcli_register_command(struct slcli_st *slcli, const struct slcli_command *command);
extern int slcli_register_commands(struct slcli_st *slcli,
								const struct slcli_command *commands, int num_commands);



extern int slcli_register_def_commands(struct slcli_st *slcli,
									const struct slcli_command *commands, uint32_t num_commands);

extern int slcli_register_cmd_def_commands(struct slcli_st *slcli);

extern void slcli_printf(struct slcli_st *slcli,uint32_t buf_len,const char *fmt, ...);
extern void slcli_output_array(struct slcli_st *slcli,uint32_t buf_len,char *msg,uint32_t msg_len);


#endif
