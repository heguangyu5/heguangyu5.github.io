#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <stdio.h>
#include <stdlib.h>

union semun {
	int val;
};

int main(int argc, char *argv[])
{
	if (argc != 2 && argc != 3) {
		printf("%s init-value\n", argv[0]);
		printf("%s semid operation\n", argv[0]);
		return 1;
	}
	
	int semid;

	if (argc == 2) {
		semid = semget(IPC_PRIVATE, 1, 0600);
		union semun arg;
		arg.val = atoi(argv[1]);
		semctl(semid, 0, SETVAL, arg);
		printf("semid = %d, semval[0] = %d\n", semid, semctl(semid, 0, GETVAL));
		return 0;
	}

	semid = atoi(argv[1]);
	int operation = atoi(argv[2]);
	printf("semid = %d will %d\n", semid, operation);

	struct sembuf sop;
	sop.sem_num = 0;
	sop.sem_op = operation;
	sop.sem_flg = 0;
	semop(semid, &sop, 1);
	printf("done, semval[0] = %d\n", semctl(semid, 0, GETVAL));

	return 0;
}
