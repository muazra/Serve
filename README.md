Serve
=====

A simple Web-server written for the Linux kernel.

Setup
=====

1) Run "make" from terminal to initialize. 
2) Run "./server <port> <option>" to use.

Details
=====

Options available for <option> in Setup:

"0": serve only a single request\n
"1": use only a single thread for multiple requests\n
"2": use fork to create process for each request\n
"4": create new thread for each request\n
"5": use atomic instructions to implement task queue\n
"6": use a thread pool to serve requests (in progress)\n
