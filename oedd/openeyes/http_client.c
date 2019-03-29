/**
 * @file http_client.c
 *
 */

/*********************
 *      INCLUDES
 *********************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <sys/types.h>
#include <fcntl.h>
#include "openeyes_gui.h"

#if GUI_USE_HTTP

int http_socket_connect(char *host, in_port_t port){
	struct hostent *hp;
	struct sockaddr_in addr;
	int sock=0;
	struct timeval tv;

	if((hp = gethostbyname(host)) == NULL){
		return -1;
	}

    memcpy(&addr.sin_addr.s_addr,hp->h_addr,hp->h_length);

	addr.sin_port = htons(port);
	addr.sin_family = AF_INET;
	sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	//setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (const char *)&on, sizeof(int));
	tv.tv_sec = 10;
	tv.tv_usec = 0;
	setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

	if(sock == -1){
		return -1;
	}

	if(connect(sock, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) == -1){
		return -1;
	}

	return sock;
}

int http_send_event(char type, char *message)
{
    int message_size;
	int i;
	char buffer[1024];
	int sockfd;
    char *host;
    int portno;
    int bytes;
    int sent, total;
    int received;

	host = HTTP_EVENT_ADDRESS;
	portno = HTTP_EVENT_PORT;

     /* fill in the parameters */
	switch(type)
	{
		case HTTP_TYPE_GET:
			sprintf( buffer, "GET /gui/?%s HTTP/1.1\r\n", message );
			strcat( buffer, "\r\n");                                /* blank line     */
			break;
		case HTTP_TYPE_POST:
			sprintf(buffer,"POST /gui HTTP/1.1\r\n");
			sprintf(buffer+strlen(buffer),"Content-Length: %d\r\n",strlen(message));
			strcat(buffer,"\r\n");                                /* blank line     */
			strcat(buffer,message);
			break;
		default:
			return -1;
    }

	message_size=strlen(buffer);

    /* What are we going to send? */
    printf("Request:(%d)\n%s\n",message_size,buffer);

	sockfd=http_socket_connect(host,(in_port_t)portno);
	if(sockfd<0)
		return sockfd;

    /* send the request */
    total = message_size;
    sent = 0;
    do {
        bytes = write(sockfd,buffer+sent,total-sent);
        if (bytes < 0)
        	return -1;
        if (bytes == 0)
            break;
        sent+=bytes;
    } while (sent < total);

    printf("Trasmitted %d bytes\n",total);

    /* receive the response */
    memset(buffer,0,sizeof(buffer));
    total = sizeof(buffer)-1;
    received = 0;
    do {
        bytes = read(sockfd,buffer+received,2048);
        if (bytes < 0)
            return -1;
        if (bytes == 0)
            break;
        printf("Received %d bytes\n%s",bytes,buffer);
        received+=bytes;
    } while (received < total);

    if (received == total)
        return -1;

    /* close the socket */
    close(sockfd);

    /* process response */
    printf("Response:\n%s\n",buffer);

    return 0;
}
#endif
