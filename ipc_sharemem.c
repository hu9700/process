#include <sys/ipc.h>
#include <sys/shm.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>


#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>

/*
structure
1	shmid_ds		linux/shm.h			save information of sharemem

function
1	int shmget(key_t key, int size, int shmflg)
	shmflg can be IPC_CREAT IPC_EXCL,  return shmid
	
2	int	shmat(int shmid, char *shmaddr, int shmflg)
	map the share mem to process's own mem space
	shmaddr can be NULL, then we do not need to alloc mem to shmaddr, kernel will do it
	return the point we map to.
	shmflg can be SHM_RND make memory size as page size, SHM_RDONLY make memory as readonly
	
3	int shmctl(int shmqid, int cmd, struct shmid_ds *buf)
	IPC_STAT
	IPC_SET
	IPC_RMID system will wait until all the connect is done with memory, then delete it
	
4	int shmdt(char *shmaddr)
	disconnect the map to the share memory
*/

int shm_create_open(key_t key, int size, ushort perm)
{
	return shmget(key, size, IPC_CREAT | perm);
}

int shm_create(key_t key, int size, ushort perm)
{
	return shmget(key, size, IPC_CREAT | IPC_EXCL | perm);	
}

char *shm_map(int shmid, char *addr)
{
	return (void *)(shmat(shmid, addr, 0));
}

char *shm_map_readonly(int shmid, char *addr)
{
	return (void *)(shmat(shmid, addr, SHM_RDONLY));
}

int shm_unmap(char *addr)
{
	return shmdt(addr);
}

int shm_get_state(int shmid, struct shmid_ds *shm_attr)
{
	return shmctl(shmid, IPC_STAT, shm_attr);
}

int shm_set_state(int shmid, struct shmid_ds *shm_attr)
{
	return shmctl(shmid, IPC_SET, shm_attr);
}

int shm_delete(int shmid)
{
	return shmctl(shmid, IPC_RMID, 0);
}

#define MEM_SIZE 512
void child_sync(int sig)
{
	return;
}

void parent_sync(int sig)
{
	return;
}

int main()
{
	int pid_child;
	key_t shm_key;
	int shmid;
	
	umask(0);
	shm_key = ftok(".", 'a');
	
	signal(SIGUSR2, parent_sync);
	
	pid_child = fork();
	if(pid_child > 0)
	{//parent
		char * share_mem = NULL;
		
		pause();//wait for child send signal
		shmid = shm_create(shm_key, 512, 0666);
		if(shmid < 0)
		{
			perror("shm create");
			if(errno == EEXIST)
			{
				shmid = shm_create_open(shm_key, 512, 0666);
				shm_delete(shmid);
			}
			exit(1);
		}
		
		share_mem = shm_map(shmid, NULL);
		if((int)share_mem == -1)
		{
			perror("shm map");
			shm_delete(shmid);
			exit(1);
		}
		
		memset(share_mem, 0, MEM_SIZE);
		snprintf(share_mem, MEM_SIZE, "Send data from Parent\n");
		
		shm_unmap(share_mem);
		
		kill(pid_child, SIGUSR1);
		
		wait(NULL);
		shm_delete(shmid);
		exit(1);
	}
	else if(pid_child == 0)
	{//child
		char * share_mem = NULL;
		
		sleep(1);//wait for parent pause()
		signal(SIGUSR1, child_sync);
		kill(getppid(), SIGUSR2);
		pause();//wait for data ready
		
		shmid = shm_create_open(shm_key, 512, 0666);
		share_mem = shm_map_readonly(shmid, NULL);
		printf(share_mem);
		shm_unmap(share_mem);
		exit(1);
	}
	else
	{
		perror("fork");
		exit(1);
	}
}
