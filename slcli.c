#include "slcli.h"
#include "slcli_comm.h"
#include "slcli_port.h"
#include "string.h"
#include "slcli_cmd.h"
#include "stdarg.h"
#include "stdlib.h"
#include "stdio.h"





static int _slcli_getchar(struct slcli_st* slcli,char *inbuf)
{
	if(slcli && slcli->_getchar) {
		return slcli->_getchar(inbuf);
	}
	return 0;
}

static int _slcli_put_array(struct slcli_st* slcli,char *out,uint32_t len)
{
	if(slcli && slcli->_put_array) {
		return slcli->_put_array(out, len);
	}
	return 0;
}

static int _slcli_put_string(struct slcli_st* slcli,char *str)
{
	return _slcli_put_array(slcli,str,strlen(str));
}

static int _slcli_put_char(struct slcli_st* slcli,char str)
{
	return _slcli_put_array(slcli,&str,1);
}

static void _slcli_printf(struct slcli_st* slcli,const char *fmt, ...)
{
	va_list args;
#if SLCLI_PRINTF_BUF_SIZE <= 1024
	static char string[SLCLI_PRINTF_BUF_SIZE + 2];
	uint32_t buf_size =  sizeof(string);
#else
	char *string = NULL;
	int32_t buf_size =  SLCLI_PRINTF_BUF_SIZE + 2;
	string = slcli_malloc(buf_size);
	if(string == NULL){
		return;
	}
#endif
	long length;

	va_start(args, fmt);
	///vsprintf(string, fmt, args);
	length = vsnprintf(string, buf_size - 1, fmt, args);
	if (length > (buf_size - 1))
		length = buf_size - 1;
	string[buf_size - 1] = '\0';
	va_end(args);

	_slcli_put_array(slcli,string,length);
#if SLCLI_PRINTF_BUF_SIZE > 1024
	if(string){
		slcli_free(string);
	}
#endif
}

/*  Perform basic tab-completion on the input buffer by string-matching the
*   current input line against the cli functions table.  The current input line
*   is assumed to be NULL-terminated. 
*/
static void slcli_tab_complete(struct slcli_st* slcli,char *inbuf, uint32_t *bp)
{
    int i, n, m = 0;
    const char *fm = NULL;

    SLCLI_LOG("\r\n");

    /* show matching commands */
	for(i = 0; i < slcli->def_num_commands; i++) {
		if (slcli->def_commands[i].name != NULL) {
			if (strncmp(inbuf, slcli->def_commands[i].name, *bp)) {
				m++;
				if (m == 1) {
					fm = slcli->def_commands[i].name;
				}else if (m == 2) {
					_slcli_printf(slcli,"%s %s ", fm,slcli->def_commands[i].name);
				}else{
					_slcli_printf(slcli,"%s ",slcli->def_commands[i].name);
				}
			}
		}
	}

    for (i = 0, n = 0; i < SLCLI_MAX_COMMANDS && n < slcli->num_commands; i++) {
        if (slcli->commands[i]->name != NULL) {
            if (!strncmp(inbuf, slcli->commands[i]->name, *bp)) {
                m++;
                if (m == 1) {
                    fm = slcli->commands[i]->name;
                }else if (m == 2) {
                    _slcli_printf(slcli,"%s %s ", fm, slcli->commands[i]->name);
                }else {
                    _slcli_printf(slcli,"%s ", slcli->commands[i]->name);
                }
            }
            n++;
        }
    }

    /* there's only one match, so complete the line */
    if (m == 1 && fm) {
        n = strlen(fm) - *bp;
        if (*bp + n < SLCLI_INBUF_SIZE) {
            memcpy(inbuf + *bp, fm + *bp, n);
            *bp += n;
            inbuf[(*bp)++] = ' ';
            inbuf[*bp] = '\0';
        }
    }

    /* just redraw input line */
    _slcli_printf(slcli,"%s%s", SLCLI_PROMPT, inbuf);
}

static int slcli_get_input(struct slcli_st* slcli,char *inbuf, uint32_t *bp)
{
	if ( inbuf == NULL ) {
		return 0;
	}

	while (_slcli_getchar(slcli, &inbuf[*bp]) == 1)
	{
		///console_print("slcli_get_input: %x\n",(uint8_t)inbuf[*bp]);
		if (inbuf[*bp] == SLCLI_RET_CHAR) {
			continue;
		}
		if (inbuf[*bp] == SLCLI_END_CHAR) {   /* end of input line */
			inbuf[*bp] = '\0';
			*bp = 0;
			return 1;
		}

		if ((inbuf[*bp] == 0x08) || /* backspace */
			(inbuf[*bp] == 0x7f)) { /* DEL */
			if (*bp > 0) {
				(*bp)--;
				if (slcli->flag & SLCLI_ECHO_FLAG){
					_slcli_printf(slcli,"%c %c", 0x08, 0x08);
				}
			}
			continue;
		}

		if (inbuf[*bp] == '\t') {
			inbuf[*bp] = '\0';
			slcli_tab_complete(slcli,inbuf, bp);
			continue;
		}

		if (slcli->flag & SLCLI_ECHO_FLAG){
			_slcli_printf(slcli,"%c", inbuf[*bp]);
		}

		(*bp)++;
		if (*bp >= SLCLI_INBUF_SIZE) {
			_slcli_printf(slcli,"Warn: input buffer overflow\r\n");
			_slcli_printf(slcli,SLCLI_PROMPT);
			*bp = 0;
			return 0;
		}
	}

	return 0;
}

/* Print out a bad command string, including a hex
*  representation of non-printable characters.
*  Non-printable characters show as "\0xXX".
*/
static void slble_print_bad_command(struct slcli_st* slcli,char *cmd_string)
{
	if (cmd_string != NULL) {
		char *c = cmd_string;

		_slcli_put_string(slcli, "COMMAND '");
		while (*c != '\0')
		{
			if (slcli_is_print(*c)){
				_slcli_put_char(slcli, *c);
			}
			else{
				_slcli_put_char(slcli, *c);
			}
			++c;
		}
		_slcli_put_string(slcli,"' not found\r\n");
	}
}


/* Find the command 'name' in the cli commands table.
* If len is 0 then full match will be performed else upto len bytes.
* Returns: a pointer to the corresponding cli_command struct or NULL.
*/
static const struct slcli_command *lookup_slcli_command_tab(struct slcli_st* slcli,char *name, int len)
{
	int i = 0, n = 0;

	while (i < SLCLI_MAX_COMMANDS && n < slcli->num_commands)
	{
		if (slcli->commands[i]->name == NULL){
			i++;
			continue;
		}
		/* See if partial or full match is expected */
		if (len != 0){
			if (!strncmp(slcli->commands[i]->name, name, len)){
				return slcli->commands[i];
			}
		}
		else{
			if (!strcmp(slcli->commands[i]->name, name)){
				return slcli->commands[i];
			}
		}

		i++;
		n++;
	}

	i = 0;
	while (i < slcli->def_num_commands) {
		if (slcli->def_commands[i].name == NULL) {
			break;
		}
		/* See if partial or full match is expected */
		if (len != 0){
			if (!strncmp(slcli->def_commands[i].name, name, len))
				return &slcli->def_commands[i];
		}
		else{
			if (!strcmp(slcli->def_commands[i].name, name))
				return &slcli->def_commands[i];
		}

		i++;
	}

	return NULL;
}

/* Parse input line and locate arguments (if any), keeping count of the number
* of arguments and their locations.  Look up and call the corresponding cli
* function if one is found and pass it the argv array.
*
* Returns: 0 on success: the input line contained at least a function name and
*          that function exists and was called.
*          1 on lookup failure: there is no corresponding function for the
*          input line.
*          2 on invalid syntax: the arguments list couldn't be parsed
*/
struct slcli_input_msg {
	uint8_t inArg;
	uint8_t done;
	uint8_t inQuote;
};

static int slcli_input_msg_handler(struct slcli_st* slcli,char *inbuf)
{
    struct slcli_input_msg stat;
    char *argv[SLCLI_ARG_NUM];
    uint32_t argc = 0;
    const struct slcli_command *command = NULL;
    const char *p;
	uint32_t i = 0;

    memset((void *)&argv, 0, sizeof(argv));
    memset(&stat, 0, sizeof(stat));

    do
    {
        switch (inbuf[i])
        {
        case '\0':
        	{
	            if (stat.inQuote)
	                return SLCLI_SYNTAR_ERROR;
	            stat.done = 1;
        	}
            break;
        case '"':
            if (i > 0 && inbuf[i - 1] == '\\' && stat.inArg)
            {
                memcpy(&inbuf[i - 1], &inbuf[i],strlen(&inbuf[i]) + 1);
                --i;
                break;
            }
            if (!stat.inQuote && stat.inArg)
                break;
            if (stat.inQuote && !stat.inArg)
                return SLCLI_SYNTAR_ERROR;

            if (!stat.inQuote && !stat.inArg)
            {
                stat.inArg = 1;
                stat.inQuote = 1;
                argc++;
                argv[argc - 1] = &inbuf[i + 1];
            }
            else if (stat.inQuote && stat.inArg)
            {
                stat.inArg = 0;
                stat.inQuote = 0;
                inbuf[i] = '\0';
            }
            break;

        case ' ':
            if (i > 0 && inbuf[i - 1] == '\\' && stat.inArg)
            {
                memcpy(&inbuf[i - 1], &inbuf[i],strlen(&inbuf[i]) + 1);
                --i;
                break;
            }
            if (!stat.inQuote && stat.inArg)
            {
                stat.inArg = 0;
                inbuf[i] = '\0';
            }
            break;

        default:
            if (!stat.inArg)
            {
                stat.inArg = 1;
                argc++;
                argv[argc - 1] = &inbuf[i];
            }
            break;
        }
    }
    while (!stat.done && ++i < SLCLI_INBUF_SIZE);

    if (stat.inQuote){
        return SLCLI_SYNTAR_ERROR;
    }

    if (argc < 1){
        return SLCLI_SUCCEED;
    }

    if (slcli->flag & SLCLI_ECHO_FLAG) {
        _slcli_put_string(slcli,"\r\n");
    }

    /*
    * Some comamands can allow extensions like foo.a, foo.b and hence
    * compare commands before first dot.
    */
    i = ((p = strchr(argv[0], '.')) == NULL) ? 0 : (p - argv[0]);
    command = lookup_slcli_command_tab(slcli,argv[0], i);
    if (command == NULL) {
        return SLCLI_BAD_COMMAND_ERROR;
    }

    memset(slcli->outbuf, 0, SLCLI_OUTBUF_SIZE);
    _slcli_put_string(slcli,"\r\n");

	slcli->output_len = 0;
   	int ret = command->function(slcli, argc, argv);
	if(slcli->output_ptr == NULL) {
		if(slcli->output_len) {
    		_slcli_put_array(slcli, slcli->outbuf, slcli->output_len);
		}
	}else {
		if(slcli->output_len) {
			_slcli_put_array(slcli, (char*)slcli->output_ptr, slcli->output_len);
		}
		slcli_free(slcli->output_ptr);
		slcli->output_ptr = NULL;
	}
	slcli->outbuf[0] = '\0';
	///slcli->outbuf[1] = '\0';
	///slcli->outbuf[2] = '\0';
	///slcli->outbuf[3] = '\0';
	slcli->output_len = 0;

	if(ret == SLCLI_CTXT_QUIT){
		return ret;
	}

    return SLCLI_SUCCEED;
}


int slcli_schedule(struct slcli_st* slcli,uint32_t timeout)
{
	int ret = SLCLI_SUCCEED;

	do
	{
		if ( slcli == NULL ) {
			ret = SLCLI_ENV_IS_NULL;
			break;
		}

		if(slcli->initialized != SLCLI_PATTERN_NUM){
			ret = SLCLI_IS_UN_PATTERN;
			break;
		}

		if(slcli->status != SLCLI_WORK_PATTERN){
			ret = SLCLI_IS_UN_PATTERN;
			break;
		}

		int ret;
		char *msg = NULL;

		ret = slcli_get_sem_timeout(&slcli->sema, timeout);
		if(ret != SLCLI_SUCCEED){
			///ret = SLCLI_SEM_TIMEOUT;
			ret = SLCLI_SUCCEED;
			break;
		}

		do
		{
			if(slcli_get_input(slcli,slcli->inbuf, &slcli->input_bp))
			{
				msg = slcli->inbuf;
				int result;
				result = slcli_input_msg_handler(slcli, msg);
				if (result == SLCLI_BAD_COMMAND_ERROR){
					slble_print_bad_command(slcli, msg);
				}else if (result == SLCLI_SYNTAR_ERROR){
					_slcli_put_string(slcli, "syntax error\r\n");
				}else if (result == SLCLI_CTXT_QUIT) {
					slcli->status = SLCLI_EXIT_PATTERN;
					///slcli_set_work_status(slcli,SLCLI_EXIT_PATTERN);
					return SLCLI_CTXT_QUIT;
				}
			#if defined(SLCLI_PROMPT_CFG) && SLCLI_PROMPT_CFG
				_slcli_put_string(slcli, SLCLI_PROMPT);
			#endif
				continue;
			}
		}while(0);
	}while(0);

	return ret;
}


struct slcli_st* slcli_init(char*name, int *status)
{
    int ret;
	struct slcli_st *ptr_slcli = NULL;

    ptr_slcli = (struct slcli_st *)slcli_malloc(sizeof(struct slcli_st));
    if (ptr_slcli == NULL) {
		if ( status ) {
			*status = SLCLI_NO_MEMORY_ERROR;
		}
        goto error;
    }

    memset((void *)ptr_slcli, 0, sizeof(struct slcli_st));

	ret = slcli_register_cmd_def_commands(ptr_slcli);
	if ( ret != SLCLI_SUCCEED ) {
		if ( status ) {
			*status = SLCLI_DEF_CMD_REG_ERROR;
		}
		goto error;
	}

	ret = slcli_sem_init(&ptr_slcli->sema);
	if ( ret != SLCLI_SUCCEED ) {
		if ( status ) {
			*status = SLCLI_ERROR;
		}
		goto error;
	}

	ret = slcli_lock_init(&ptr_slcli->lock);
	if ( ret != SLCLI_SUCCEED ) {
		if ( status ) {
			*status = SLCLI_ERROR;
		}
		goto error;
	}

	ptr_slcli->name = name;
    ptr_slcli->initialized = SLCLI_PATTERN_NUM;
	ptr_slcli->status = SLCLI_WORK_PATTERN;
    ptr_slcli->flag |= (SLCLI_ECHO_STATE_CFG ? SLCLI_ECHO_FLAG : 0);
	if ( status ) {
		*status = SLCLI_SUCCEED;
	}

    return ptr_slcli;

error:
	if ( ptr_slcli ) {
		if(ptr_slcli->sema){
			slcli_sem_deinit(&ptr_slcli->sema);
		}

		if(ptr_slcli->lock){
			slcli_lock_deinit(&ptr_slcli->lock);
		}

		if(ptr_slcli->inbuf_ptr){
			slcli_free(ptr_slcli->inbuf_ptr);
			ptr_slcli->inbuf_ptr = NULL;
		}

		if(ptr_slcli->output_ptr){
			slcli_free(ptr_slcli->output_ptr);
			ptr_slcli->output_ptr = NULL;
		}
		slcli_free(ptr_slcli);
		ptr_slcli = NULL;
	}
	return NULL;
}


int slcli_deinit(struct slcli_st* slcli)
{
	if((slcli == NULL) || (slcli->initialized != SLCLI_PATTERN_NUM)) {
		return SLCLI_ERROR;
	}

	slcli->status = SLCLI_DEINIT_PATTERN;
	slcli->initialized = 0;
	if(slcli->flag & SLCLI_NAME_STR_MALLOC){
		slcli_free(slcli->name);
		slcli->name = NULL;
		slcli->flag &= (~SLCLI_NAME_STR_MALLOC);
	}

	if(slcli->sema) {
		slcli_sem_deinit(&slcli->sema);
	}

	if(slcli->lock) {
		slcli_lock_deinit(&slcli->lock);
	}

	if(slcli->inbuf_ptr) {
		slcli_free(slcli->inbuf_ptr);
		slcli->inbuf_ptr = NULL;
	}

	if(slcli->output_ptr) {
		slcli_free(slcli->output_ptr);
		slcli->output_ptr = NULL;
	}

	slcli_free(slcli);
	slcli = NULL;

	return SLCLI_SUCCEED;
}

int slcli_register_port_api(struct slcli_st* slcli,Fslcli_getchar _getchar,Fslcli_put_array _put_array)
{
	if((slcli == NULL) || (slcli->initialized != SLCLI_PATTERN_NUM)) {
		return SLCLI_ERROR;
	}

	slcli->_getchar = _getchar;
	slcli->_put_array = _put_array;

	return SLCLI_SUCCEED;
}

uint16_t slcli_get_work_status(struct slcli_st* slcli)
{
	if((slcli == NULL) || (slcli->initialized != SLCLI_PATTERN_NUM)) {
		return SLCLI_ERROR;
	}

	return slcli->status;
}


int slcli_set_work_status(struct slcli_st* slcli, uint16_t status)
{
	if((slcli == NULL) || (slcli->initialized != SLCLI_PATTERN_NUM)) {
		return SLCLI_ERROR;
	}

	slcli->status = status;

	return SLCLI_SUCCEED;
}

int slcli_set_name(struct slcli_st* slcli, char* name)
{
	if((slcli == NULL) || (slcli->initialized != SLCLI_PATTERN_NUM)) {
		return SLCLI_ERROR;
	}

	if(slcli->flag & SLCLI_NAME_STR_MALLOC){
		slcli_free(slcli->name);
		slcli->name = NULL;
		slcli->flag &= (~SLCLI_NAME_STR_MALLOC);
	}

	slcli->name = name;

	return SLCLI_SUCCEED;
}

int slcli_set_alloc_name(struct slcli_st* slcli, char* name)
{
	if((slcli == NULL) || (slcli->initialized != SLCLI_PATTERN_NUM)) {
		return SLCLI_ERROR;
	}
	int str_len;
	char *old_name = slcli->name;
	int old_name_is_malloc = 0;

	if(name == NULL){
		str_len = 0;
	}else{
		str_len = strlen(name) + 1;
	}
	if(slcli->flag & SLCLI_NAME_STR_MALLOC){
		old_name_is_malloc = 1;
	}

	if(str_len){
		slcli->name = slcli_malloc(str_len);
		if(slcli->name == NULL){
			slcli->name = old_name;
			return SLCLI_NO_MEMORY_ERROR;
		}
		strncpy(slcli->name,name,str_len);
		slcli->flag |= SLCLI_NAME_STR_MALLOC;
	}

	if(old_name_is_malloc){
		slcli_free(old_name);
		old_name = NULL;
	}

	return SLCLI_SUCCEED;
}

char* slcli_get_name(struct slcli_st* slcli)
{
	if((slcli == NULL) || (slcli->initialized != SLCLI_PATTERN_NUM)) {
		return NULL;
	}

	return slcli->name;
}

int slcli_set_semaphore(struct slcli_st* slcli)
{
	if((slcli == NULL) || (slcli->initialized != SLCLI_PATTERN_NUM)) {
		return NULL;
	}

	return slcli_set_sem(&slcli->sema);
}


