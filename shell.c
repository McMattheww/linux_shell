#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <stdbool.h>
#define maxInput 300
#define maxTokens 100
#define maxArgs 20
#define maxCommands 20

enum redirection {none, in, out, inOut, outForce, inOutForce};

typedef struct command_t {
	char* name;
	char* args[maxArgs];
	int argc;
	char* inFile;
	char* outFile;
	enum redirection redir;
} command_t;

void executeCommand(command_t command){
        execvp(command.name, command.args);
	perror("execution failed");
        exit(1);	
}

void executeCommands(command_t* commands, size_t commandCount){
	int status;
	pid_t pid;
	int currentPipe[2];
	int nextIn = -1;
	bool first, last;

	for (int i = 0; i < commandCount; i++){
		if (i == 0) first = true;
		else first = false;
		if (i == (commandCount - 1)) last = true;
		else last = false;

		if (!last) pipe(currentPipe);
	
		if ((pid = fork()) < 0){ 
			perror("fork failed"); exit(1);
		}
		else if (pid == 0){
			if (!first){ 
				dup2(nextIn, 0);
				close(nextIn);
			}
			if (!last){ 
				dup2(currentPipe[1], 1);
				close(currentPipe[1]);
				close(currentPipe[0]);
			}
			executeCommand(commands[i]);
		}
		else{ //the parent
			perror("application launched");
			if (!first){
				close(nextIn);
			}
			if (!last){
				nextIn = currentPipe[0];
				close(currentPipe[1]);
			}
			
		}
	}
	for (int i = 0; i < commandCount; i++) wait(&status);
}

void processInput(char* tokens[], int tokenCount){
	
	int i = 0; int offset = 0; int commandCount = 0;
	command_t com;
	command_t commands[maxCommands];
	com.argc = 0;
	com.redir = none;
	

	while(i < tokenCount && commandCount < maxCommands){
		if (*(tokens[i]) == '|'){
			com.args[i - offset] = NULL;
			com.name = com.args[0];
			commands[commandCount] = com;	
			commandCount++;
			com.argc = 0;
			com.redir = none;
			offset = i + 1;
			
		}
		else if(*(tokens[i]) == '<'){
			if ((i+1)<tokenCount){
				com.redir = in;
				com.inFile = tokens[i + 1]; i++;
			}
			else break;
		}
		else if(*(tokens[i]) == '>'){
			if ((i+1)<tokenCount){
				if (com.redir != in)com.redir = out;
				else com.redir = inOut;
				com.outFile = tokens[i + 1]; i++;
			}
			else break;
		}
		else{
			com.args[i - offset] = tokens[i];
			com.argc++;
		}
		i++;
	}
	com.args[tokenCount] = NULL;
	com.name = com.args[0];
	commands[commandCount] = com;
	commandCount++;

	for (int i = 0; i < commandCount; i++){
		printf("%s  enum:%i", commands[i].name, commands[i].redir);
	}
	executeCommands(commands, commandCount);
	return;	
}

int main(int argc, char* argv[]) {

	char input[maxInput] = "  "; //user shell input
	char* tokens[maxTokens]; //tokenized input
	int tokenCount; // amount of tokens in "tokens"
	char* tok; //curent token

	puts("Welcome to VM shell");
	puts("enter command followed by arguments");
	puts("enter | between commands to pipe");
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


