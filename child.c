#include<stdio.h>
#include<fcntl.h>
#include<sys/ipc.h>
#include<string.h>
#include<sys/shm.h>
#include<sys/types.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/msg.h>
#include<time.h>

struct PCB{
	unsigned long PCTime, totalTime, lastQuanta, timeClock, created, childKey;
	unsigned long messageQueue, logical_Num;
	pid_t localPid;
};

struct mesg_buffer{
	long type;
	int user_pid;
	int quanta_used;
	int priority;
	int logical;
	int terminated;
} message;

int main(int argc, char * argv[]){
	srand(time(0));
	char * ptr;
	int msgid, timeFlag = 0;
	pid_t pid = getpid();
	int i = 0, blocked = 0;
	struct PCB * shmPTR;
	unsigned long shmID;
	unsigned long key = strtoul(argv[1], &ptr, 10);
	unsigned long life = strtoul(argv[2], &ptr, 10);
	unsigned long logicalNum = strtoul(argv[3], &ptr, 10);
	unsigned long existence = (shmPTR[0].timeClock + (rand() % 250000));
	//msgid = msgget(sh, 0666 | IPC_CREAT);
	shmID = shmget(key, sizeof(struct PCB)*19, 0777);
	printf("Child shmat error, %li\n", shmID);
	shmPTR = (struct PCB *) shmat(shmID, (void *)0, 0);
	printf("In child, message queue %li\n", shmPTR[logicalNum].messageQueue);
	msgrcv(shmPTR[logicalNum].messageQueue, &message, sizeof(message), 1, 0);
	message.type = 1;
	while((shmPTR[0].timeClock < existence) && (timeFlag == 0)){
		if (i > 5){
			timeFlag = 1;
		}
		blocked = rand() % 100;
		if (blocked < 20){
			message.quanta_used = (rand() % 99 * shmPTR[logicalNum].lastQuanta);
			message.user_pid = getpid();
			message.logical = logicalNum;
			message.terminated = 0;
			msgsnd(shmPTR[logicalNum].messageQueue, &message, sizeof(message), 0);
			shmPTR[logicalNum].PCTime += message.quanta_used;
			msgrcv(shmPTR[logicalNum].messageQueue, &message, sizeof(message), 1, 0);
			shmPTR[logicalNum].totalTime = shmPTR[0].timeClock - shmPTR[logicalNum+1].created;
			printf("%d running after block.\n", pid);
		}
		if (blocked >= 20)
			printf("%d not blocked.\n", pid);
		i++;
	}
	message.terminated = 1;
	message.type = 1;
	message.user_pid = pid;
	msgsnd(shmPTR[logicalNum].messageQueue, &message, sizeof(message), 0);
	shmdt(shmPTR);
	printf("%d terminating", pid);
	if (timeFlag)
		printf(" due to timeflag.");
	printf("\n");
	return 0;
}
