/*
*Auth: Brayan Lemus Gonzalez
*Date: 11/30/2021 (Due: 12/12/2021)
*Course: CSCI-3550 (Sec:851)
*Desc: PROJECT-01, Server
*This program will accept in the command line the port number that the server is going to connect to and 
*it is going to be wating for a connection from the client program to recived data.
*/

#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
 #include <fcntl.h>
 #include <unistd.h>
 #include <sys/stat.h>
 #include<sys/socket.h>
 #include<arpa/inet.h>
 #include<sys/types.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include<string.h>
  #include <errno.h>
  #include<signal.h>
       #define _BSD_SOURCE
  #define BUF_SIZE (100*1024*1024)
 char *buf = NULL;
int fd_out;
char fname[80];

    int cl_sockfd; /*Socket descriptor for accepted clients*/
        int sockfd = 0;

/*This is the clean up function which cleans once the program is done with th work it has to do it close the file and dealocate memory
as well as closing the client and the server socket*/

void cleanup(void)
{
    if( buf != NULL ) {
    free( buf ); /* Deallocate memory. */
    buf = NULL;  /* Safety first! */
    }
    if (fd_out >= 0)
    {
        close(fd_out);
        fd_out = -1;
    }

    if (sockfd)
    {
        close(sockfd);
        sockfd = 0;
    }
    if (cl_sockfd)
    {
        close (cl_sockfd);
        cl_sockfd = 0;
    }
}
/*This function gives the program a sing which is CONTROL + C that closess the program it got cathc with this function and closses the server*/
void SIGINT_handler( int sig ) {

   /* Issue a message */
   fprintf( stderr, "Server interrupted.  It will now close.\n" );
   
   /* Cleanup */
   cleanup();

   /* Terminate the program HERE */
   exit( EXIT_FAILURE );

}



int main (int argc, char *argv[])
{
    if (argc >= 2)
    {
    int flag = 1;
    int file_cntr = 1;
    int bytes_read;/* The number of bytes I have read*/
    int byte_written;
    char *buf = NULL; /* for stroing incoming data receive*/
    int val = 1;
    /*declaring the Internet address and the socket address*/
    struct in_addr ia; /*internet address*/
    struct sockaddr_in sa;/*sockect address*/
    struct sockaddr_in cl_sa; /*peer/clent scoket address when connected*/
    const char *port_str = argv[1];
    unsigned short int port = (unsigned short int) atoi (port_str);
    /*store the isze of the socket address tructure so we can pass it to the accept() function*/
     socklen_t cl_sa_size = sizeof( cl_sa);
                      signal(SIGINT,SIGINT_handler);
    

    if (port >= 0 && port <= 1023)
    {
        fprintf(stderr,"server: ERROR: Port number is privileged\n");
        fflush(stderr);
    }

    buf = (void*) malloc(BUF_SIZE); /*Allocate the buffers*/
    if (buf == NULL)
    {
        fprintf(stderr,"server: ERROR: Could not allocate memory.\n");
        fflush(stderr);
    }
    /*Creatringg a new TCP socket*/
    if ((sockfd = socket(AF_INET,SOCK_STREAM, 0))==-1)
    {
        fprintf(stderr,"server: ERROR: Failed to create a socket.\n");
        fflush(stderr);
    }
    

     /*make sure it can be reused*/
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,(const void*)&val, sizeof(int) ) != 0)
    {
        fprintf(stderr, "server: ERROR: setsockopt() failed.\n");
        fflush(stderr);
    }

    /*Cheking if that suceed*/
    if (sockfd < 0 )
    {
        fprintf(stderr,"server: ERROR: Unable to obtain a new socket.\n ");
        fflush(stderr);

    }
   
    if ( (inet_aton ("127.0.0.1", &ia )) == 0) 
    {
        fprintf(stderr, "server: ERROR: setting the IP address.\n");
        fflush(stderr);
    }

   
   ia.s_addr = inet_addr( "127.0.0.1" ); 
 
sa.sin_family = AF_INET;  /* Use IPv4 addresses */ 
sa.sin_addr   = ia;   /* Attach the structure for IP address */ 
sa.sin_port = htons( port ); 
    

    if (bind( sockfd, (struct sockaddr *) &sa, sizeof(sa) ) != 0 )
    {
        fprintf( stderr, "server: ERROR: Failed to bind socket.\n" );
        fflush(stderr);
    }
     
     if (listen(sockfd, 32) != 0)
     {
         fprintf(stderr,"Server: ERROR: Failed to switch socket to listening socket.\n");
         fflush(stderr);

     }
     


     

    

    while (flag)
    {
        printf("server: Awaiting TCP connections over port %s...\n",argv[1]);
        fflush(stdout);
        cl_sockfd = accept(sockfd,(struct sockaddr*) &cl_sa, &cl_sa_size);
                     /*Open a file*/   
                     sprintf(fname,"file-%02d.dat",file_cntr);    
                fd_out = open(fname,O_CREAT|O_WRONLY|O_TRUNC, S_IRUSR|S_IWUSR);
                if (fd_out < 0)
                 {
                    fprintf(stderr, "server: ERROR: Failed to create: %s.\n",fname);
                    fflush(stderr);
                    cleanup();
                    exit(EXIT_FAILURE);
                }

        if (cl_sockfd > 0)
        {
            printf("server: Connection accepted...\n");
            fflush(stdout);
            
            while (bytes_read >= 0)
            {
                 /*clear all the bytes of the structure to zero*/
     memset ((void*) &cl_sa,0,sizeof(cl_sa));
                /*Attempt to read data from the socket*/
            bytes_read = recv(cl_sockfd, (void*) buf, BUF_SIZE, 0);
            
            
                if (bytes_read > 0)
                {
                    /*check how many bytes we read*/
              printf("server: Receiving file...\n");
              fflush(stdout);
                    /*do someting whith the data*/
                byte_written =  write(fd_out,buf,(size_t) bytes_read);
                  if (byte_written != bytes_read)
                    {
                        fprintf(stderr, "server: ERROR: Unable to write: %s.\n",fname);
                        fflush(stderr);
                        cleanup();
                        exit(EXIT_FAILURE);
                        close(fd_out);

                    }
                }
                else if (bytes_read == 0)
                {
                        printf("server: Connection closed.\n");
                        printf("server: Saving file: %s\n",fname);
                        printf("server: Done.\n\n\n");
                        fflush(stdout);
                        if (fd_out >= 0)
                      {
                            close(fd_out);
                        fd_out = -1;
                        }
                
                    if (cl_sockfd)
                        {
                          close (cl_sockfd);
                        cl_sockfd = 0;
                        }
                        break;
                }
                else if (bytes_read < 0)
                {
                     fprintf(stderr, "server: ERROR: Reading from socket.\n");
                     fflush(stderr);
                }
            }

        }
        else
        {
            fprintf(stderr, "server: ERROR: while attempting to accept a connection.\n");
            fflush(stderr);
        }
        
        file_cntr++; 
    }

    
}
    else 
    {
        fprintf(stderr,"server: USAGE: server <Port>\n");
        fflush(stderr);
    }
    return 0;
}
