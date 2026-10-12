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

int historyCount = 0;
char history[5][maxInput];

enum redirection {none, in, out, inOut, outForce, inOutForce};

//struct stores a single command to be executed
typedef struct command_t {
	char* name;		//exe path name
	char* args[maxArgs];	//launch arguments
	int argc;		//argument count
	char* inFile;		//input file (only meaningful if command using input redirection)
	char* outFile;		//output file (only meaningful if command using output redirection)
	enum redirection redir;	//whether this command redirects input/output
} command_t;


//launch command from struct
void executeCommand(command_t command){
	if (strcmp(command.name, "history") == 0){
		for(int i = 0; i < historyCount; i++){
			printf("[%i] %s",(historyCount-i), history[i]);
		}
		exit(1);
	}
        execvp(command.name, command.args);
	perror("execution failed");
        exit(1);	
}

//given an array of command structs that are piped together and the count,
//execute each command, handling piping and in/output redirection correctly 
void executeCommands(command_t* commands, size_t commandCount, char* input){
	int status; 
	pid_t pid;  
	int currentPipe[2];
	int nextIn = -1;
	bool first, last;
	int fd;

	//for each command in the pipeline
	for (int i = 0; i < commandCount; i++){
		if (i == 0) first = true; //is this command the first?
		else first = false;
		if (i == (commandCount - 1)) last = true; // is this command the last?
		else last = false;

		//if this is not the last command, we want to create a pipe to connect it to the next 
		if (!last) pipe(currentPipe); 
		
		//fork and check for error
		if ((pid = fork()) < 0){ 
			perror("fork failed"); exit(1);
		}
		// child process - execute command
		else if (pid == 0){ 
			if (!first){ // if not the first command - need to pipe input from last one
				dup2(nextIn, 0); // dup input end of last command's pipe to this command's stdin
				close(nextIn);
			}
			else{ //is first command - check for input redirection
				if (commands[i].redir == in || commands[i].redir == inOut || commands[i].redir == inOutForce){
				//if first command has input redirection
					fd = open(commands[i].inFile, O_RDONLY); //attempt to open its input file
					if (fd  != -1){
						dup2(fd, 0); // dup input file to stdin
						close(fd);		
					}	
					else{
						perror("input file could not be opened");
					}
				}
			}
			if (!last){ //is not the last command - need to pipe output to next one
				dup2(currentPipe[1], 1); //dup output end of current pipe to stdout
				close(currentPipe[1]); //close pipes created by parent
				close(currentPipe[0]);
			}
			else{ //is last command - check for output redirection
				if (commands[i].redir == out || commands[i].redir == inOut){ //command had output redirection
					fd = open(commands[i].outFile, O_WRONLY | O_CREAT | O_APPEND, 0644);
					if (fd  != -1){
						dup2(fd, 1); // dup output file to stdout
						close(fd);
					}
					else{
						perror("output file could not be opened");
					}
				}
				else if (commands[i].redir == outForce || commands[i].redir == inOutForce){ //command had forced output redirection
					fd = open(commands[i].outFile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
					if (fd  != -1){
						dup2(fd, 1); // dup output file to stdout
						close(fd);
					}
					else{
						perror("output file could not be opened");
					}
				}
			}
			//input/output setup finished, launch program
			executeCommand(commands[i]);
		}
		//parent process
		else{ 
			if (!first){
				close(nextIn); //close input pipe left over from previous iteration
			}
			if (!last){
				nextIn = currentPipe[0]; //save this iteration's input pipe for use next iteration
				close(currentPipe[1]); //close output pipe from this iteration
			}
			
		}
	}
	//once all commands are lauched, parent waits for each process to finsh
	bool success = true;
	for (int i = 0; i < commandCount; i++){ 
		wait(&status); 
		if (status != 0){ success = false;}
	}
	if (success){
	
		if (historyCount < 5) {
			strcpy(history[historyCount], input);
			historyCount++;
		}
		else{
			for (int i = 0; i < 4; i++){
				strcpy(history[i], history[i + 1]);
			}
			strcpy(history[4], input);
		}
		
	}
}

void processInput(char *tokens[], int tokenCount, char* input) {
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

	executeCommands(commands, commandCount, input);
}

int main(int argc, char* argv[]) {

	char input[maxInput] = "  "; //user shell input
	char inputBak[maxInput];
	char* tokens[maxTokens]; //tokenized input
	int tokenCount; // amount of tokens in "tokens"
	char* tok; //curent token

	puts("Welcome to VM shell");
	puts("enter command followed by arguments");
	puts("enter | between commands to pipe");
	printf("$ ");
	fgets(input, maxInput, stdin);
	strcpy(inputBak, input);
	while(!(tolower(input[0]) == 'q' && tolower(input[1]) == 'u' && tolower(input[2]) == 'i' && tolower(input[3]) == 't')){
		tokenCount = 0;
		tok = strtok(input, " \n");
		while (tok != NULL && tokenCount < maxTokens){
			tokens[tokenCount] = tok;
			tokenCount++;
			tok = strtok(NULL, " \n");
		}
		processInput(tokens, tokenCount, inputBak);
		printf("$ ");
		fgets(input, maxInput, stdin);
		if (input[0] == '!' && input[1] >= '1' && input[1] <= '5' && (input[2] == '\n' || input[2] == '\0')) {
			int number = input[1] - '0';
			if (number <= historyCount) {
				strcpy(input, history[historyCount - number]);
			} 
			else {
				fprintf(stderr, "Invalid history number\n");
				continue;
			}
		}
		strcpy(inputBak, input);
	}

	return 0;

}


