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
	char* command[20]; //tokens for a single command - program name + args
	char* tok; //curent token

	pid_t pid;
	int status, tokenCount, pipeCount, round;
	int outputPipe[2] = {-1, -1};
	int inputPipe[2] = {-1, -1};
	int pvc[2] = {-1, -1};

	fgets(input, maxInput, stdin);
	while(!(tolower(input[0]) == 'q' && tolower(input[1]) == 'u' && tolower(input[2]) == 'i' && tolower(input[3]) == 't')){
		tokenCount = 0; pipeCount = 0; round = 0;

		tok = strtok(input, " \n");
		while (tok != NULL && tokenCount < maxTokens){
			tokens[tokenCount] = tok;
			tokenCount++;
			if (tok[0] == '|' && tok[1] == '\0'){
				pipeCount++;
			}
			tok = strtok(NULL, " \n");
		}


		int startIndex = 0;
		for (int i = 0; i < tokenCount; i++){
			if (( *(tokens[i]) == '|' && *(tokens[i] + 1) == '\0') || (i == tokenCount - 1)){
				printf("FORK");
				fflush(stdout);


				if (round != pipeCount){
					pipe(pvc);
				}
				
				if (startIndex != 0){ //program inherits input pipe from output of last program
                                	inputPipe[0] = outputPipe[0];
					inputPipe[1] = outputPipe[1];
                        	}
                        	if (round != pipeCount){ //only create new pipe if this is not the last command
                                	pipe(outputPipe);
                        	}

				if ((pid = fork()) < 0){
                                	perror("fork failed");
                                	exit(1);
                        	}
                        	else if (pid == 0){ //the child
					
                                	if (outputPipe[0] >= 0){
                                        	close(outputPipe[0]);
                                        	dup2(outputPipe[1], 1);
                                	}
                                	if (inputPipe[0] >= 0){
                                        	close(inputPipe[1]);
                                        	dup2(inputPipe[0], 0);
                                	}
					

					char* test[] = {"test", NULL};
                                	execvp(tokens[startIndex], test);
                                	printf("launching %s failed", tokens[0]);
                                	perror("execution of program failed");
                                	exit(1);
                        	}
                        	else{ //the parent
                        		close(inputPipe[0]); close(inputPipe[1]);
					inputPipe[0] = -1;
                                	wait(&status);// wait for child program to exit before continuing to get input
                        	}
				round++;
				startIndex = i + 1;		
			}
		}


		fgets(input, maxInput, stdin);
	}
	return 0;
}
