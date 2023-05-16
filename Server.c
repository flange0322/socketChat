#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#define order_count 2
void broadcast(fd_set fds,int fdmax,int fd,int sockfd,char *temp);
int order(char nickset[20][1024],int fd,char *message,char *buf);
int main(int argc,char *argv[])
{
  int sockfd = 0,forClientSockfd = 0;

  //socket
  if((sockfd = socket(AF_INET,SOCK_STREAM,0)) == -1){
    printf("Fail to create a socket.");
  }

  struct sockaddr_in serverInfo,clientInfo;
  unsigned int addrlen;
  bzero(&serverInfo,sizeof(serverInfo));
  serverInfo.sin_family = PF_INET;
  serverInfo.sin_addr.s_addr = INADDR_ANY;
  serverInfo.sin_port = htons(atoi(argv[1]));

  //bind
  if(bind(sockfd,(struct sockaddr*)&serverInfo,sizeof(serverInfo)) == -1){
    printf("Fail to bind.");
  }

  //listen
  if(listen(sockfd,5) == -1){
    printf("Fail to listen");
  }

  fd_set masterfds,readfds;
  int fdmax;
  char buf[1024];
  char message[1024];
  char nickset[20][1024];
  char system_message_into[1024] = {"\"進入了聊天室"};
  char system_message_leave[1024] = {"\"離開了聊天室"};
  char cast[1024];
  int ret;

  FD_ZERO(&masterfds);
  FD_ZERO(&readfds);
  memset(buf,0,1024);
  memset(message,0,1024);
  FD_SET(sockfd,&masterfds);

  fdmax = sockfd;

  printf("%s\n","Waiting for the Client to Connect...");
  while(1){
    int nread;
    readfds = masterfds;

    //Select
    if((ret = select(fdmax+1,&readfds,NULL,NULL,NULL)) < 1){
      printf("Fail to Select");
    }

    for(int fd = 0;fd <= fdmax;fd++){
      if(FD_ISSET(fd,&readfds)){
        if(fd == sockfd){
          addrlen = sizeof(clientInfo);
          forClientSockfd = accept(sockfd,(struct sockaddr*) &clientInfo, &addrlen);
          FD_SET(forClientSockfd,&masterfds);

          recv(forClientSockfd,nickset[forClientSockfd],sizeof(nickset[forClientSockfd]),0);
          printf("nick: %s,fd: %d\n",nickset[forClientSockfd],forClientSockfd);
          memset(cast,0,1024);
          snprintf(cast,sizeof(cast),"Server廣播:\" %s %s",nickset[forClientSockfd],system_message_into);

          if(forClientSockfd > fdmax){
            fdmax = forClientSockfd;
          }
          broadcast(masterfds,fdmax,forClientSockfd,sockfd,cast);

          printf("%s\n","Connect Successed.");
        }
        else{
          ioctl(fd,FIONREAD,&nread);
          if(nread == 0){
            memset(cast,0,1024);
            snprintf(cast,sizeof(cast),"Server廣播:\" %s %s",nickset[fd],system_message_leave);
            memset(nickset[fd],0,1024);

            broadcast(masterfds,fdmax,fd,sockfd,cast);

            close(fd);
            FD_CLR(fd,&masterfds);
            printf("FD:%d leave the chat.\n",fd);
          }
          else{
            memset(buf,0,1024);
            if(recv(fd,buf,sizeof(buf),0)){
              printf("recv | %s\n",buf);
            }

           if(order(nickset,fd,message,buf) != 1){
              for(int j = 0;j <= fdmax; j++){
                if(FD_ISSET(j,&masterfds)){
                  if(j != sockfd){
                    if(send(j,message,sizeof(message),0)){
                      printf("send to %d | %s\n",j,message);
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }
  printf("close Socket\n");
  close(sockfd);
  return 0;
}

void broadcast(fd_set fds,int fdmax,int fd,int sockfd,char *temp){
   char order_tip[1024] = {"提示:輸入\"!ORDER\"可以得知聊天室內的指令種類"};
   for(int k = 0;k <= fdmax;k++){
     if(FD_ISSET(k,&fds)){
       if(k != fd && k != sockfd){
         send(k,temp,strlen(temp),0);
       }
       else if(k == fd){
         send(k,order_tip,strlen(order_tip),0);
       }
     }
   }
}

int order(char nickset[20][1024],int fd,char *message,char *buf){
   char orders[order_count][1024] = {"!ORDER","!CONNECT"};

   int true_false = 1;
   char tmp[1024];
   for(int j = 0;j < order_count;j++){
     memset(tmp,0,1024);
     snprintf(tmp,sizeof(tmp),"%s: %s",nickset[fd],orders[j]);
     memset(message,0,1024);
     if(strcmp(buf,tmp) == 0 && j == 0){
       strcat(message,"==========指令清單==========\n");
       for(int k = 0;k < order_count;k++){
         strcat(message,orders[k]);
         strcat(message,"\n");
       }
       strcat(message,"============================");
       send(fd,message,strlen(message),0);
       true_false = 1;
       break;
     }
     else if(strcmp(buf,tmp) == 0 && j == 1){
       strcat(message,"==========目前連線中的人員清單==========\n");
       for(int k = 0;k < 20;k++){
         if(strcmp(nickset[k],"\0") != 0){
           strcat(message,nickset[k]);
           strcat(message,"\n");
         }
       }
       strcat(message,"========================================");
       send(fd,message,strlen(message),0);
       true_false = 1;
       break;
     }//由此加入新指令，需更動最上面的order_count個數，以及在orders內增加的指令名稱
     else if(strcmp(buf,tmp) != 0 && j == order_count - 1){
       strcpy(message,buf);
       true_false = 0;
       break;
     }
   }
   return true_false;
}
