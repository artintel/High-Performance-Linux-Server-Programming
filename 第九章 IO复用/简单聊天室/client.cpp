/*
 * @Author: Mayc
 * @Date: 2021-01-28 17:03:28
 * @LastEditTime: 2021-01-28 17:03:38
 * @FilePath: \undefinedd:\VSC++&GO\High-Performance-Linux-Server-Programming\第九章\简单聊天室\client.cpp
 * @这是一个漂亮的注释头
 */
#define _GNU_SOURCE 1
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>
#include <poll.h>

#define BUFFER_SIZE 64
int main( int argc, char* argv[] ){
    if( argc <= 2 ){
        printf( "usage: %s ip_address port_number\n", basename( argv[0] ) );
        return 1;
    }
    const char* ip = argv[1];
    int port = atoi( argv[2] );
    struct sockaddr_in server_address;
    bzero( &server_address, sizeof( server_address ) );
    server_address.sin_family = AF_INET;
    inet_pton( AF_INET, ip, &server_address.sin_addr );
    server_address.sin_port = htons( port );

    int sockfd = socket( PF_INET, SOCK_STREAM, 0 );
    assert( sockfd >= 0 );
    if ( connect( sockfd, ( struct sockaddr*)&server_address, sizeof( server_address ) ) < 0 ){
        printf( "connection failed\n" );
        close( sockfd );
        return 1;
    } 
    pollfd fds[2];
    /* 注册文件描述符 0 (标准输入) 和文件描述符 sockfd 上的可读事件 */
    fds[0].fd = 0;
    fds[0].events = POLLIN;
    fds[0].revents = 0;
    fds[1].fd = sockfd;
    fds[1].events = POLLIN | POLLRDHUP;
    fds[1].revents = 0;
    char read_buf[ BUFFER_SIZE ];
    int pipefd[2];
    int ret = pipe( pipefd );
    assert( ret != -1 );
    while( 1 ){
        ret = poll( fds, 2, -1 );
        if( ret < 0 ){
            printf( "poll failure\n" );
            break;
        }
        if( fds[1].revents & POLLRDHUP ){
            printf( "server close the connection\n" );
            break;
        }
        else if( fds[0].revents & POLLIN ){
            /* 使用 splice 将用户输入的数据直接写到 sockfd 上(零拷贝) */
            /* 0 标准输入 */
            /* pipefd[0] 只能从 pipefd[1] 读取数据 */
            ret = splice( 0, NULL, pipefd[1], NULL, 32768, SPLICE_F_MORE | SPLICE_F_MOVE );
            ret = splice( pipefd[0], NULL, sockfd, NULL, 32768, SPLICE_F_MORE | SPLICE_F_MOVE );
        }
    }
    close( sockfd );
    return 0;
}