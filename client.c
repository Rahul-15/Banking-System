#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <netinet/in.h>   
#include <string.h>

int main(){
    struct sockaddr_in server;
    int socket_desc;
    char buff[1024];
    socket_desc = socket(AF_INET,SOCK_STREAM,0);

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(1123);
    connect(socket_desc, (void *)(&server), sizeof(server));
    printf("--------------------Welcome to the banking sys-------------------\n\n\n");
    
    
    while(1){
        while(1){
        //printf("--------------Welcome to Banking System -------------------\n\n");
        printf("Kindly enter your credentials:\n");
        char uname[1024]; char userpwd[1024];                                               // for storing password 
        int i; 
        printf("username : "); 
        scanf("%s",uname); 
        printf("password : "); 
        scanf("%s",userpwd); 
        send(socket_desc,uname, sizeof(uname),0);                                           // sending user name to server
        send(socket_desc,userpwd, sizeof(userpwd),0);                                       // sending user password
        buff[0] = '\0';
        read(socket_desc, buff, sizeof(buff));
        printf("server result: %s\n",buff);


        if(strcmp("authentic user", buff) == 0){
            printf("-------------------Welcome: %s-------------------\n\n", uname);
            while(1){

                buff[0]='\0';                                        
                read(socket_desc, buff, sizeof(buff));                                      // reading instn. from server
                printf("%s", buff);                                                          // printing what server has send.
                
                buff[0]='\0';
                scanf("%s",buff);
                
                if(strcmp("-1", buff) == 0)break;                                           // takeing i/p from client
                
                write(socket_desc,buff, sizeof(buff));                                    // conveying to server
                
            }
        }
        else{
            printf("No information regarding this username\n");
        }
        printf("------------------- exit !! -------------------\n\n");        
    }
    }
    return 0;
}