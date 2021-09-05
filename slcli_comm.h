#ifndef __SLCLI_COMM_H_____
#define __SLCLI_COMM_H_____

#include "slcli_cfg.h"
#include "stdint.h"

#include "sl_log.h"

#define SLCLI_LOG               console_print

#define SLCLI_PATTERN_NUM       0xEA16U

struct slcli_st;

typedef int (*Fslcli_getchar)(char *inbuf);
typedef int (*Fslcli_put_array)(char *out,uint32_t length);

/** Structure for registering CLI commands */
struct slcli_command
{
    /** The name of the CLI command */
    const char *name;
    /** The help text associated with the command */
    const char *help;
    /** The function that should be invoked for this command. */
    int (*function) (struct slcli_st *env, int argc, char **argv);
};

typedef struct slcli_st
{
	uint16_t initialized;
	uint16_t status;
#define SLCLI_INIT_PATTERN    0x0000U
#define SLCLI_WORK_PATTERN    0xE151U
#define SLCLI_EXIT_PATTERN    0x15E6U
#define SLCLI_DEINIT_PATTERN  0x15E6U

	char *name;
	uint32_t flag;
#define SLCLI_NAME_STR_MALLOC      (1U << 31)
#define SLCLI_ECHO_FLAG            (1U << 30)

	uint16_t num_commands;
	uint16_t def_num_commands;
	const struct slcli_command *commands[SLCLI_MAX_COMMANDS];
	const struct slcli_command *def_commands;

	void *lock;
	void *sema;

	uint32_t input_bp;	/* buffer pointer */
	uint32_t output_len;
	uint8_t *inbuf_ptr;
	uint8_t *output_ptr;
    char inbuf[SLCLI_INBUF_SIZE];
    char outbuf[SLCLI_OUTBUF_SIZE];

	Fslcli_getchar _getchar;
	Fslcli_put_array _put_array;
}slcli_st_t;



typedef enum{
	SLCLI_SUCCEED = 0,
	SLCLI_ERROR,
	SLCLI_ENV_IS_NULL,
	SLCLI_IS_UN_PATTERN,
	SLCLI_NO_MEMORY_ERROR,

	SLCLI_DEF_CMD_REG_ERROR,
	SLCLI_SEM_TIMEOUT,

	SLCLI_SYNTAR_ERROR = 0x8791,   ///syntax
	SLCLI_BAD_COMMAND_ERROR,

	SLCLI_CTXT_QUIT = 0xF08101,    ///退出当前 SLCLI
}SLCLI_STATE_E;

#if defined(SLCLI_LOG_CFG) && (!SLCLI_LOG_CFG)
#undef SLCLI_LOG
#endif

#ifndef SLCLI_LOG
#define SLCLI_LOG(...)
#endif


#ifndef SLCLI_SEM_NUM
#define SLCLI_SEM_NUM              5
#endif

#define SLCLI_PROMPT			              "\r\n# "
#define SLCLI_RET_CHAR                        '\n'
#define SLCLI_END_CHAR		                  '\r'

#define slcli_is_print(c)           ((uint8_t)c >= 0x20 && (uint8_t)c <= 0x7f)

#ifndef SLCLI_PRINTF_BUF_SIZE
#define SLCLI_PRINTF_BUF_SIZE        512
#endif

#ifndef SLCLI_ARG_NUM
#define SLCLI_ARG_NUM                16
#endif


#define SLCLI_ARGV1_CMP(str)         (argc > 1 && strcmp(argv[1], str) == 0)

#define SLCLI_SHOW_ARGV1()           slcli_printf(slcli, 0, "arg[1]:%s\r\n", (argc > 1 && argv[1])? argv[1] : "")

#endif

