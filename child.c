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
	unsigned long PCTime, totalTime, lastQuanta;
	unsigned int priority;
	pid_t localPid;
};

struct mesg_buffer{
	long mesg_type;
	char mesg_text[100];
} message;

int main(int argc, char * argv[]){
	srand(time(0));
	char * ptr;
	int msgid;
	pid_t pid = getpid();
	unsigned long * shmPTR;
	struct PCB * pdb;
	unsigned long shmID1;
	unsigned long key = strtoul(argv[0], &ptr, 10);
	unsigned long life = strtoul(argv[1], &ptr, 10);
	unsigned long shmID;
	unsigned long key1 = strtoul(argv[2], &ptr, 10);
	unsigned long msgKey = strtoul(argv[3], &ptr, 10);
	unsigned long existence;
	msgid = msgget(msgKey, 0666 | IPC_CREAT);
	msgrcv(msgid, &message, sizeof(message), 1, 0);
	shmID = shmget(key, sizeof(unsigned long), O_RDONLY);
	shmPTR = (unsigned long *) shmat(shmID, (void *)0, 0);
	shmID1 = shmget(key1, sizeof(struct PCB[18]), O_RDONLY);
	pdb = (struct PCB *) shmat(shmID1, (void *)0, 0);
	existence = shmPTR[0] + rand() % 500000;
	while(shmPTR[0] < existence){
		int blocked = rand() % 100;
		if (blocked < 25){
			char messageText[50];
			sprintf(messageText, "%d", pid);
			strcat(messageText, " blocked\n");
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
