/*

libnetfiles.c 

*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#include "libnetfiles.h"



/*
Opens a file on a remote client. 
Returns a new file descriptor to use in read/write/close functions
Returns 0 and sets the errno value if error is caught. 
File can be opened with the following restrictions:  
Flags are O_RDONLY=0, O_WRONLY=1, O_RDWR=2

*/
int netopen(const char *pathname, int flags ){
 
if(isInitialized != 0){
printf("ERROR: Init was not called or hostname does not exist  \n");
 h_errno = HOST_NOT_FOUND;
return -1; 
}


if(flags == 0 || flags == 1 || flags == 2 ){
}
else{
printf("Flag entered for netopen is invalid\n"); 
return -1;

}



  int bytesSent0, bytesSent1, bytesReceived, connectCheck;
  struct addrinfo *cliInfo; 
  int results[5];
  

for(cliInfo = cliList; cliInfo != NULL; cliInfo = cliInfo ->ai_next){
   //Going throught the linked list checking for a useable socket to use.                                                                                                                                       
   
    cliSockFD = socket(cliList->ai_family, cliList->ai_socktype, cliList->ai_protocol);
    if(cliSockFD == -1){  //Error on creating a socket for server     
      perror("Error: ");   
      printf("Error Socket could not be created for client at this particular socket. Try entering a new PORT NUM \n");
      continue;
    }
    connectCheck = connect(cliSockFD, cliInfo->ai_addr, cliInfo->ai_addrlen);
    if(connectCheck == -1) {
      close(cliSockFD);
      perror("Error: ");
      continue; 
    }
    else{
      break;
    }
  }  //end of linked list loop

if(cliInfo == NULL){
    fprintf(stderr, "Error: Couldn't find a socket to connect. Try entereing a new PORT NUM to\n");
    return -1;
  }
 
//At this point, has created a socket with SocketFD = cliSockFD
//And connected this socket to the server  


 int pathLength = strlen(pathname);

 char *path = (char*) malloc (pathLength +1) ;

 strcpy (path, pathname);
 path[pathLength] = '\0';

 int sentLength = strlen(path);
 printf("Sent lenght: %d\n", sentLength); 
//Sending the INDICATOR to OPEN (0) along with the flags, and filemode. 

int indArr[5];
indArr[0] = 0;
indArr[1] = flags;
indArr[2] = 0;
indArr[3] = serv_mode;
indArr[4] = sentLength;

bytesSent0 = send(cliSockFD, indArr, 5, 0);
printf("bytes sent for indicator: %d\n", bytesSent0); 
if(bytesSent0 == -1){
     perror("error: ");
     exit(1);
}
 



//Sending the PATHNAME

//int sentLength = strlen(path); 
  bytesSent1 = send(cliSockFD, path, sentLength, 0);

if(bytesSent1 == -1){
     perror("error: ");
     exit(1);
}
 

  bytesReceived = recv(cliSockFD, results, 2*sizeof(int), 0);

  if(bytesReceived == -1){
     perror("error: ");
     exit(1);
}


  if(results[0] == -1) {

   errno = results[1];
    perror("Error with netopen: ");
    return -1;
  } 
free(path); 
  return results[0];
}



/*
Reads a remote file
Returns an integer with the number of bytes that were read actually read. 
Returns -1 and sets the errno value appropriately if an error was caught. 
 */

ssize_t netread(int fildes, void *buf, size_t nbyte) {


 if(isInitialized != 0){
printf("ERROR: Init was not called or hostname does not exist  \n");
 h_errno = HOST_NOT_FOUND;
return -1; 
}

struct addrinfo *cliInfo; 
int connectCheck;


for(cliInfo = cliList; cliInfo != NULL; cliInfo = cliInfo ->ai_next){
   //Going throught the linked list checking for a useable socket to use.                                                                                                                                       
   
    cliSockFD = socket(cliList->ai_family, cliList->ai_socktype, cliList->ai_protocol);
    if(cliSockFD == -1){  //Error on creating a socket for server     
      perror("Error: ");   
      printf("Error Socket could not be created for client at this particular socket. Try entereing a new PORT NUM\n");
      continue;
    }
    connectCheck = connect(cliSockFD, cliInfo->ai_addr, cliInfo->ai_addrlen);
    if(connectCheck == -1) {
      close(cliSockFD);
      perror("Error: ");
      continue; 
    }
    else{
      break;
    }
  }  //end of linked list loop

if(cliInfo == NULL){
    fprintf(stderr, "Error: Couldn't find a socket to connect to. Try entering a new PORT NUM.\n");
    return -1;
  }


//Sending the INDICATOR to READ (1), the file descriptor for the file, and number of bytes to be read. 
  
int bytesReceived, bytesSent0, bytesSent1; 
int indArr[3];

int nBytes = (int) nbyte;

indArr[0] = 1;
indArr[1] = fildes;
indArr[2] = nBytes; 

bytesSent0 = send(cliSockFD, indArr, 10, 0);

if(bytesSent0 == -1){
     perror("error: ");
     exit(1);
}
 



//Sending the PATHNAME

char *path = "A"; 
printf("New path in READ: %s\n", path);
int sentLength = strlen(path); 
  bytesSent1 = send(cliSockFD, path, sentLength, 0);
printf("bytes sent path: %d\n", bytesSent1); 
if(bytesSent1 == -1){
     perror("error: ");
     exit(1);
}



//Recv results from the server 

int results [4]; 
//results[0] = "# of bytes read" on success, "-1" on error. 
//results[1] = "0" on success, "Errno value" on error 
 bytesReceived = recv(cliSockFD, results, 2*sizeof(int), 0);
  //bytesReceived = recv(cliSockFD, buff, 100, 0);
  if(bytesReceived == -1){
     perror("Error: ");
     exit(1);
}

//Setting the errno value
  if(results[0] == -1) {
   errno = results[1];
    perror("Error: ");
    return -1;
  } 

  return results[0];

}


/*
Writes to a remote file
Returns an int with the number of bytes actually written to the file. 
Returns -1 and sets the appropriate errno value if error is caught. 
 */


ssize_t netwrite(int fildes, const void *buf, size_t nbyte) {

struct addrinfo *cliInfo; 
int connectCheck;


if(isInitialized != 0){
printf("ERROR: Init was not called or hostname does not exist  \n");
 h_errno = HOST_NOT_FOUND;
return -1; 
}

for(cliInfo = cliList; cliInfo != NULL; cliInfo = cliInfo ->ai_next){
   //Going throught the linked list checking for a useable socket to use.                                                                                                                                       
   
    cliSockFD = socket(cliList->ai_family, cliList->ai_socktype, cliList->ai_protocol);
    if(cliSockFD == -1){  //Error on creating a socket for server     
      perror("Error: ");   
      printf("Error Socket could not be created for client at this particular socketry entering a new PORT NUM.\n");
      continue;
    }
    connectCheck = connect(cliSockFD, cliInfo->ai_addr, cliInfo->ai_addrlen);
    if(connectCheck == -1) {
      close(cliSockFD);
      perror("Error: ");
      continue; 
    }
    else{
      break;
    }
  }  //end of linked list loop

if(cliInfo == NULL){
    fprintf(stderr, "Error: Couldn't find a socket to connect to. Try entering a new PORT NUM.\n");
    return -1;
  }




//Sending the INDICATOR to WRITE (2), the file descriptor and the number of bytes to write.   
int bytesReceived, bytesSent0, bytesSent1, nBytes; 
int indArr[3];

nBytes = (int) nbyte;
 
indArr[0] = 2;
indArr[1] = fildes;
indArr[2] = nBytes; 
 printf("nbytes: %d\n", nBytes);
bytesSent0 = send(cliSockFD, indArr, 10, 0);
  
if(bytesSent0 == -1){
     perror("error: ");
     exit(1);
}
 




//Sending the BUFFER
int buffLen = strlen(buf); 
char path[buffLen + 1]; 
strcpy(path, buf); 
printf("Buffer sent in WRITE: %s\n", path);
  bytesSent1 = send(cliSockFD, path, buffLen, 0);

if(bytesSent1 == -1){
     perror("error: ");
     exit(1);
}


int results [4]; 
 bytesReceived = recv(cliSockFD, results, 2*sizeof(int), 0);
  //bytesReceived = recv(cliSockFD, buff, 100, 0);
  if(bytesReceived == -1){
     perror("error: ");
     exit(1);
}


  if(results[0] == -1) {
   errno = results[1];
    perror("Error: ");
    return -1;
  } 

  return results[0];


}

/*
Closes a remote file or returns the errno value if an error was caught
Returns 0 if the file is closed properly. 
Returns -1 and sets the errno appropriately if an error is caught.  
*/

int netclose(int fd) {
 
struct addrinfo *cliInfo; 
int connectCheck;


if(isInitialized != 0){
printf("ERROR: Init was not called or hostname does not exist  \n");
 h_errno = HOST_NOT_FOUND;
return -1; 
}

for(cliInfo = cliList; cliInfo != NULL; cliInfo = cliInfo ->ai_next){
   //Going throught the linked list checking for a useable socket to use.                                                                                                                                       
   
    cliSockFD = socket(cliList->ai_family, cliList->ai_socktype, cliList->ai_protocol);
    if(cliSockFD == -1){  //Error on creating a socket for server     
      perror("Error: ");   
      printf("Error Socket could not be created for client at this particular socket. Try entering a new PORT NUM.\n");
      continue;
    }
    connectCheck = connect(cliSockFD, cliInfo->ai_addr, cliInfo->ai_addrlen);
    if(connectCheck == -1) {
      close(cliSockFD);
      perror("Error: ");
      continue; 
    }
    else{
      break;
    }
  }  //end of linked list loop

if(cliInfo == NULL){
    fprintf(stderr, "Error: Couldn't find a socket to connect to. Try entering a new PORT NUM.\n");
    return -1;
  }




//Sending the INDICATOR to CLOSE (3)  
int bytesReceived, bytesSent0, bytesSent1; 
int indArr[2];
indArr[0] = 3;
indArr[1] = fd;

bytesSent0 = send(cliSockFD, indArr, 5, 0);

if(bytesSent0 == -1){
     perror("error: ");
     exit(1);
}
 


//Sending the BUFFER

char *path = "A"; 

  bytesSent1 = send(cliSockFD, path, 5, 0);

if(bytesSent1 == -1){
     perror("error: ");
     exit(1);
}




int results [4]; 
 bytesReceived = recv(cliSockFD, results, 2*sizeof(int), 0);

  if(bytesReceived == -1){
     perror("error: ");
     exit(1);
}


  if(results[0] == -1) {
  
   errno = results[1];
    perror("Error: ");
    return -1;
  } 

  return results[0];



}



/*Checks that the server can be connected to. Returns 0 on success, -1 on error 
Filemode unrestricted=0, exclusive=1, transaction=2 
Should be the first function called from a client in order to establish a connection. 

Returns 0 on succes, -1 on error. 
*/

int netserverinit(char * hostname,int filemode) {
  
  int addrTest; 
  struct addrinfo hints; //servinfo requirements
  char portNum[6]; //hardcoded port number


  serv_mode = filemode; 
  memset(&hints, 0, sizeof hints); //makes sure hints is empty
  hints.ai_flags = AI_PASSIVE; 
  hints.ai_family = AF_INET; //IPv4
  hints.ai_socktype = SOCK_STREAM; //TCP stream sockets
  

  snprintf(portNum, 6, "%d", PORT_NUM); 

  //sets h_errno
 addrTest = getaddrinfo(hostname, portNum, &hints, &cliList);

  if(addrTest != 0) {
    h_errno = HOST_NOT_FOUND;  //check if proper errno 
    herror("Error: ");
    isInitialized = -1; 
    return -1;   //returns -1 if error occurred while initializing server 

}
isInitialized = 0;  
  return 0;

}
