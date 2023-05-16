#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <sys/time.h>
int main(int argc , char *argv[])
{
  int sockfd = 0;
  char nick[1024];

  printf("Please Enter Your Name: ");
  scanf(" %[^\n]",nick);

  //socket
  if((sockfd = socket(AF_INET,SOCK_STREAM,0)) == -1){
    printf("Fail to create a socket.");
  }

  struct sockaddr_in info;
  bzero(&info,sizeof(info));
  info.sin_family = PF_INET;
  info.sin_addr.s_addr = inet_addr(argv[1]);
  info.sin_port = htons(atoi(argv[2]));

  //connect
  int err = connect(sockfd,(struct sockaddr *)&info,sizeof(info));
  if(err == -1){
    printf("Fail to Connect.");
  }
  else if(err >= 0){
    printf("Connection to the Server.\n");
    send(sockfd,nick,sizeof(nick),0);
  }

  fd_set masterfds;
  char buf[1024];
  char message[1024];

  FD_ZERO(&masterfds);
  memset(buf,0,1024);
  memset(message,0,1024);
  FD_SET(sockfd,&masterfds);

  while(1){
    int nread;
    fd_set readfds;
    struct timeval tv;
    int fd = fileno(stdin);
    FD_ZERO(&readfds);
    FD_SET(fd,&readfds);
    tv.tv_sec = 0;
    tv.tv_usec = 1;

    int ret = select(fd + 1, &readfds, NULL, NULL, &tv);
    if(ret != 0){
      memset(buf,0,1024);
      fscanf(stdin," %[^\n]",buf);
      if(strcmp(buf,"exit") == 0){
        break;
      }

      memset(message,0,102);
      snprintf(message,sizeof(message),"%s: %s",nick,buf);
      send(sockfd,message,sizeof(message),0);
    }

    ioctl(sockfd,FIONREAD,&nread);
    if(nread != 0){
      if(FD_ISSET(sockfd,&masterfds)){
        memset(buf,0,1024);
        recv(sockfd,buf,sizeof(buf),0);
        printf("%s\n",buf);
      }
    }
  }
  printf("close Socket\n");
  close(sockfd);
  return 0;
}