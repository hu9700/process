#include <sys/ipc.h>
#include <sys/msg.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <stdio.h>
#include <errno.h>
/*
structure
1	ipc_perm	linux/ipc.h		save the permission of ipc object
2	msgbuf		linux/msg.h		the text of msg saved here, we can define by ourself
	struct msgbuf{
		long mtype;
		char mtext[1];
	};	maxium size is 4056 byte
3	msg			linux/msg.h		element of the link list of msg queue
4	msgqid_ds	linux/msg.h		object of one msg queue

function
1	int msgget(key_t key, int msgflg)
	to create new msg queue or get exist msg queue, return msg queue id
	
2	int msgsnd(int msqid, struct msgbuf *msgp, int msgsz, int msgflg)
	send a msg to msg queue
	
3	int msgrcv(int msqid, struct msgbuf* msgp, int msgsz, long mtype, int msgflg)
	receive a msg from msg queue
	
4	int msgctl(int msgqid, int cmd, struct msqid_ds *buf)
	control the msg queue
*/

int msg_queue_create_open(key_t key, int permit)
{
	return msgget(key, IPC_CREAT | permit);
}

//only create one, if exist, return error
int msg_queue_create(key_t key, int permit)
{
	return msgget(key, IPC_CREAT | IPC_EXCL | permit);
}

int msg_send(int msqid, void *msg_buff, int msg_size)
{
	return msgsnd(msqid, msg_buff, msg_size, 0);
}

int msg_send_nowait(int msqid, void *msg_buff, int msg_size)
{
	return msgsnd(msqid, msg_buff, msg_size, IPC_NOWAIT);
}

int msg_receive(int msqid, void *msg_buff, int msg_size, long msg_type)
{
	return msgrcv(msqid, msg_buff, msg_size, msg_type, MSG_NOERROR);
}

int msg_receive_nowait(int msqid, void *msg_buff, int msg_size, long msg_type)
{
	return msgrcv(msqid, msg_buff, msg_size, msg_type, MSG_NOERROR | IPC_NOWAIT);
}

int msg_receive_all(int msqid, void *msg_buff, int msg_size)
{
	return msgrcv(msqid, msg_buff, msg_size, 0, MSG_NOERROR);
}

int msg_receive_all_nowait(int msqid, void *msg_buff, int msg_size)
{
	return msgrcv(msqid, msg_buff, msg_size, 0, MSG_NOERROR | IPC_NOWAIT);
}

int msg_queue_getstate(int msgqid, struct msqid_ds *msq_attr)
{
	return msgctl(msgqid, IPC_STAT, msq_attr);
}

int msg_queue_setstate(int msgqid, struct msqid_ds *msq_attr)
{
	return msgctl(msgqid, IPC_SET, msq_attr);
}

int msg_queue_delete(int msgqid)
{
	return msgctl(msgqid, IPC_RMID, NULL);
}

void msg_queue_init_attr(struct msqid_ds *msq_attr, ushort mode, ushort uid, ushort gid)
{
	memset(msq_attr, 0, sizeof(struct msqid_ds));
	(msq_attr->msg_perm).mode = mode;
	(msq_attr->msg_perm).uid = uid;
	(msq_attr->msg_perm).gid = gid;
}

int main(int argc, char **argv)
{
	key_t msg_key = ftok(".", 'a');
	struct my_msg_buff
	{
		long msg_type;
		char text[64];
	};
	int child_pid;
	int count;
	int msg_queue_id;
	struct my_msg_buff msg_buff;
	struct msqid_ds msq_attr;
	
	msg_queue_id = msg_queue_create(msg_key, 0666);
	if(msg_queue_id < 0)
	{
		perror("msg create");
		exit(1);
	}
	
	child_pid = fork();
	if(child_pid > 0)
	{//parent
		for(count = 0; count < 5; count ++)
		{
			msg_receive(msg_queue_id, &msg_buff, sizeof(msg_buff) - sizeof(long), 0);
			printf(msg_buff.text);
		}
		
		msg_queue_delete(msg_queue_id);
	}
	else if(child_pid == 0)
	{//child
		for(count = 0; count < 10; count ++)
		{
			msg_buff.msg_type = count % 2;
			memset(msg_buff.text, 0, sizeof(msg_buff.text));
			snprintf(msg_buff.text, sizeof(msg_buff.text), "child msg send %d\n", count);
			msg_send(msg_queue_id, &msg_buff, sizeof(msg_buff) - sizeof(long));
		}	
	}
	else
	{
		perror("fork");
		exit(1);
	}
}
