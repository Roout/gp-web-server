# Webserver

A simple webserver which response to any **GET** and **POST** request.
It doesn't handle files, possible responses are fixed.  
Server uses model 1 process for 1 request. Number of processes (as well as connections) is limited by macros `MAX_CONNECTIONS` in `main.c` file.
In other words server will wait until any child process terminates if current number of processes is equal `MAX_CONNECTIONS`.
It also tries to reap all exit codes of the terminated child processes in non-blocking mode beforehand. And blocks only if no process has been terminated yet.

## Configuration

Current server configuration (all macroses in `main.c`):

```C
#define MAX_CONNECTIONS 1000
#define HOST "127.0.0.1"
#define PORT "18001"
#define BACKLOG 64
```

`BACKLOG` used for `listen` syscall.

## Quick Start

```console
$ make
$ ./webserver
```

## Tests

Execute bash script `./run.sh` when the server is running. It will create 4 **GET**, 4 **POST** and 4 **PUT** detached requests via `curl -X request_type http://localhost:18001` command.
Each of **GET** and **POST** requests will print **GET** and POST as response.

## Responses

Responses which are sent by server:

For **GET** request:
```console
HTTP/1.1 200 OK\r\n
Content-Length: 3\r\n
\r\n
GET
```

For **POST** request:
```console
HTTP/1.1 200 OK\r\n
Content-Length: 4\r\n
\r\n
POST
```

For other requests:
```console
HTTP/1.1 400 Bad Request\r\n
Content-Length: 0\r\n
\r\n
```
