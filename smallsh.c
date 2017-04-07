/**********************************************
 *Name: Taylor Del Matto
 *Assignment 3
 *Operating Systems
 *In this assignment a shell script was created that forks and executes
 *into various processes, the shell has a series of if statements
 *to handle built in statements, and a different series of if statements
 * to handle i/o for non built in statements.  All processes are tracked
 * and cleaned up by the shell  
 * ******************************************/

#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <dirent.h>
//declarations
pid_t cpid;
int j;
int fd;
int exitsignal;
int exitstatus = -1;
char *tempstr;
int currentpidstatus;
int bpidstatus;
int i;
char *otherargs[512];
char argin[2049];
int bpids[100];
int bpidssize = 0;
char *args[512];
int argssize=0;
int isbackground = 0;
char *arg1;
int pri=0;

//this function checks if a file exists
int doesfileexist(char *filename){
	FILE* pointer = fopen(filename, "r");
	if(pointer == NULL){
		return 0;
	}
	return 1;
}

char temparg[20];
//this function is used to kill foreground ps
//when ctrl c is pressed
void endforeground(){
	kill(cpid, SIGKILL);
}

//various print functions
//prints active background pids 
void printbpids(){
	for(pri=0; pri < 20; pri++){
		printf("bpid %d is 0%d0  \n", pri, bpids[pri]);
	}
	printf("bpidssize is %d\n", bpidssize);
}
void printargs(){
	for(pri=0; pri < argssize+1; pri++){
		printf("arg %d is 0%s0  \n", pri, args[pri]);
	}
	printf("argssize is %d\n", argssize);
}
void printotherargs(){
	for(pri=0; pri < argssize+1; pri++){
		printf("arg %d is 0%s0\n", pri, otherargs[pri]);
	}
	printf("argssize is %d\n", argssize);
}
//program loop begins here
int main(int argc, char** argv){

sigset_t emptyset;
sigemptyset(&emptyset);

struct sigaction beforefore;
beforefore.sa_handler = endforeground;
beforefore.sa_flags = SA_RESTART;

struct sigaction mosttime;
mosttime.sa_handler = SIG_IGN;
sigaction(SIGINT, &mosttime, NULL);
DIR* dir;

for(i=0; i < 100; i++){
	bpids[i] = -1;
}
for(i=0; i<512; i++){
	args[i]=malloc(60*sizeof(char));
}


chdir(getenv("HOME"));
while(1){
	//loop through background processes and check for completed processes
	//if anything is completed, clean it up?
	for(i = 0; i < bpidssize; i++){
		if(waitpid(bpids[i], &bpidstatus, WNOHANG)){
			printf("background pid %d is done: ", bpids[i]);
			if(WIFEXITED(bpidstatus)){
				exitstatus = WEXITSTATUS(bpidstatus);
				//printf("exited successfully\n");
				printf("exit value is %d\n", exitstatus);
			}
			else if(WIFSIGNALED(bpidstatus)){
				exitsignal = WTERMSIG(bpidstatus);
				printf("process was terminated by signal %d\n", exitsignal);	
			} 
			
			//now remove from array
			for(j=i; j<bpidssize-1; j++){
				bpids[j] = bpids[j+1];
			}
			bpids[bpidssize-1] = -1;
			bpidssize = bpidssize - 1;
		}
	}
	for(i=0; i<512; i++){
		strcpy(args[i], "\0");
	}	
	
	//program prompt
	fflush(stdin);
	fflush(stdout);
	printf(":");
	fgets(argin, 2049, stdin);
	fflush(stdin);
	arg1 = strtok(argin, " ");
	argssize = 0;

	//put arguments into args array using strtok
	while(arg1 != NULL && argssize < 512){
		strcpy(args[argssize], arg1);
		argssize++;
		arg1 = strtok(NULL, " ");
	}
	//remove newline
	arg1= strtok(args[argssize-1], "\n");
	arg1= strtok(args[argssize-1], " ");	
	strcpy(args[argssize-1], arg1);
	strcpy(args[argssize], "");
	
	//built in commands start here
	//if condition for cd
	strcpy(temparg, "cd");
	if(strcmp(args[0], temparg) == 0){
		//if arguments greater than 1
		if(argssize > 1){
			dir = opendir(args[1]);
			//and if directory exists
			if(dir){
				tempstr = malloc(strlen(args[1])*sizeof(char));
				//printf("directory is valid");
				//printf("this is args1, %s", args[1]);
				strcpy(tempstr, args[1]);
				chdir(tempstr);
				free(tempstr);
				tempstr = NULL;
			}
			else{
				//if not, home
				printf("%s: no such file or directory\n", args[1]); 
				chdir(getenv("HOME"));
			
			}
		}
		else{	//if not, home
			chdir(getenv("HOME"));
				
		}
		
	}
	//elseif condition for status
	else if(strcmp(args[0], "status") == 0){
		if(WIFEXITED(currentpidstatus)){
			exitstatus = WEXITSTATUS(currentpidstatus);
			printf("exit value %d\n", exitstatus);
		}
		else if(WIFSIGNALED(currentpidstatus)){
			exitsignal = WTERMSIG(currentpidstatus);
			printf("terminated by signal %d\n", exitsignal);	
		}	
	}

	//if condition for exit
	else if(strcmp(args[0], "exit") == 0){
		//for all background processes, send kill signal to that background processv
		for(i=0; i < bpidssize; i++){
			kill(bpids[i], SIGKILL);
		}
		exit(0);
		//return 0;
		//change directory to ...
		
	}
	else if(strncmp(args[0], "#", 1) == 0){
		//comment....
	}
	else{//must be a shell script or an invalid command	
		for(i=0; i<argssize; i++){
			otherargs[i]=malloc(60*sizeof(char));
			strcpy(otherargs[i], args[i]);
		}
		//before fork, set background flag (otherwise must do this twice)
		if(strcmp(args[argssize-1], "&") == 0){
			isbackground = 1;
		}
		else{
			isbackground = 0;
		}
		cpid = fork();
		if(cpid == -1){
			printf("child wasnt created, error\n");
			exit(1);
		}
		else if(cpid == 0){//in the child, execute here	
			if(isbackground == 1){//if background process, then redirect i/o, get rid of & character
				//printf("background process");
				//set background flag
				//once background flag is set, we can get rid of the & character
				free(otherargs[argssize-1]);
				otherargs[argssize-1] = NULL;
				argssize= argssize - 1;
				
				//redirect i/o
				fd = open("/dev/null", O_WRONLY | O_TRUNC | O_CREAT);
				dup2(fd, 1);
				dup2(fd, 2);
				
			}
			if(strcmp(args[1], "<")== 0 ){//open file for read, pass file pointer to i/o using dup2()	
				//printf("read from file");
				fd = open(otherargs[argssize-1], O_RDONLY);
				if(fd == -1){
					printf("cannot open %s for input\n", args[2]);
					exit(1);
				}
				dup2(fd, 0);	
				
				//get rid of last two arguments
				free(otherargs[argssize-1]);
				otherargs[argssize-1] = NULL;
				argssize= argssize - 1;
				free(otherargs[argssize-1]);
				otherargs[argssize-1] = NULL;
				argssize= argssize - 1;
				
			}
			else if(strcmp(args[1], ">")== 0){//open file for write, pass file pointer to i/o using dup2()
				fd = open(otherargs[argssize-1], O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
				if(fd == -1){
					printf("cannot open %s for output\n", args[2]);
					exit(1);
				}
				dup2(fd, 1);	
				
				//get rid of last two arguments
				free(otherargs[argssize-1]);
				otherargs[argssize-1] = NULL;
				argssize= argssize - 1;
				
				free(otherargs[argssize-1]);
				otherargs[argssize-1] = NULL;
				argssize= argssize - 1;
			}
			//foreground process
			if(isbackground == 0){
				//printf("foreground process");
				//printf("other args0 0%s0", otherargs[0]);
				sigaction(SIGINT, &beforefore, NULL);
				execvp(otherargs[0], otherargs);
				printf("%s: no such file or directory", otherargs[0]);	
				exit(1);	
			}
			else{//background process execution (without waitpid)
				execvp(otherargs[0], otherargs);
				printf("%s: no such file or directory", otherargs[0]);
				exit(1);
			}			
		}
		else{//in the parent, add to backgroundprocesses array here
			//printf("in the parent");
			for(i=0; i<argssize; i++){
				free(otherargs[i]);
				otherargs[i] = NULL;
			}
			if(isbackground == 0){//if foreground, waitpid
				waitpid(cpid, &currentpidstatus, 0);
			}
			else{//background, add to background pid array
				bpids[bpidssize] = cpid;
				bpidssize++;
				//printbpids();
				printf("background pid is %d\n", cpid);	
			}
		}	
	}
}//enc program loop
}//end main


