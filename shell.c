#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/wait.h>
#include <sys/types.h>
#define maxInput 300
#define maxTokens 100

int main(int argc, char* argv[]) {

	char input[maxInput] = "    "; //user shell input
	char* tokens[maxTokens]; //tokenized input
	char* command[20] //tokens for a single command - program name + args
	char* tok; //curent token

	pid_t pid;
	int status, tokenCount, pipeCount, round;
	int outputPipe[2] = NULL;
	int inputPipe[2] = NULL;

	fgets(input, maxInput, stdin);
	while(!(tolower(input[0]) == 'q' && tolower(input[1]) == 'u' && tolower(input[2]) == 'i' && tolower(input[3]) == 't')){
		tokenCount = 0; pipeCount = 0; round = 0;

		tok = strtok(input, " \n");
		while (tok != NULL && tokenCount < maxTokens - 1){
			tokens[tokenCount] = tok;
			tokenCount++;
			if (tok[0] == '|' && tok[1] == '\0'){
				pipeCount++;
			}
			tok = strtok(NULL, " \n");
		}


		int commands = pipeCount + 1;
		int currentToken = 0;
		while (round < pipeCount+1){


			if (outputPipe != null){ //program inherits input pipe from output of last program
				inputPipe = OutputPipe;
			}
			if (round != pipeCount){ //only create new pipe if this is not the last command
				outputPipe = pipe();
			}

		
			if ((pid = fork()) < 0){
				perror("fork failed");
				exit(1);
			}
			else if (pid == 0){ //the child
				tokens[tokenCount] = NULL; //set last argument to NULL as required when passing to execute a program
				
				if (outputPipe != NULL){
					close(outputPipe[0]);
					dup2(outputPipe[1], stdout)
				}
				if (inputPipe != NULL){
					close(outputPipe[1]);
					dup2(outputPipe[0], stdin);
				}



				execvp(tokens[0], tokens); 
				printf("launching %s failed", tokens[0]);
				perror("execution of program failed"); 
				exit(1);
			}		
			else{ //the parent
			      close(outputPipe);
			        round++;

				wait(&status);// wait for child program to exit before continuing to get input 
			}
		}

		fgets(input, maxInput, stdin);
	}
	return 0;
}
