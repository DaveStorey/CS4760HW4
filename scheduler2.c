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
	unsigned long PCTime, totalTime, lastQuanta, timeClock, created, childKey;
	unsigned int messageQueue, logical_Num;
	pid_t localPid;
};

struct mesg_buffer{
	long type;
	int user_pid;
	int quanta_used;
	int priority;
	int terminated;
}message;

void scheduler(char* outfile, int limit, int total){
	/*Variables for the keeping track of pids, a loop variable, waitPID status, how many processes are alive, a flag for if there have been any children spawned, a flag for if the overall total has been reached, a flag for if the time limit has been reached, and a flag for if the instant limit has been reached.*/
	int i = 0, k, totalSpawn = 0, alive = 1, noChildFlag = 1, totalFlag = 0, timeFlag = 0, limitFlag = 0, msgid, msgid1, msgid2, msgid3;
	//Variables for process nanoseconds, life, shared memory ID, seconds, and the timer increment, respectively.
	unsigned long quantum = 500000, shmID, shmID1, increment = 0, timeBetween = 0, launchTime = 0;
	//Pointer for the shared memory timer
	struct PCB * shmPTR;
	//Character pointers for arguments to pass through exec
	char * parameter[32], parameter1[32], parameter2[32], parameter3[32], parameter4[32], parameter5[32];
	pid_t pid[total], endID = 1; 
	//Time variables for the time out function
	time_t when, when2;
	//File pointers for input and output, respectively
	FILE * outPut;
	//Key variable for shared memory access.
	unsigned long key, key1, msgKey, msgKey1, msgKey2, msgKey3;
	srand(time(0));
	timeBetween = (rand() % 300000000);
	key = rand();
	key1 = rand();
	msgKey = rand();
	msgKey1 = rand();
	msgKey2 = rand();
	msgKey3 = rand();
	msgid = msgget(msgKey, 0666 | IPC_CREAT);
	msgid1 = msgget(msgKey1, 0666 | IPC_CREAT);
	msgid2 = msgget(msgKey2, 0666 | IPC_CREAT);
	msgid3 = msgget(msgKey3, 0666 | IPC_CREAT);
	message.type = 1;
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
	printf("Message queues: %d, %d, %d\n", msgid1, msgid2, msgid3);
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
					int level = rand() % 10;
				//Converting key, shmID and life to char* for passing to exec.
					sprintf(parameter1, "%li", key);
					sprintf(parameter2, "%li", quantum);
					sprintf(parameter3, "%d", i+1);
					printf("Level = %d\n", level);
					if (level <=1){
						shmPTR[i+1].messageQueue = msgid1;
						shmPTR[i+1].childKey = msgKey1;
					}
					else if (level <=6){
						shmPTR[i+1].messageQueue = msgid2;
						shmPTR[i+1].childKey = msgKey2;
					}
					else{
						shmPTR[i+1].messageQueue = msgid3;
						shmPTR[i+1].childKey = msgKey3;
					}
					shmPTR[i+1].logical_Num = i+1;
					shmPTR[i+1].lastQuanta = 50000;
					shmPTR[i+1].PCTime = 0;
					shmPTR[i+1].totalTime = 0;
					shmPTR[i+1].created = shmPTR[0].timeClock;
					char * args[] = {parameter1, parameter2, parameter3, NULL};
					printf("Launching child %d in message queue %d\n", getpid(), shmPTR[i+1].messageQueue);
					execvp("./child\0", args);
				}
				else{
				//If statement to reset alive counter after getting into while loop initially
					
							
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
		for (k = 1; k <= alive; k++){
			if (shmPTR[k].messageQueue == msgid1){
				msgsnd(msgid1, &message, sizeof(message), 0);
				msgrcv(msgid1, &message, sizeof(message), 1, 0);
			}
		}
		for (k = 1; k <= alive; k++){
			if (shmPTR[k].messageQueue == msgid2){
				msgsnd(msgid2, &message, sizeof(message), 0);
				msgrcv(msgid2, &message, sizeof(message), 1, 0);
			}
		}
		for (k = 1; k <= alive; k++){
			if (shmPTR[k].messageQueue == msgid3){
				msgsnd(msgid3, &message, sizeof(message), 0);
				msgrcv(msgid3, &message, sizeof(message), 1, 0);
			}
		}
		//If statements check against total children spawned and instant limit.
		if (message.terminated == 1){
			limitFlag = 0;
			alive--;
		}
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
		printf("Terminated due to ctrl-c signal at %li with %d children.\n", shmPTR[0].timeClock, alive);
		fprintf(outPut, "Scheduler terminated at %li nanoseconds due to ctrl-c signal with %d children.\n", shmPTR[0].timeClock, alive);
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
	msgctl(msgid1, IPC_RMID, NULL);
	msgctl(msgid2, IPC_RMID, NULL);
	msgctl(msgid3, IPC_RMID, NULL);
	fclose(outPut);
}
