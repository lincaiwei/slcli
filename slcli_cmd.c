#include "slcli_cmd.h"
#include "string.h"
#include "slcli.h"
#include "stdio.h"
#include "stdlib.h"
#include "stdarg.h"

#include "slcli_port.h"

static int help_command_handler(struct slcli_st *slcli, int argc, char **argv)
{
	uint32_t i, n;
	uint32_t length = 0;
	char *output = slcli->outbuf;
	uint32_t out_len = SLCLI_OUTBUF_SIZE;

	if(slcli == NULL){
		return 0;
	}
	slcli->output_len = 0;

#if 1
	length += snprintf(slcli->outbuf,out_len - length,
							"====SLCLI def Commands====\r\n");
#else
	SLCLI_LOG( "====SLCLI def Commands====\r\n" );
#endif
    for (i = 0; i < slcli->def_num_commands; i++)
    {
        if (slcli->def_commands[i].name)
        {
        #if 0
            SLCLI_LOG("%s: %s\r\n", slcli->def_commands[i].name,
                      slcli->def_commands[i].help ?
                      slcli->def_commands[i].help : "");
		#else
			length += snprintf(&slcli->outbuf[length], out_len - length,
							"%s: %s\r\n", slcli->def_commands[i].name,
                      slcli->def_commands[i].help ?
                      slcli->def_commands[i].help : "");
		#endif
        }
    }

#if 0
	SLCLI_LOG("\r\n====SLCLI User Commands====\r\n");
#else
	length += snprintf(&slcli->outbuf[length],out_len - length,
							"\r\n====SLCLI User Commands====\r\n");
#endif
	for (i = 0, n = 0; i < SLCLI_MAX_COMMANDS && n < slcli->num_commands; i++)
    {
        if (slcli->commands[i]->name)
        {
        #if 0
            SLCLI_LOG("%s: %s\r\n", slcli->commands[i]->name,
                      slcli->commands[i]->help ?
                      slcli->commands[i]->help : "");
		#else
			length += snprintf(&slcli->outbuf[length],out_len - length,
							"%s: %s\r\n", slcli->commands[i]->name,
                      slcli->commands[i]->help ?
                      slcli->commands[i]->help : "");
		#endif
            n++;
        }
    }

#if 0
	SLCLI_LOG("\r\n==== SLCLI Commands End ====\r\n");
#else
	length += snprintf(&slcli->outbuf[length], out_len - length,
							"\r\n==== SLCLI Commands End ====\r\n");
#endif
	(void)output;
	(void)out_len;
	slcli->output_len = length;

	return 0;
}

static int exit_command_handler(struct slcli_st *slcli, int argc, char **argv)
{
	uint32_t length = 0;
	uint32_t out_len = SLCLI_OUTBUF_SIZE;

	length = snprintf(slcli->outbuf, out_len - length,
							"\r\n==== SLCLI(%p) %s exit ====\r\n",
                 		 slcli,slcli->name ? slcli->name : "");
	slcli->output_len = length;

	return SLCLI_CTXT_QUIT;
}

static int echo_cmd_handler(struct slcli_st *slcli, int argc, char **argv)
{
	uint32_t length = 0;
	char *out = slcli->outbuf;
	uint32_t out_len = SLCLI_OUTBUF_SIZE;
    if (argc == 1) {
		length = snprintf(slcli->outbuf,out_len,
							"Usage: echo on/off. Echo is currently %s\r\n",
                 		 (slcli->flag & SLCLI_ECHO_FLAG) ? "Enabled" : "Disabled");
		slcli->output_len = length;
        return SLCLI_ERROR;
    }

    if (!strcasecmp(argv[1], "on")){
		length += snprintf(&out[length],out_len - length,"Enable echo\r\n");
		slcli->flag |= SLCLI_ECHO_FLAG;
    }
    else if (!strcasecmp(argv[1], "off")){
		length += snprintf(&out[length],out_len - length,"Disable echo\r\n");
		slcli->flag &= ~SLCLI_ECHO_FLAG;
    }
	slcli->output_len = length;

	return SLCLI_SUCCEED;
}


static const struct slcli_command def_comm_tab[] = {
	{.name = "help",.help = "help", .function = help_command_handler},
	{.name = "exit",.help = "exit", .function = exit_command_handler},
	{.name = "echo",.help = "echo", .function = echo_cmd_handler},
};




int slcli_register_command(struct slcli_st *slcli, const struct slcli_command *command)
{
    int i;
    if (!command->name || !command->function)
        return 1;

    if (slcli->num_commands < SLCLI_MAX_COMMANDS)
    {
        /* Check if the command has already been registered.
        * Return 0, if it has been registered.
        */
        for (i = 0; i < slcli->num_commands; i++)
        {
            if (slcli->commands[i] == command)
                return 0;
        }
        slcli->commands[slcli->num_commands++] = command;
        return SLCLI_SUCCEED;
    }

    return SLCLI_ERROR;
}


int slcli_register_commands(struct slcli_st *slcli,const struct slcli_command *commands, int num_commands)
{
    int i;

    for (i = 0; i < num_commands; i++){
        if (slcli_register_command(slcli, commands++)){
            return SLCLI_ERROR;
        }
    }
    return SLCLI_SUCCEED;
}

int slcli_register_def_commands(struct slcli_st *slcli,const struct slcli_command *commands, uint32_t num_commands)
{
	if ( slcli ) {
		slcli->def_commands = commands;
		slcli->def_num_commands = num_commands;
	}

    return SLCLI_SUCCEED;
}


int slcli_register_cmd_def_commands(struct slcli_st *slcli)
{
	return slcli_register_def_commands(slcli,def_comm_tab,sizeof(def_comm_tab)/sizeof(struct slcli_command));
}


void slcli_printf(struct slcli_st *slcli,uint32_t buf_len,const char *fmt, ...)
{
	va_list args;
	char *string;
	uint32_t length;

	if(slcli == NULL){
		return;
	}

	if (buf_len > SLCLI_OUTBUF_SIZE) {
		if (slcli->output_ptr == NULL) {
			slcli->output_ptr = slcli_malloc(buf_len);
			if (slcli->output_ptr == NULL) {
				buf_len = SLCLI_OUTBUF_SIZE;
			}else{
				if(slcli->output_len > 0){
					if(slcli->output_len <= buf_len){
						memcpy(slcli->output_ptr, slcli->outbuf, slcli->output_len);
					}else{
						memcpy(slcli->output_ptr, slcli->outbuf, buf_len);
					}
				}
				slcli->outbuf[0] = buf_len & 0xFFU;
				slcli->outbuf[1] = (buf_len >> 8) & 0xFFU;
				slcli->outbuf[2] = (buf_len >> 16) & 0xFFU;
				slcli->outbuf[3] = (buf_len >> 24) & 0xFFU;
			}
		}else{
			uint32_t cur_buf_len;
			cur_buf_len = slcli->outbuf[0] | (slcli->outbuf[1] << 8);
			cur_buf_len |= (slcli->outbuf[2] << 16);
			cur_buf_len |= (slcli->outbuf[3] << 24);
			if(cur_buf_len < buf_len){
				uint8_t *output_ptr = slcli->output_ptr;
				slcli->output_ptr = slcli_malloc(buf_len);
				if (slcli->output_ptr == NULL) {
					if (output_ptr) {
						slcli->output_ptr = output_ptr;
					}
				}else {
					if(slcli->output_len <= buf_len){
						memcpy(slcli->output_ptr, output_ptr, slcli->output_len);
					}else{
						memcpy(slcli->output_ptr, output_ptr, buf_len);
					}
					slcli_free(output_ptr);
					slcli->outbuf[0] = buf_len & 0xFFU;
					slcli->outbuf[1] = (buf_len >> 8) & 0xFFU;
					slcli->outbuf[2] = (buf_len >> 16) & 0xFFU;
					slcli->outbuf[3] = (buf_len >> 24) & 0xFFU;
				}
			}
		}
	}

	length = slcli->output_len;

	if (slcli->output_ptr) {
		string = (char *)slcli->output_ptr;
		buf_len = slcli->outbuf[0] | (slcli->outbuf[1] << 8);
		buf_len |= (slcli->outbuf[2] << 16);
		buf_len |= (slcli->outbuf[3] << 24);
	}else{
		string = slcli->outbuf;
		buf_len = SLCLI_OUTBUF_SIZE;
	}

	if (buf_len <= length) {
		return;
	}

	va_start(args, fmt);
	length += vsnprintf(&string[length], buf_len - length, fmt, args);
	va_end(args);
	console_print("%s %d\r\n",__FUNCTION__,__LINE__);

	slcli->output_len = length;
}


void slcli_output_array(struct slcli_st *slcli,uint32_t buf_len,char *msg,uint32_t msg_len)
{
	char *string;
	uint32_t length;

	if(slcli == NULL){
		return;
	}

	if (buf_len > SLCLI_OUTBUF_SIZE) {
		if (slcli->output_ptr == NULL) {
			slcli->output_ptr = slcli_malloc(buf_len);
			if (slcli->output_ptr == NULL) {
				buf_len = SLCLI_OUTBUF_SIZE;
			}else{
				if(slcli->output_len > 0){
					if(slcli->output_len <= buf_len){
						memcpy(slcli->output_ptr, slcli->outbuf, slcli->output_len);
					}else{
						memcpy(slcli->output_ptr, slcli->outbuf, buf_len);
					}
				}
				slcli->outbuf[0] = buf_len & 0xFFU;
				slcli->outbuf[1] = (buf_len >> 8) & 0xFFU;
				slcli->outbuf[2] = (buf_len >> 16) & 0xFFU;
				slcli->outbuf[3] = (buf_len >> 24) & 0xFFU;
			}
		}else{
			uint32_t cur_buf_len;
			cur_buf_len = slcli->outbuf[0] | (slcli->outbuf[1] << 8);
			cur_buf_len |= (slcli->outbuf[2] << 16);
			cur_buf_len |= (slcli->outbuf[3] << 24);
			if(cur_buf_len < buf_len){
				uint8_t *output_ptr = slcli->output_ptr;
				slcli->output_ptr = slcli_malloc(buf_len);
				if (slcli->output_ptr == NULL) {
					if (output_ptr) {
						slcli->output_ptr = output_ptr;
					}
				}else {
					if(slcli->output_len <= buf_len){
						memcpy(slcli->output_ptr, output_ptr, slcli->output_len);
					}else{
						memcpy(slcli->output_ptr, output_ptr, buf_len);
					}
					slcli_free(output_ptr);
					slcli->outbuf[0] = buf_len & 0xFFU;
					slcli->outbuf[1] = (buf_len >> 8) & 0xFFU;
					slcli->outbuf[2] = (buf_len >> 16) & 0xFFU;
					slcli->outbuf[3] = (buf_len >> 24) & 0xFFU;
				}
			}
		}
	}

	length = slcli->output_len;

	if (slcli->output_ptr) {
		string = (char *)slcli->output_ptr;
		buf_len = slcli->outbuf[0] | (slcli->outbuf[1] << 8);
		buf_len |= (slcli->outbuf[2] << 16);
		buf_len |= (slcli->outbuf[3] << 24);
	}else{
		string = slcli->outbuf;
		buf_len = SLCLI_OUTBUF_SIZE;
	}

	if (buf_len <= length) {
		return;
	}

	uint32_t cp_len = buf_len - length;

	if(cp_len > msg_len){
		cp_len = msg_len;
	}
	memcpy(&string[length], msg, cp_len);

	slcli->output_len = length + cp_len;
}



