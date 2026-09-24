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


				if ((pid = fork()) < 0){
                                	perror("fork failed");
                                	exit(1);
                        	}
                        	else if (pid == 0){ //the child
					
					if (round != 0){
						dup2(pvc[0], 0);
					}
                                	if (round != pipeCount){
						dup2(pvc[1], 1);
					}
					close(pvc[0]); close(pvc[1]);
					

					char* test[] = {"test", NULL};
                                	execvp(tokens[startIndex], test);
                                	printf("launching %s failed", tokens[0]);
                                	perror("execution of program failed");
                                	exit(1);
                        	}
                        	else{ //the parent
                        		close(pvc[0]); close(pvc[1]);
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
