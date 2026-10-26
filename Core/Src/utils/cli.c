#include <stdint.h>
#include <string.h>
#include <stm32f429xx.h>
#include "usart.h"

#define MAX_ARGUMENTS 8
#define CMD_COUNT (sizeof(command_table) / sizeof(command_table[0]))

typedef void (*cli_command_function)(int argc, char **argv);

typedef struct{
	const char *command_name;
	const char *helper_text;
	uint8_t argument;
	cli_command_function handler;
} cmd_commands;

void echo(int argc, char **argv);
void pin_manage(int argc, char **argv); //TODO

const cmd_commands command_table[] = {
		{"echo", "Echoes entered argument.", 1, echo},
		{"pin", "Controls specified pin", 2, pin_manage}
};

void echo(int argc, char **argv){
	if (argc < 2) {
	  usart_write("Empty echo used.\r\n");
	  return;
	}
	usart_write(argv[1]);
	usart_write("\r\n");
}

void pin_manage(int argc, char **argv){

}

void cli_init(char *command){
	char *argv[MAX_ARGUMENTS]; // Max arguments
	int argc = 0;

	//Tokenize
	char *token = strtok(command, " \r\n");
	while(token != NULL && argc < MAX_ARGUMENTS){
		argv[argc++] = token;
		token = strtok(NULL, " \r\n");
	}

	if(argc == 0) return;

	//Find the corresponding command
	for(size_t i = 0; i < CMD_COUNT; i++){
		if(strcmp(argv[0], command_table[i].command_name) == 0){
			//argc is command itself so argc - 1
			if((argc - 1) < command_table[i].argument){
				usart_write("Too few arguments\r\n");
				return;
			}
			//Call the command
			command_table[i].handler(argc, argv);
			return;
		}
	}
	usart_write("Invalid command\r\n");
}
