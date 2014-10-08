Setup
=====

1) Run "make" from terminal to initialize.<br>
2) Run " ./server 'port#' 'option#' " to use. (i.e. "./server 8080 0")

Details
=====

Options available for "option#" in Setup:

"0": serve only a single request <br>
"1": use only a single thread for multiple requests <br>
"2": use fork to create process for each request <br>
"4": create new thread for each request <br>
"5": use atomic instructions to implement task queue <br>
"6": use a thread pool to serve requests (in progress) <br>
