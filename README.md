Setup
=====

1) Run "make" from terminal to initialize.<br>
2) Run "./server port# option#" to use.

Details
=====

Options available for "option#" in Setup:

0: serve only a single request <br>
1: use only a single thread for multiple requests <br>
2: use fork to create process for each request <br>
4: create new thread for each request <br>
5: use atomic instructions to implement task queue <br>
6: use a thread pool to serve requests <br>
7: use a thread pool w/ blocking conditions via ring buffer<br>
8: serve requests using char device - done in kernel mode <br>

	SERVER_TYPE_ONE = 0,
	SERVER_TYPE_SINGLET = 1,
	SERVER_TYPE_PROCESS = 2,
	SERVER_TYPE_FORK_EXEC,
	SERVER_TYPE_SPAWN_THREAD = 4,
	SERVER_TYPE_TASK_QUEUE = 5,
	SERVER_TYPE_THREAD_POOL = 6,
	SERVER_TYPE_THREAD_POOL_BLOCKING = 7,
	SERVER_TYPE_CHAR_DEVICE_QUEUE = 8,

