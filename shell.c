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
#define maxCommands 20


typedef struct command_t {
	char* name;
	char* args[maxArgs];
	int argc;
} command_t;



void executeCommand(command_t command){
        printf("running: %s, arguments: %i \n", command.name, command.argc);
        execvp(command.name, command.args); 
        return;
}

void executeCommands(command_t* commands, int commandCount, int current){

	int status;
	pid_t pid;
	if (current == commandCount){
		wait(&status);
	}
	else{
		if ((pid = fork()) < 0){
			perror("fork failed");
			exit(1);
		}
		else if (pid == 0){ //the child
			executeCommand(commands[current]);
		}
		else{ //the parent
			current++;
			executeCommands(commands, commandCount, current);
		}
	}
}

void processInput(char* tokens[], int tokenCount){
	
	int i = 0; int offset = 0; int commandCount = 0;

	command_t com;
	command_t commands[maxCommands];
	com.argc = 0;
	while(i < tokenCount && commandCount < maxCommands){
		com.args[i - offset] = tokens[i];
		com.argc++;

		if ((*(tokens[i]) == '|' && *(tokens[i]+1) == '\0') || i == tokenCount - 1){
			com.args[i - offset + 1] = NULL;
			if (*(tokens[i]) == '|' && *(tokens[i]+1) == '\0'){
				com.args[i - offset] = NULL; com.argc--;
			}
			com.name = com.args[0];
			
			commands[commandCount] = com;	
			commandCount++;
			com.argc = 0;
			offset = i + 1;
		}
		
		i++;
	}
	executeCommands(commands, commandCount, 0);

	return;	

}


int main(int argc, char* argv[]) {

	char input[maxInput] = "    "; //user shell input
	char* tokens[maxTokens]; //tokenized input
	int tokenCount; // amount of tokens in "tokens"
	char* tok; //curent token

	printf("Welcome to VM shell\nenter command followed by arguments\nenter | between commands to pipe\n");
	fgets(input, maxInput, stdin);
	while(!(tolower(input[0]) == 'q' && tolower(input[1]) == 'u' && tolower(input[2]) == 'i' && tolower(input[3]) == 't')){
		tokenCount = 0;

		tok = strtok(input, " \n");
		while (tok != NULL && tokenCount < maxTokens){
			tokens[tokenCount] = tok;
			tokenCount++;
			tok = strtok(NULL, " \n");
		}

		processInput(tokens, tokenCount);
		fgets(input, maxInput, stdin);
	}

	return 0;

}


