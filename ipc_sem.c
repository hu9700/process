#include <sys/ipc.h>
#include <sys/msg.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sem.h>
#include <sys/types.h>

#include <unistd.h>
#include <stdio.h>
#include <errno.h>

/*
structure
1	sem			linux/sem.h			a singal semaphore
2	semun		linux/sem.h			it's the structure be used in senctl()
	union semun{
		int val;
		struct semid_ds *buf;
		ushort *array;
		struct seminfo *__buf
		void *__pad
	}
3	sembuf		linux/sem.h			be used by semop() to do basic operation to sem
	struct sembuf{
		unsigned shor sem_num;	//semaphore index in array
		short	sem_op;			//semaphore operation, positive num is release resource, negative num is ask for resource
		short	sem_flg;		//operation flags, can be IPC_NOWAIT and SEM_UNDO
	}
4	semid_ds						to save the information of one semaphore set

function
1	int semget(key_t key, int nsems, int semflg)
	To create semaphore set, or open one exist semaphore set, return semid

2	int semop(int semid, struct sembuf *sops, unsigned nsops)
	To change singal semaphore's value, or wait until semaphore value to 0
	
3	int semctl(int semid, int semnum, int cmd, union semun arg)
	To set the setting of semphore set or singal semaphore.
*/
union semun{
	int val;
	struct semid_ds *buf;
	ushort *array;
};

int sem_set_create_open(key_t key, int sem_num, ushort perm)
{
	return semget(key, sem_num, IPC_CREAT | perm);
}

int sem_set_create(key_t key, int sem_num, ushort perm)
{
	return semget(key, sem_num, IPC_CREAT | IPC_EXCL | perm);
}

int sem_get(int semset_id, unsigned short sem_index, short value)
{
	unsigned op_num = 1;
	struct sembuf sem_op_buf;
	sem_op_buf.sem_num = sem_index;
	sem_op_buf.sem_op = 0 - value;
	sem_op_buf.sem_flg = 0;
	
	return semop(semset_id, &sem_op_buf, op_num);
}

int sem_get_nowait(int semset_id, unsigned short sem_index, short value)
{
	unsigned op_num = 1;
	struct sembuf sem_op_buf;
	sem_op_buf.sem_num = sem_index;
	sem_op_buf.sem_op = 0 - value;
	sem_op_buf.sem_flg = IPC_NOWAIT;
	
	return semop(semset_id, &sem_op_buf, op_num);
}

int sem_release(int semset_id, unsigned short sem_index, short value)
{
	unsigned op_num = 1;
	struct sembuf sem_op_buf;
	sem_op_buf.sem_num = sem_index;
	sem_op_buf.sem_op = value;
	sem_op_buf.sem_flg = 0;
	
	return semop(semset_id, &sem_op_buf, op_num);
}

int sem_release_nowait(int semset_id, unsigned short sem_index, short value)
{
	unsigned op_num = 1;
	struct sembuf sem_op_buf;
	sem_op_buf.sem_num = sem_index;
	sem_op_buf.sem_op = value;
	sem_op_buf.sem_flg = IPC_NOWAIT;
	
	return semop(semset_id, &sem_op_buf, op_num);
}

int sem_wait_zero(int semset_id, unsigned short sem_index)//wait until the semaphore arrive to zero
{
	unsigned op_num = 1;
	struct sembuf sem_op_buf;
	sem_op_buf.sem_num = sem_index;
	sem_op_buf.sem_op = 0;
	sem_op_buf.sem_flg = 0;
	
	return semop(semset_id, &sem_op_buf, op_num);
}

//the sem_attr need to be alloc mem before been send to here
int sem_get_state(int semid, int sem_index, struct semid_ds *sem_attr)
{
	return semctl(semid, sem_index, IPC_STAT, sem_attr);
}

int sem_set_state(int semid, int sem_index, struct semid_ds *sem_attr)
{
	return semctl(semid, sem_index, IPC_SET, sem_attr);
}

int sem_set_delete(int semid)
{
	return semctl(semid, 0, IPC_RMID, 0);
}

//array should be alloc mem before use
int sem_set_get_allvalue(int semid, ushort *array)
{
	return semctl(semid, 0, GETALL, array);
}

//array should be alloc mem before use
int sem_set_set_allvalue(int semid, ushort *array)
{
	return semctl(semid, 0, SETALL, array);
}

int sem_get_waitingprocess_count(int semid, int sem_index)
{
	return semctl(semid, sem_index, GETNCNT, 0);
}

int sem_get_recent_waitingprocess_pid(int semid, int sem_index)
{
	return semctl(semid, sem_index, GETPID, 0);
}

int sem_get_value(int semid, int sem_index)
{
	return semctl(semid, sem_index, GETVAL, 0);
}

int sem_set_value(int semid, int sem_index, int value)
{
	return semctl(semid, sem_index, SETVAL, value);
}

int sem_get_usingprocess_count(int semid, int sem_index)
{
	return semctl(semid, sem_index, GETZCNT, 0);
}

#define PROCESS_NUM 20
int child_process(void)
{
	//do semaphore work
	key_t ipc_key;
	int semid;
	
	ipc_key = ftok(".", 'a');
	semid = sem_set_create_open(ipc_key, 1, 0666);
	sem_get(semid, 0, 1);
	
	sleep(2);
	
	sem_release(semid, 0, 1);
	
	exit(1);
	return 0;
}

int create_process(int process_count, int *pid_list)
{
	if(process_count < PROCESS_NUM)
	{
		int pid_child;
		process_count ++;
		pid_child = fork();
		
		if(pid_child > 0)
		{//parent
			key_t ipc_key;
			int semid;
			int wait_count;
			int sem_value;
			
			ipc_key = ftok(".", 'a');
			semid = sem_set_create_open(ipc_key, 1, 0666);
			sem_value = sem_get_value(semid, 0);
			wait_count = sem_get_waitingprocess_count(semid, 0);
			printf("sem_value=%d, wait_count=%d\n",sem_value, wait_count);
			
			pid_list[process_count - 1] = pid_child;
			return create_process(process_count, pid_list);
		}
		else if(pid_child == 0)
		{//child
			return child_process();
		}
		else
		{
			perror("fork fail");
			return (PROCESS_NUM - process_count + 1);//return how many process remain hasn't been created
		}
	}
	else
	{
		return 0;
	}
}

int main(int argc, char **argv)
{
	int pid_list[PROCESS_NUM];	
	key_t ipc_key;
	int semid;
	int res;
	ushort sem_top = PROCESS_NUM / 2;
	
	umask(0);
	ipc_key = ftok(".", 'a');
	memset(pid_list, 0, sizeof(pid_list));
	
	semid = sem_set_create_open(ipc_key, 1 , 0666);
	if(semid < 0)
	{
		perror("create");
		exit(1);
	}
	
	res = sem_set_value(semid, 0, sem_top);
	if(res < 0)
	{
		perror("set value");
		res = sem_set_delete(semid);
		if(res < 0)
		{
			perror("sem delete");
		}		
		exit(1);
	}
	
	res = create_process(0, pid_list);
	
	if(res > 0)
	{
		printf("ERROR, remain process %d\n", res);
		exit(1);
	}
	else
	{
		int count;
		
		for(count = 0; count < PROCESS_NUM; count ++)
		{
			waitpid(pid_list[count], NULL, 0);
		}
		
		res = sem_set_delete(semid);
		if(res < 0)
		{
			perror("sem delete");
		}
		exit(1);
	}
}
