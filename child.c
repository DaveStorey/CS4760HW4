#include<stdio.h>
#include<fcntl.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<sys/types.h>
#include<unistd.h>
#include<stdlib.h>

struct PCB{
	unsigned long PCTime, totalTime, lastQuanta;
	unsigned int priority;
	pid_t localPid;
};

int main(int argc, char * argv[]){
	char * ptr;
	pid_t pid = getpid();
	unsigned long * shmPTR;
	struct PCB * pdb;
	printf("Child %d PDB address: %p \n",getpid(), pdb);
	unsigned long shmID1;
	unsigned long key = strtoul(argv[0], &ptr, 10);
	unsigned long life = strtoul(argv[1], &ptr, 10);
	unsigned long shmID;
	unsigned long key1 = strtoul(argv[2], &ptr, 10);
	unsigned long existence;
	shmID = shmget(key, sizeof(unsigned long), O_RDONLY);
	shmPTR = (unsigned long *) shmat(shmID, NULL, 0);
	shmID1 = shmget(key1, sizeof(struct PCB[18]), O_RDONLY);
	pdb = (struct PCB *) shmat(shmID1, NULL, 0);
	printf("Child %d PDB address: %p \n",getpid(), pdb);
	existence = shmPTR[0] + life;
	printf("Shared Memory: %li \n", pdb[0].PCTime);
	while(shmPTR[0] < existence);
	printf("Process %d exiting at %li\n", pid, shmPTR[0]);
	shmdt(shmPTR);
	return 0;
}
