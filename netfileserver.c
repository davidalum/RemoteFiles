/*

netfileserver.c 

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
#include <pthread.h>

struct file{
  int isOpen;
  int filemode;
  int RWAccess;
  char *pathname;
  struct file *next;
  struct file *prev;
  int newFD;
  int perm;
};
struct file *root;



//Helper function where the thread starts to work

void* helper_Function(void * newSock){
  
  // all the variables required for the varior error checks
  int stat, bindCheck, listenCheck, recvCheck1,recvCheck2 ,sendCheck1,sendCheck2,sendCheck3,sendCheck4, recvCheck;
  int newInSockFD = *(int*) newSock;   //sockFD is the socket file descriptor for server 
  
  int recArr;
  int func;
  int nBytes = 0;
  int arr[5];
  int fmode;
  recvCheck = recv(newInSockFD, arr, 100, 0); 
  //intializing these variables to use in our server
  func = arr[0];
  recArr = arr[1];
  nBytes=arr[2];
  fmode=arr[3];
  int len = arr[4];
  char buff[len];
  
  //checking if there was an error in reiving this info
  if(recvCheck == -1){
    printf("Error in second receiving information from client\n"); 
  }
  
  recvCheck1 = recv(newInSockFD, buff, 10000, 0); // receiving the buffer
  
  if(recvCheck1 == -1){
    printf("Error in first receiving information from client\n"); 
  }
     
  //Beginning of open()
   
  struct file *newFile = malloc (sizeof(struct file));
   
  if(func == 0){
    
    int FD_open  = open(buff,recArr); // opening the file
    
    newFile->RWAccess = recArr;
    newFile->pathname= malloc(strlen(buff)+1);
    strcpy(newFile->pathname,buff);
    newFile->isOpen = 1;
    newFile->newFD=FD_open;
    newFile->filemode=fmode;
    struct file *ptr = malloc (sizeof(struct file));
    ptr=root;
    //This is for extention A
    //creating the first node in the linked list 
    if(root==NULL){
      root=newFile;
      newFile->perm = recArr;
      newFile->next=NULL;   
    }
    //creating the nodes for the other clients 
    else{
      //checks to see the what mode the current file and the other files in teh list are and setting the RWAccess or returning the error
      while(ptr->next!=NULL || ptr==root){
	if(strcmp(ptr->pathname, newFile->pathname)==0){ //check for if its same file
	  if(ptr->filemode==0){  //Client in list already is unrestricted
	    if(newFile->filemode==0){ //both are UN restricted, for all read/write
	      newFile->perm = recArr; 
	    }
	    else if(newFile->filemode==1){
	      if(ptr->isOpen==0){
                newFile->perm = recArr;
              }
	      else{
                if (newFile->RWAccess==0||newFile->RWAccess==2){
                  newFile->perm =0;
                }
                else if (newFile->RWAccess==1 && ptr->RWAccess==0){
                  newFile->perm = 1;
                }
                else if (newFile->RWAccess==1 && (ptr->RWAccess==1|| ptr->RWAccess==2)){
		  FD_open =-1;
		  errno=1;
                }
	      }
	    }
	    else if(newFile->filemode==2){
              if(ptr->isOpen=0){
                newFile->perm=recArr;
              }
              else{
                FD_open=-1;
                errno=1;
              }
            }
	  }
	  else if(ptr->filemode==1){
	    if(newFile->filemode==0){
	      if(ptr->isOpen=0){
                newFile->perm=recArr;
              }
              else{
                if (newFile->RWAccess==0){
		  newFile->perm =recArr;
                }
                else if ((newFile->RWAccess==1||newFile->RWAccess==2) && (ptr->RWAccess==1|| ptr->RWAccess==2))
		  {
		    FD_open=-1;
		    errno=1;
		  }
              }
	    }
	    else  if(newFile->filemode==1){
	      if(ptr->isOpen==0){
                newFile->perm = recArr;
              }
              else{
                if (newFile->RWAccess==0 || newFile->RWAccess ==2){
                  newFile->perm=0;
                }
                else{
                  FD_open=-1;
                  errno=1;
                }
	      }
	    }
	    else if(newFile->filemode==2){
	      if(ptr->isOpen==0){
                newFile->perm = recArr;
              }
	      else{
		FD_open=-1;
		errno=1;
	      }
	    }
	  }
	  else if(ptr->filemode==2){
            if(newFile->filemode==0){
              FD_open=-1;
	      errno=1;
 
            }
            else if(newFile->filemode==1){
              FD_open=-1;
	      errno=1;
            }
            else if(newFile->filemode==2){
              FD_open=-1;
	      errno=1;
            }
	  }
	}
	if(ptr == root){
	  break;
	}
	ptr=ptr->next;
      } 
      ptr->next =newFile;
      newFile->next =NULL;
    }
    
    int res[3] ; //storing the errno value and the data 
    res[0] = FD_open;
    res[1] = errno;
    sendCheck1 = send(newInSockFD, res, 1000, 0); // sending the result back to the libnets 
    if(sendCheck1 == -1){ 
      perror("error: ");
      
    }
  }



  //checking if the file is open or else printing an error message and exiting from the program
  //beginning of read function
  else if(func == 1){
  
    int bytesRead [2];
    char buffer1[1000];
    int noSent[2]; 
    noSent[0]= recArr;
    //getting the number of bytes actually read from the file
  
    bytesRead[0]= read(noSent[0], buffer1, nBytes);
    bytesRead[1]=errno;
  
    sendCheck2 = send(newInSockFD, bytesRead, 1000, 0); // sending the result back to the libnets 
    if(sendCheck2 == -1){ 
      perror("error: ");
      
    }
  }


  //Beginning of the write() funtion
  else if(func == 2){
  
    int bytesWrite[2];
    int i = 0;
    //getting the size of the string we are writing into the file
    while(buff[i]!=0){
      i++;
    }
    char newStr[i];
    strncpy(newStr,buff,i); 
  
    bytesWrite[0] = write(recArr, newStr, nBytes); //changes from i to nBytes
  
    if(bytesWrite[0] == -1){ 
      perror("error: ");
      
    }
    bytesWrite[1]=errno;
    sendCheck3 = send(newInSockFD, bytesWrite, 1000, 0); // sending the result back to the libnets 
    if(sendCheck3 == -1){ 
      perror("error: ");
      
    } 
  }


  //beginnging of the close() function 
  else if (func == 3){
  
    int FD_close  = close(recArr); // sending the file to the open function
    int res[3] ; //storing the errno value and the data 
    res[0] = FD_close;
    res[1] = errno;
    sendCheck4 = send(newInSockFD, res, 1000, 0); // sending the result back to the client  
    if(sendCheck4 == -1){ 
      perror("error: ");
    }
  }
  memset(buff,0,sizeof(buff)); //making the buff empty again
}  



int main(int argc, char **argv){
 
  int stat, bindCheck, listenCheck;//, recvCheck1,recvCheck2 ,sendCheck1,sendCheck2,sendCheck3,sendCheck4, recvCheck;
  int servSockFD, newInSockFD;   //sockFD is the socket file descriptor for server 
  
  struct addrinfo hints, *servList, *servInfo;
  struct sockaddr_in inAddr;
  socklen_t inSize; 
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE; // use my IP
 
  stat = getaddrinfo(NULL, "11135", &hints, &servList);  //first arg is host name, should be argv1? 
  // check if argv[1] is hostname 
  if(stat != 0){  //Error in getAddrInfo 
    printf("ERROR: getAddrInfo did not work properly\n"); 
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(stat));
    exit(1); 
  }


  for(servInfo = servList; servInfo != NULL; servInfo = servInfo ->ai_next){   
    //Going throught the linked list checking for a useable socket to use. 
    servSockFD = socket(servList->ai_family, servList->ai_socktype,servList->ai_protocol);
    if(servSockFD == -1){  //Error on creating a socket for server 
      printf("Error Socket could not be created for server at this particular socket\n"); 
      continue; 
    }
    bindCheck = bind(servSockFD, servInfo -> ai_addr, servInfo->ai_addrlen);
    if(bindCheck == -1){
      printf("Error Bind could not be completed properly for this socket\n");
      close(servSockFD);
      continue; 
    }
    else{break;}
  }  //end of linked list loop 
  freeaddrinfo(servList); //Freeing the linked list of addr info structs 


 
  if(servInfo == NULL){  //If exits the loop without finding a socket 
    printf("ERROR: Server could not bind to a socket\n");
    exit(1);  
  }
  //listening for connections from a client 
  listenCheck = listen(servSockFD, 10 );   //second arg is backlog 
  if(listenCheck == -1){
    printf("Error with listen\n");
    exit(1); 
  }


  while(1){
    // printf("Waiting for connection..\n"); 
    inSize = sizeof(struct sockaddr_in);
    newInSockFD = accept(servSockFD, (struct sockaddr *) &inAddr, &inSize);   //newInFD is FD from client 
   
    if(newInSockFD == -1){
      printf("Error with accepting an incomoing connection\n");  
    }
    int *newSock = malloc(1);
    *newSock = newInSockFD;
    pthread_t worker;
    if(pthread_create(&worker , NULL ,  helper_Function,(void*)newSock) < 0){ //creating a thread 
      perror("could not create thread");
      return 1;
    }
    pthread_join(worker , NULL);
  } //End of continuous loop 
  return 0;
}
