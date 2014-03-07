#include <sys/stat.h>//for umask
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <string.h>
#include <linux/limits.h>//for PATH_MAX
#include <linux/ipc.h>
//ipcs -q -m -s check ipc status
//ipcrm <msg|sem|shm> <IPC_ID> remove ipc object

//user need to free the point return
char * get_pwd(void)
{
	char *pwd_buff = NULL;
	int count = 0;
	pwd_buff = malloc(PATH_MAX);
	memset(pwd_buff, 0, (count + 1) * PATH_MAX);
	while(getcwd(pwd_buff, (count + 1) * PATH_MAX) == NULL)
	{
		count ++;
		pwd_buff = realloc(pwd_buff, (count + 1) * PATH_MAX);
		memset(pwd_buff, 0, (count + 1) * PATH_MAX);
	}
	return pwd_buff;
}

//user need to free the point return
//it need sudo
char * get_image_path(void)
{
	char *process_dir = NULL;
	char proc_dir_buff[PATH_MAX];
	int count = 0;
	
	memset(proc_dir_buff, 0, sizeof(proc_dir_buff));
	snprintf(proc_dir_buff, sizeof(proc_dir_buff), "/proc/%d/exe", getpid());
	
	process_dir = malloc(PATH_MAX);
	memset(process_dir, 0, (count + 1) * PATH_MAX);
	while(readlink(proc_dir_buff, process_dir, (count + 1) * PATH_MAX) >= (count + 1) * PATH_MAX)
	{
		count ++;
		process_dir = realloc(process_dir, (count + 1) * PATH_MAX);
		memset(process_dir, 0, (count + 1) * PATH_MAX);
	}
	return process_dir;
}



int main(int argc, char **argv)
{
	key_t ipc_key;
	
//	char *process_dir = NULL;
//	process_dir = get_image_path();
//	ipc_key = ftok(process_dir, 'a');//generate key
//	free(process_dir);
	
	ipc_key = ftok(".", 'a');
	
	return 0;
}
