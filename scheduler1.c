#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <time.h>
#include <signal.h>
#include <sys/msg.h>

//Signal handler to catch ctrl-c
static volatile int keepRunning = 1;

void intHandler(int dummy) {
    keepRunning = 0;
}

struct PCB{
	unsigned long PCTime, totalTime, lastQuanta, timeClock;
	unsigned int priority, logicalNum;
	pid_t localPid;
};

struct mesg_buffer{
	long mesg_type;
	int mesg_data[10];
}message;

void scheduler(char* outfile, int limit, int total){
	/*Variables for the keeping track of pids, a loop variable, waitPID status, how many processes are alive, a flag for if there have been any children spawned, a flag for if the overall total has been reached, a flag for if the time limit has been reached, and a flag for if the instant limit has been reached.*/
	int i = 0, k, status, totalSpawn = 0, alive = 1, noChildFlag = 1, totalFlag = 0, timeFlag = 0, limitFlag = 0, msgid;
	//Variables for process nanoseconds, life, shared memory ID, seconds, and the timer increment, respectively.
	unsigned long quantum = 500000, shmID, shmID1, increment = 0, timeBetween = 0, launchTime = 0;
	//Pointer for the shared memory timer
	struct PCB * shmPTR;
	//Character pointers for arguments to pass through exec
	char * parameter[32], parameter1[32], parameter2[32], parameter3[32], parameter4[32];
	pid_t pid[total], endID = 1; 
	//Time variables for the time out function
	time_t when, when2;
	//File pointers for input and output, respectively
	FILE * outPut;
	//Key variable for shared memory access.
	unsigned long key, key1, msgKey;
	srand(time(0));
	timeBetween = (rand() % 300000000);
	key = rand();
	key1 = rand();
	msgKey = rand();
	msgid = msgget(msgKey, 0666 | IPC_CREAT);
	message.mesg_type = 1;
	//Setting initial time for later check.
	time(&when);
	outPut = fopen(outfile, "a");
	//Check for file error.
	if (outPut == NULL){
		perror("Error");
		printf("Output file could not be created.\n");
		exit(EXIT_SUCCESS);
	}
	
	//Get and access shared memory, setting initial timer state to 0.
	shmID = shmget(key, sizeof(struct PCB) * 19, IPC_CREAT | IPC_EXCL | 0777);
	shmPTR = (struct PCB *) shmat(shmID, (void *) 0, 0);
	shmPTR[0].timeClock = 0;
	/*shmID1 = shmget(key1, sizeof(struct PCB)*18, IPC_CREAT | IPC_EXCL | 0777);
	pcbPTR = (struct PCB *) shmat(shmID, (void *) 0, 0);
	pcbPTR[0].PCTime = 1234;*/
	//Initializing pids to -1 
	for(k = 0; k < total; k++){
		pid[k] = -1;
	}
	//Call to signal handler for ctrl-c
	signal(SIGINT, intHandler);
	increment = (rand() + 10000) % 5000000;
	//While loop keeps running until all children are dead, ctrl-c, or time is reached.
	while((alive > 0) && (keepRunning == 1) && (timeFlag == 0)){
		time(&when2);
		if ((when2 - when) >= 10){
			timeFlag = 1;
		}
		//Incrementing the timer.
		shmPTR[0].timeClock += increment;
		/*If statement will only run check for new children to spawn if limit has not been hit.  If limit has been hit, it will allow the clock to continue to increment to allow currently alive children to naturally die.*/
		if((totalFlag == 0) && (limitFlag == 0)){
		//If statement to spawn child if timer has passed its birth time.
			if(shmPTR[0].timeClock >= (launchTime + timeBetween)){
				if((pid[i] = fork()) == 0){
				//Converting key, shmID and life to char* for passing to exec.
					sprintf(parameter1, "%li", key);
					sprintf(parameter2, "%li", quantum);
					sprintf(parameter3, "%li", msgKey);
					sprintf(parameter4, "%d", i+1);
					int level = rand() % 10;
					if (level <=1)
						shmPTR[i+1].priority = 0;
					else{
						shmPTR[i+1].priority = 1;
					}
					shmPTR[i+1].logicalNum = i+1;
					char * args[] = {parameter1, parameter2, parameter3, parameter4, NULL};
					printf("Launching child %d\n", getpid());
					execvp("./child\0", args);
				}
				else{
				//If statement to reset alive counter after getting into while loop initially
					mesg_data[0]
					msgsnd(msgid, &message, sizeof(message), 0);
					if (noChildFlag > 0){
						alive--;
						noChildFlag = 0;
					}
				//Setting seconds to high number for above loop protecting against twin children.
					launchTime = shmPTR[0].timeClock;
					alive++;
					totalSpawn++;
					i++;
				}
			}
		}
		//EndID to check for status of group children
		endID = waitpid(0, &status, WNOHANG | WUNTRACED);
		if (endID == -1){
		//	perror("waitpid error");
		}
		else if (endID == 0);
		else{
		//For loop steps through array of pids, checking if return value matches.
			for(k = 0; k <= i; k++){
				if(endID == pid[k]){
					alive--;
					limitFlag = 0;
				}
			}
		}
		//If statements check against total children spawned and instant limit.
		if (totalSpawn >= total){
			totalFlag = 1;
		}
		if (alive >= limit){
			limitFlag = 1;
		}
	}
	if(timeFlag == 1){
		printf("Program has reached its allotted time, exiting.\n");
		fprintf(outPut, "Scheduler terminated at %li nanoseconds due to time limit.\n",  shmPTR[0].timeClock);
	}
	if(totalFlag == 1){
		printf("Program has reached its allotted children, exiting.\n");
		fprintf(outPut, "Scheduler terminated at %li nanoseconds due to process limit.\n",  shmPTR[0].timeClock);
	}
	if(keepRunning == 0){
		printf("Terminated due to ctrl-c signal.\n");
		fprintf(outPut, "Scheduler terminated at %li nanoseconds due to ctrl-c signal.\n",  shmPTR[0].timeClock);
	}
	if(alive > 0){
	int j = 0;
	printf("Killing children\n");
		while(pid[j] != -1){
			kill(pid[j], SIGQUIT);
			j++;
		}
	}
	shmdt(shmPTR);
	shmctl(shmID, IPC_RMID, NULL);
	msgctl(msgid, IPC_RMID, NULL);
	fclose(outPut);
}
