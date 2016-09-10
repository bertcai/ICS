#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void cpu_exec(uint32_t);

/* We use the ``readline'' library to provide more flexibility to read from stdin. */
char* rl_gets() {
	static char *line_read = NULL;

	if (line_read) {
		free(line_read);
		line_read = NULL;
	}

	line_read = readline("(nemu) ");

	if (line_read && *line_read) {
		add_history(line_read);
	}

	return line_read;
}

static int cmd_c(char *args) {
	cpu_exec(-1);
	return 0;
}

static int cmd_q(char *args) {
	return -1;
}

static int cmd_si(char *args){
	if (args == NULL){
		cpu_exec(1);
	}
	else {
		long a;
		a = strtol(args,NULL,10);
		cpu_exec(a);
	}
	return 0;
}
static int cmd_info(char *args){
	    if (strcmp(args,"r") == 0){
			printf(" %x \t %d \n", cpu.eax, cpu.eax);
			printf(" %x \t %d \n", cpu.edx, cpu.edx);
			printf(" %x \t %d \n", cpu.ecx, cpu.ecx);
			printf(" %x \t %d \n", cpu.ebx, cpu.ebx);
			printf(" %x \t %d \n", cpu.ebp, cpu.ebp);
			printf(" %x \t %d \n", cpu.esi, cpu.esi);
			printf(" %x \t %d \n", cpu.edi, cpu.edi);
			printf(" %x \t %d \n", cpu.esp, cpu.esp);																			    
		}
		return 0;
}
 
static int cmd_x(char *args){
		int temp;
		char *a = strtok(args, " ");
		char *b = strtok(NULL, " ");
		temp = strtol(a,NULL,10);
		long start_ad;
		start_ad = strtol(b, NULL, 16);
		int i = 1;
		for(i =1; i <= temp; i++){
			printf("%d \n", swaddr_read(start_ad, 1));
			start_ad++;
		}
		return 0;
}
static int cmd_help(char *args);

static struct {
	char *name;
	char *description;
	int (*handler) (char *);
} cmd_table [] = {
	{ "help", "Display informations about all supported commands", cmd_help },
	{ "c", "Continue the execution of the program", cmd_c },
	{ "q", "Exit NEMU", cmd_q },
	{ "si", "single step", cmd_si},
	{ "info", "print registe", cmd_info},
	{ "x", "scanning ram", cmd_x}
	/* TODO: Add more commands */

};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args) {
	/* extract the first argument */
	char *arg = strtok(NULL, " ");
	int i;

	if(arg == NULL) {
		/* no argument given */
		for(i = 0; i < NR_CMD; i ++) {
			printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
		}
	}
	else {
		for(i = 0; i < NR_CMD; i ++) {
			if(strcmp(arg, cmd_table[i].name) == 0) {
				printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
				return 0;
			}
		}
		printf("Unknown command '%s'\n", arg);
	}
	return 0;
}

void ui_mainloop() {
	while(1) {
		char *str = rl_gets();
		char *str_end = str + strlen(str);

		/* extract the first token as the command */
		char *cmd = strtok(str, " ");
		if(cmd == NULL) { continue; }

		/* treat the remaining string as the arguments,
		 * which may need further parsing
		 */
		char *args = cmd + strlen(cmd) + 1;
		if(args >= str_end) {
			args = NULL;
		}

#ifdef HAS_DEVICE
		extern void sdl_clear_event_queue(void);
		sdl_clear_event_queue();
#endif

		int i;
		for(i = 0; i < NR_CMD; i ++) {
			if(strcmp(cmd, cmd_table[i].name) == 0) {
				if(cmd_table[i].handler(args) < 0) { return; }
				break;
			}
		}

		if(i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
	}
}
