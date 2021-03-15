/*
 * @Author: Mayc
 * @Date: 2021-01-17 18:44:11
 * @LastEditTime: 2021-01-17 18:44:24
 * @FilePath: \undefinedd:\VSC++&GO\High-Performance-Linux-Server-Programming\第六章&第七章\daemonize.c
 * @这是一个漂亮的注释头
 */
bool daemonize(){
    /* 创建子进程，关闭父进程， 这样可以使程序在后台运行 */
    pid_t pid = fork();
    if( pid < 0 ) return false;
    else if( pid > 0 ) exit();
    /* 设置文件权限掩码。当进程创建新文件 (使用 open( const char* pathname, int flags, mode_t mode ) 系统调用) 时，
    文件的权限将是 mode & 0777 */
    umask( 0 );
    /* 创建新的会话，设置本进程为进程组的首领 */
    pid_t sid = setsid();
    if ( sid < 0 ) return false;
    /* 切换工作目录 */
    if ( ( chdir( "/" ) ) > 0 ) return false;
    /* 关闭标准输入设备、标准输出设备和标准错误输出设备 */
    close( STDIN_FILENO );
    close( STDOUT_FILENO );
    close( STDERR_FILENO );
    /* 关闭其他已经打开的文件描述符，代码省略 */
    /* 将标准输入、标准输出和标准错误输出都定向到 /dev/null 文件 */
    open( "/dev/null", O_RDONLY );
    open( "/dev/null", O_RDWR );
    open( "/dev/null", O_RDWR );
    return true;
}