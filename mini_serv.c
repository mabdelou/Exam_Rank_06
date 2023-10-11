#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>

#define MAX_BUFF 200000
#define MAX_CLIENTS_BUFF 3000
#define MAX_CLIENTS 128

uint16_t ft_htons(int port)
{
    int hex_right = port >> 8 & 0xffff;
    int hex_left = port << 8 & 0xffff;
    return (hex_right | hex_left);
}

void ft_exit(char *error_msg,int fd)
{
    write(fd,error_msg,strlen(error_msg));
    exit(-1);
}

int main(int argc, char **argv)
{
    fd_set read_fds;
    struct sockaddr_in addr,cli;

    int serv_socket = 0;
    int clients_socket[MAX_CLIENTS_BUFF] = {0};
    int current_clients = 0;
    int connected_clients = 0;
    int new_client = 0;
    int len=0,pos=0,bits=0;
    int access = 1;
    unsigned int clilen = sizeof(cli);

    char buff[MAX_BUFF] = {};
    char msg[MAX_BUFF] = {};

    if(argc == 1)
        ft_exit("Wrong number of arguments\n",2);

    bzero(&addr,sizeof(addr));
    bzero(&cli,sizeof(cli));
    bzero(&clients_socket,MAX_CLIENTS_BUFF*4);
    bzero(&buff,MAX_BUFF);
    bzero(&msg,MAX_BUFF);

    addr.sin_family = AF_INET; 
    addr.sin_addr.s_addr = 16777343;
    addr.sin_port = htons(atoi(argv[1]));

    if((serv_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        ft_exit("Fatal error\n",2);
    else if(bind(serv_socket,(const struct sockaddr *)&addr,sizeof(addr)) == -1)
        ft_exit("Fatal error\n",2);
    else if(listen(serv_socket,MAX_CLIENTS))
        ft_exit("Fatal error\n",2);
    current_clients = serv_socket;
    while(true)
    {
        bzero(&buff,MAX_BUFF);
        bzero(&msg,MAX_BUFF);
        FD_ZERO(&read_fds);
        FD_SET(serv_socket,&read_fds);

        for(int a=0;a<connected_clients;a++)
        {
            if(clients_socket[a] > 0)
            {
                FD_SET(clients_socket[a],&read_fds);
                if(clients_socket[a] > current_clients)
                    current_clients  = clients_socket[a];
            }
        }
        select(current_clients+1,&read_fds,NULL,NULL,NULL);

        if(FD_ISSET(serv_socket,&read_fds))
        {
            new_client = accept(serv_socket,(struct sockaddr *)&cli,&clilen);
            if(new_client > 0)
            {
                connected_clients++;
                for(int a=0;a<connected_clients;a++)
                {
                    if(clients_socket[a] == 0)
                    {
                        clients_socket[a] = new_client;
                        if(clients_socket[a] > current_clients)
                            current_clients  = clients_socket[a];
                        sprintf(msg,"server: client %d just arrived\n",a);
                        for(int b=0;b<connected_clients;b++)
                            if(b != a && clients_socket[b] > 0)
                                send(clients_socket[b],msg,strlen(msg),0);
                        bzero(&buff,MAX_BUFF);
                        bzero(&msg,MAX_BUFF);
                        break;
                    }
                }
            }
        }
        for(int a=0;a<connected_clients;a++)
        {
            if(clients_socket[a] > 0)
            {
                if(FD_ISSET(clients_socket[a],&read_fds))
                {
                    bits = recv(clients_socket[a],buff,MAX_BUFF,0);
                    if(bits == 0)
                    {
                        sprintf(msg,"server: client %d just left\n",a);
                        for(int b=0;b<connected_clients;b++)
                            if(b != a && clients_socket[b] > 0)
                                send(clients_socket[b],msg,strlen(msg),0);
                        close(clients_socket[a]);
                        clients_socket[a] = -1;
                        bzero(&buff,MAX_BUFF);
                        bzero(&msg,MAX_BUFF);
                    }
                    else if(bits > 0)
                    {
                        if(access)
                        {
                            sprintf(msg,"client %d: ",a);
                            pos = len = strlen(msg);
                            access = 0;
                        }
                        else
                        {
                            sprintf(msg,"client %d: ",a);
                            pos = 0;
                            len = strlen(msg);
                            bzero(&msg,MAX_BUFF);
                        }

                        for(int b=0;buff[b];b++,pos++)
                        {
                            sprintf(&msg[pos],"%c",buff[b]);
                            if(buff[b] == '\n' && buff[b+1] != '\0')
                            {
                                sprintf(&msg[pos+1],"client %d: ",a);
                                pos += len;
                            }
                            else if(buff[b] == '\n' && buff[b+1] == '\0')
                            {
                                access = 1;
                            }
                        }
                        for(int b=0;b<connected_clients;b++)
                            if(b != a && clients_socket[b] > 0)
                                send(clients_socket[b],msg,strlen(msg),0);
                        bzero(&buff,MAX_BUFF);
                        bzero(&msg,MAX_BUFF);
                    }
                }
            }
        }
    }
    return 0;
} 