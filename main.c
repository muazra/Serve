/**
 * Redistribution of this file is permitted under the GNU General
 * Public License v2.
 *
 * Copyright 2012 by Gabriel Parmer.
 * Author: Gabriel Parmer, gparmer@gwu.edu, 2012
 */
/* 
 * This is a HTTP server.  It accepts connections on port 8080, and
 * serves a local static document.
 *
 * The clients you can use are 
 * - httperf (e.g., httperf --port=8080),
 * - wget (e.g. wget localhost:8080 /), 
 * - or even your browser.  
 *
 * To measure the efficiency and concurrency of your server, use
 * httperf and explore its options using the manual pages (man
 * httperf) to see the maximum number of connections per second you
 * can maintain over, for example, a 10 second period.
 */

/**
 * Name - Muaz Rahman
 * Email - muazra@gwmail.gwu.edu
 * ECE 3411 - Operating Systems
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <sys/wait.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <util.h> 		/* client_process */
#include <server.h>		/* server_accept and server_create */

#include <cas.h>
#include <ring.h>

#define MAX_DATA_SZ 1024
#define MAX_CONCURRENCY 16

/* 
 * This is the function for handling a _single_ request.  Understand
 * what each of the steps in this function do, so that you can handle
 * _multiple_ requests.  Use this function as an _example_ of the
 * basic functionality.  As you increase the server in functionality,
 * you will want to probably keep all of the functions called in this
 * function, but define different code to use them.
 */
void
server_single_request(int accept_fd)
{
	int fd;

	/* 
	 * The server thread will always want to be doing the accept.
	 * That main thread will want to hand off the new fd to the
	 * new threads/processes/thread pool.
	 */
	fd = server_accept(accept_fd);
	client_process(fd);

	return;
}

void
server_multiple_requests(int accept_fd)
{
  int fd;
  
  /*
   * Infinite loop to serve multiple requests
   * on one thread.
   */
  while(1){
    fd = server_accept(accept_fd);
    client_process(fd);
  }

  return;
}

void
server_processes(int accept_fd)
{
  int fd;
  int status;
  int pid;
  int temp=0;

  while(1){
    fd = server_accept(accept_fd);
    pid = fork();
    temp++;
    if(pid < 0){                             /*if process fails to create*/
      exit(0);
    }
    else if(pid == 0){                       /*child creation success*/
      client_process(fd);
      exit(0);
    }
    else{                                 /* parent */
        if(temp == MAX_CONCURRENCY){      /* if max has been reached */
	waitpid(&pid, &status, WNOHANG);  /* waiting for child process */
	temp--;                           /* decrementing on child finish */    
	close(fd);
      }
    }

  }

  exit(0);
  return;
}

void
server_dynamic(int accept_fd)
{
	return;
}

/*
 *Creating a new thread
 *for every request
 */

//function prototypes
void* worker(void*);

void
server_thread_per(int accept_fd)
{
  int fd;
  pthread_t thread;
  int temp=0;

  while(1){
    fd = server_accept(accept_fd);
    if(pthread_create(&thread, NULL, &worker, (void*)&fd)){
      exit(0);
    }
    temp++;
   
    if(temp == MAX_CONCURRENCY){
      pthread_join(thread, NULL);
      temp--;
      close(fd);
    }
  }
  
  return;
}

void* worker(void *arg){
  client_process(*(int*)arg);
  pthread_exit(NULL);
}

/*
 *Creating max threads
 *and serving requests
 *via atomic instructions
 */

//structure definition
struct request{
  volatile int fd;
  struct request *next;
};

//global structure 
volatile struct request *requests;

//function prototypes
struct request *get_request(void);
void put_request(struct request *r);
void* worker1(void);

void
server_task_queue(int accept_fd)
{
  int file;
  int temp=0;
  pthread_t thread;
  
  //initialize global requests pointer
  requests = (struct request*) malloc(sizeof(struct request));
  requests->fd = NULL;
  requests->next = NULL;
  
  //create max threads
  while(temp < MAX_CONCURRENCY){
    if(pthread_create(&thread, NULL, &worker1, NULL)){
	   exit(0);
    }
    temp++;
  }

  while(1){
    file = server_accept(accept_fd);
    struct request *r= (struct request*) malloc(sizeof(struct request));
    r->fd = file;
    put_request(r);
  }
  
  pthread_exit(NULL);
  return;
}

void* worker1(void){
  while(1){
    get_request();
  }
  pthread_exit(NULL);
}

void put_request(struct request *r){ 

  unsigned long *old;
  unsigned long *new;
  struct request *temp;
  
  /*
   *Atomically swap requests 
   *with new request node
   *and move next pointer to 
   *next in list
   */
  do{
    temp = requests;
    old = (unsigned long*)temp;
    r->next = temp;
    new = (unsigned long*)r;
  }while(__cas((unsigned long*)&requests, (unsigned long)old, (unsigned long)new));

}

struct request *get_request(void){

  while(requests == NULL);
  while(requests->fd  == NULL);
  
  client_process(requests->fd);

  unsigned long *old;
  unsigned long *new;
  struct request *temp;
  
  /*
   *Atomically move requests
   *node to next in list and
   *free up original head
   *and close the file descriptor
   *client connection
   */
  do{
    temp = requests;
    old = (unsigned long*)temp;
    close(temp->fd);
    struct request *temp2 = temp;
    temp = temp->next;
    free(temp2);
    new = (unsigned long*)temp;
  }while(__cas((unsigned long*)&requests, (unsigned long)old, (unsigned long)new));
  
  return temp;
}

/*
 *Serving requests via
 *thread pool using mutex
 */

struct list{
  int fd;
  struct list *next;
};

struct list *head;
pthread_mutex_t lock;
void* worker3(void*);

void
server_thread_pool(int accept_fd)
{
  int fd, i;
  pthread_t thread[MAX_CONCURRENCY];
  
  pthread_mutex_init(&lock, NULL);

  head = (struct list*) malloc(sizeof(struct list));
  head->fd = NULL;
  head->next = NULL;

  for(i = 0; i < MAX_CONCURRENCY; i++){
    if(pthread_create(&thread[i], NULL, worker3, NULL))
      exit(0);
  }
  
  while(1){
    fd = server_accept(accept_fd);
    pthread_mutex_lock(&lock);
    struct list *node = (struct list*) malloc(sizeof(struct list));
    node->fd = fd;
    node->next = head;
    head = node;
    pthread_mutex_unlock(&lock);
  }
  
  pthread_exit(NULL);
  return;
}

void* worker3(void* arg){
  while(1){
    pthread_mutex_lock(&lock);
    while((head == NULL)||( head->fd == NULL)) {
      pthread_mutex_unlock(&lock);
      continue;
    }
    client_process(head->fd);
    struct list *temp = head;
    head = head->next;
    free(temp);
    pthread_mutex_unlock(&lock);
  }

  pthread_exit(NULL);
} 

/*
 *Create thread pool
 *using condiiton variables.
 */

void* worker4(void*);
struct ring *rbuffer;
pthread_mutex_t count_lock;
pthread_cond_t thread_cond[2];

void 
server_thread_pool_blocking(int accept_fd)
{
  int fd, i;
  pthread_t thread[MAX_CONCURRENCY];
  pthread_mutex_init(&count_lock, NULL);
  rbuffer = rb_create();

  for(i = 0; i < MAX_CONCURRENCY; i++){
    pthread_create(&thread[i], NULL, worker4, (void*)&i);
  }
  
  pthread_cond_init(&thread_cond[0], NULL);
  pthread_cond_init(&thread_cond[1], NULL);

  while(1){
     fd = server_accept(accept_fd);
     pthread_mutex_lock(&count_lock);

     while(rb_isfull(rbuffer) == -1){
       pthread_cond_wait(&thread_cond[1], &count_lock);
     }

     rb_enqueue(rbuffer, (void*)&fd);
     pthread_cond_signal(&thread_cond[0]);

     pthread_mutex_unlock(&count_lock);
  }
    
  pthread_exit(NULL);
  return;
}

void* worker4(void* arg){
  while(1){
    pthread_mutex_lock(&count_lock);

    while(rb_isempty(rbuffer) == -1){
      pthread_cond_wait(&thread_cond[0], &count_lock);
    }

    void *value = rb_dequeue(rbuffer);
    client_process(*(int*)value);
    pthread_cond_signal(&thread_cond[1]);

    pthread_mutex_unlock(&count_lock);
  }
  pthread_exit(NULL);
} 


/*
 *Serving requests via
 *char device 
*/

//function declarations and global vars
void* worker2(void*);
int chardev_fd;

void 
server_char_device_queue(int accept_fd)
{
  int fd, i;
  pthread_t thread[MAX_CONCURRENCY];

  chardev_fd = open("/dev/muaz", O_RDWR);

  for(i = 0; i < MAX_CONCURRENCY; i++){
    if(pthread_create(&thread[i], NULL, worker2, NULL))
      exit(0);
  }

  while(1){
    fd = server_accept(accept_fd);
    write(chardev_fd, (void*)&fd, sizeof(int));
  }

  pthread_exit(NULL);
  return;
}

void* worker2(void* arg){
  while(1){
    int fd;
    while(read(chardev_fd, (void*)&fd, sizeof(int)) == 0);
    client_process(fd);
    close(fd);
  }
  pthread_exit(NULL);
} 

typedef enum {
	SERVER_TYPE_ONE = 0,
	SERVER_TYPE_SINGLET = 1,
	SERVER_TYPE_PROCESS = 2,
	SERVER_TYPE_FORK_EXEC,
	SERVER_TYPE_SPAWN_THREAD = 4,
	SERVER_TYPE_TASK_QUEUE = 5,
	SERVER_TYPE_THREAD_POOL = 6,
	SERVER_TYPE_THREAD_POOL_BLOCKING = 7,
	SERVER_TYPE_CHAR_DEVICE_QUEUE = 8,
} server_type_t;

int
main(int argc, char *argv[])
{
	server_type_t server_type;
	short int port;
	int accept_fd;

	if (argc != 3) {
		printf("Proper usage of http server is:\n%s <port> <#>\n"
		       "port is the port to serve on, # is either\n"
		       "0: serve only a single request\n"
		       "1: use only a single thread for multiple requests\n"
		       "2: use fork to create a process for each request\n"
		       "3: Extra Credit: use fork and exec when the path is an executable to run the program dynamically.  This is how web servers handle dynamic (program generated) content.\n"
		       "4: create a thread for each request\n"
		       "5: use atomic instructions to implement a task queue\n"
		       "6: use a thread pool with mutex only\n"
		       "7: use a thread pool with condition variables\n"
		       "8: use a queue implemented in a char device\n"
		       "9: to be defined\n",
		       argv[0]);
		return -1;
	}

	port = atoi(argv[1]);
	accept_fd = server_create(port);
	if (accept_fd < 0) return -1;
	
	server_type = atoi(argv[2]);

	switch(server_type) {
	case SERVER_TYPE_ONE:
		server_single_request(accept_fd);
		break;
	case SERVER_TYPE_SINGLET:
		server_multiple_requests(accept_fd);
		break;
	case SERVER_TYPE_PROCESS:
		server_processes(accept_fd);
		break;
	case SERVER_TYPE_FORK_EXEC:
		server_dynamic(accept_fd);
		break;
	case SERVER_TYPE_SPAWN_THREAD:
		server_thread_per(accept_fd);
		break;
	case SERVER_TYPE_TASK_QUEUE:
		server_task_queue(accept_fd);
		break;
	case SERVER_TYPE_THREAD_POOL:
		server_thread_pool(accept_fd);
		break;
	case SERVER_TYPE_THREAD_POOL_BLOCKING:
	        server_thread_pool_blocking(accept_fd);
		break;
	case SERVER_TYPE_CHAR_DEVICE_QUEUE:
	        server_char_device_queue(accept_fd);
                break;
	}
	
        close(accept_fd);
	return 0;
}
