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
	unsigned long PCTime, totalTime, lastQuanta, timeClock;
	unsigned int priority;
	pid_t localPid;
};

struct mesg_buffer{
	long mesg_type;
	int mesg_data[10];
} message;

int main(int argc, char * argv[]){
	srand(time(0));
	char * ptr;
	int msgid, timeFlag = 0;
	pid_t pid = getpid();
	time_t when, when2;
	struct PCB * shmPTR;
	unsigned long shmID;
	unsigned long key = strtoul(argv[0], &ptr, 10);
	unsigned long life = strtoul(argv[1], &ptr, 10);
	unsigned long msgKey = strtoul(argv[2], &ptr, 10);
	unsigned long logicalNum = strtoul(argv[3], &ptr, 10);
	unsigned long existence;
	msgid = msgget(msgKey, 0666 | IPC_CREAT);
	msgrcv(msgid, &message, sizeof(message), 1, 0);
	shmID = shmget(key, sizeof(struct PCB)*19, O_RDONLY);
	shmPTR = (struct PCB *) shmat(shmID, (void *)0, 0);
	existence = shmPTR[0].timeClock + rand() % 500000;
	while((shmPTR[0].timeClock < existence) && (timeFlag == 0)){
		int blocked = rand() % 100;
		if (blocked < 20){
			char messageText[50];
			sprintf(messageText, "%d", pid);
			strcat(messageText, " blocked");
			printf("%s\n", messageText);
			strcpy(message.mesg_text, messageText);
			msgsnd(msgid, &message, sizeof(message), 0);
			msgrcv(msgid, &message, sizeof(message), 1, 0);
			printf("%d running\n", pid);
		}
	}
	shmdt(shmPTR);
	printf("%d terminating\n", pid);
	return 0;
}
