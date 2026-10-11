#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <stdbool.h>
#include <fcntl.h>
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
	int fd;

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
			else{
				if (commands[i].redir == in || commands[i].redir == inOut || commands[i].redir == inOutForce){
					fd = open(commands[i].inFile, O_RDONLY);
					if (fd  != -1){
						dup2(fd, 0);
						close(fd);		
					}	
					else{
						perror("input file could not be opened");
					}
				}
			}
			if (!last){ 
				dup2(currentPipe[1], 1);
				close(currentPipe[1]);
				close(currentPipe[0]);
			}
			else{
				if (commands[i].redir == out || commands[i].redir == inOut){
					fd = open(commands[i].outFile, O_WRONLY | O_CREAT | O_APPEND, 0644);
					if (fd  != -1){
						dup2(fd, 1);
						close(fd);
					}
					else{
						perror("output file could not be opened");
					}
				}
				else if (commands[i].redir == outForce || commands[i].redir == inOutForce){
					fd = open(commands[i].outFile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
					if (fd  != -1){
						dup2(fd, 1);
						close(fd);
					}
					else{
						perror("output file could not be opened");
					}
				}
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

void processInput(char *tokens[], int tokenCount) {
	int commandCount = 0;
	command_t com = {0};
	command_t commands[maxCommands] = {0};

	for (int i = 0; i < tokenCount; i++) {
		if (strcmp(tokens[i], "|") == 0) {
			if (com.argc == 0) {
				fprintf(stderr, "Missing command before pipe\n");
				return;
			}

			com.args[com.argc] = NULL;
			com.name = com.args[0];

			if (commandCount >= maxCommands) {
				fprintf(stderr, "Too many commands\n");
				return;
			}

			commands[commandCount++] = com;
			com = (command_t){0};
		}
		else if (strcmp(tokens[i], "<") == 0) {
			if (i + 1 >= tokenCount) {
				fprintf(stderr, "Missing input filename\n");
				return;
			}
			com.inFile = tokens[++i];
			com.redir = (com.redir == out) ? inOut : in;
		}
		else if (strcmp(tokens[i], ">") == 0) {
			if (i + 1 >= tokenCount) {
				fprintf(stderr, "Missing output filename\n");
				return;
			}
			com.outFile = tokens[++i];
			com.redir = (com.redir == in) ? inOut : out;
		}
		else if (strcmp(tokens[i], ">!") == 0) {
			if (i + 1 >= tokenCount) {
			fprintf(stderr, "Missing output filename\n");
			return;
			}
			com.outFile = tokens[++i];
			com.redir = (com.redir == in) ? inOutForce : outForce;
		}
		else {
			if (com.argc >= maxArgs - 1) {
				fprintf(stderr, "Too many arguments\n");
				return;
			}

			com.args[com.argc++] = tokens[i];
		}
	}

	if (com.argc == 0) {
		fprintf(stderr, "Missing command after pipe or empty input\n");
		return;
	}

	com.args[com.argc] = NULL;
	com.name = com.args[0];

	if (commandCount >= maxCommands) {
		fprintf(stderr, "Too many commands\n");
		return;
	}

	commands[commandCount++] = com;

	executeCommands(commands, commandCount);
}

int main(int argc, char* argv[]) {

	char input[maxInput] = "  "; //user shell input
	char* tokens[maxTokens]; //tokenized input
	int tokenCount; // amount of tokens in "tokens"
	char* tok; //curent token

	puts("Welcome to VM shell");
	puts("enter command followed by arguments");
	puts("enter | between commands to pipe");
	printf("$ ");
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
		printf("$ ");
		fgets(input, maxInput, stdin);
	}

	return 0;

}


