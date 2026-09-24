#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/wait.h>
#include <sys/types.h>
#define maxInput 300
#define maxTokens 100
#define maxArgs 20


typedef struct command_t {
	char* name;
	char* args[maxArgs];
	int argc;
} command_t;



void executeCommand(command_t command){
	printf("running: %s, arguments: %i \n", command.name, command.argc);
	return;
}

void processInput(char* tokens[], int tokenCount){
	
	int i = 0; int offset = 0;
	command_t com;
	com.argc = 0;
	while(i < tokenCount){
		com.args[i - offset] = tokens[i];
		com.argc++;

		if ((*(tokens[i]) == '|' && *(tokens[i]+1) == '\0') || i == tokenCount - 1){
			com.args[i - offset + 1] = NULL;
			if (*(tokens[i]) == '|' && *(tokens[i]+1) == '\0'){com.args[i - offset] = NULL; com.argc--;}
			com.name = com.args[0];
			executeCommand(com);
			com.argc = 0;
			offset = i + 1;
		}
		
		i++;
	}

	return;	

}


int main(int argc, char* argv[]) {

	char input[maxInput] = "    "; //user shell input
	char* tokens[maxTokens]; //tokenized input
	char* tok; //curent token


	int tokenCount, pipeCount;

	fgets(input, maxInput, stdin);
	while(!(tolower(input[0]) == 'q' && tolower(input[1]) == 'u' && tolower(input[2]) == 'i' && tolower(input[3]) == 't')){
		tokenCount = 0; pipeCount = 0;

		tok = strtok(input, " \n");
		while (tok != NULL && tokenCount < maxTokens){
			tokens[tokenCount] = tok;
			tokenCount++;
			if (tok[0] == '|' && tok[1] == '\0'){
				pipeCount++;
			}
			tok = strtok(NULL, " \n");
		}

		processInput(tokens, tokenCount);
		fgets(input, maxInput, stdin);
	}

	return 0;

}


