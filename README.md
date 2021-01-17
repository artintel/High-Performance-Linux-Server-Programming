# 第六章、高级I/O函数

 ## pipe 函数

 用于创建一个管道，以实现==进程==间通信。定义如下

 ```c
 #include <unistd.h
 int pipe( int fd[2] );
 ```

 `pipe()` 函数的参数是一个包含两个 `int` 型整数的数组指针。该函数成功时返回 0 ，并将对弈打开的文件描述符值填入其参数所指向的数组。失败返回 -1 并设置 errno。

 `pipe()` 函数创建的两个文件描述符 `fd[0]` 和 `fd[1]` 分别构成管道的两端，往 `fd[1]` 写入的数据可以从 `fd[0]` 。并且， `fd[0]` 只能用于从管道读出数据， `fd[1]` 则只能用于往管道写入数据，而不能反过来使用。如果要实现双向的数据传输，就应该使用两个管道。

 `socket` 的基础 API 中有一个 `socketpair` 函数。能够方便地创建双向管道。其定义如下：

 ```c
 #include <sys/types.h
 #include <sys/socket.h
 int socketpair( int domain, int type, int protocol, int fd[2] );
 ```

 `socketpair` 钱三个参数地含义与 `socket` 系统调用地三个参数完全相同，但 `domain` 只能使用 `UNIX` 本地域协议族 `AF_UNIX`, 因为我们仅能再本地使用这个双向管道。最后一个参数和 `pipe` 系统调用的参数一样，只不过 `socketpair` 创建的这对文件描述符都是既可读又可写的。`socketpair` 成功时返回 0 ， 失败时返回 -1 并这是 errno。

readv 函数和 writev 函数

 `readv` 函数将数据从文件描述符读到分散的内存块中，即分散度；`writev` 函数则将多块分散的内存数据一并写入文件描述符中，即集中写。定义如下：

 ```c
 #include <sys/uio.h
 ssize_t readv( int fd, const struct iovec* vector, int count );
 ssize_t writev( int fd, const struct iovec* vector, int count );
 ```

 ## sendfile 函数

 `sendfile` 函数在两个文件描述符之间直接传递数据 ( 完全在内核中操作 )，从而避免了内核缓冲区和用户缓冲区之间的数据拷贝，效率很高，这被称为零拷贝。 `sendfile` 函数定义如下：

 ```Cpp
 #include <sys/sendfile.h
 ssize_t sendfile( int out_fd, int in_fd, off_t* offset, size_t count );
 ```

 - `in_fd` 是待读出内容的文件操作符
 - `out_fd` 是待写入内容的文件描述符
 - `offset` 指定从读入文件流的哪个位置开始读，如果为空，则使用读入文件流的默认起始位置。
 - `count` 指定文件描述符 `in_fd` 和 `out_fd` 之间传输的字节数

 `sendfile` 成功时返回传输的字节数，失败返回 -1 并设置 errno。

 注意：`in_fd` 必须是一个支持类似 `mmap` 函数的文件描述符，即它必须指向真实的文件，不能是 `socket` 和管道；而 `out_fd` 则必须是一个` socket`。

 **sendfile_function.c**

 使用方式：

 ```bash
 gcc -o sendfile_function sendfile_function.c
 ./sendfile_function (host_ip number) (port number) (file_name, 测试建议用.txt)
 ```

   在客户端机 `telnet (host_ip number) (port number)`并回车得到以下信息

   Trying 192.168.136.43...
   Connected to 192.168.136.43.
   Escape character is '^]'.
   testfile
   file tranfer (这两行是我的 .txt 文档里的内容)
   Connection closed by foreign host.

 ## mmap 函数和 munmap 函数

 `mmap` 函数用于申请一段内存空间。可以将这段内存作为进程间通信的==共享内存==，也可以将文件直接映射到其中。`munmap` 函数则释放由 `mmap` 创建的这段内存空间。定义如下：

 ```c
 #include <sys/mman.h
 void* map( void* start, size_t length, int prot, int flags, int fd, off_t offset );
 int munmap( void* start, size_t length )
 ```

 - `start` 允许用户使用某个特定的地址作为这段内存的起始地址。如果它被设置成 `NULL`, 则系统自动分配一个地址。
 - `length` 指定内存段的长度
 - `prot` 用来设置内存段的访问权限
   - `PROT_READ`	内存段可读
   - `PROT_WRITE`  内存段可写
   - `PROT_EXEC`    内存段可执行
   - `PROT_NONE`    内存段不能被访问
 - `flag` 控制内存段内容被修改后程序的行为
 - `fd` 参数是被映射文件对应的文件描述符。它一般通过 `open` 系统调用获得
 - `offset` 设置从文件何处开始映射( 对于不需要读入整个文件的情况 )

 `mmap` 函数成功时返回指向目标内存区域的指针， `munmap` 成功时返回 0，失败返回 -1 并设置 errno

 ## splice 函数

 `splice` 函数用于在两个文件描述符之间==移动数据==，也是零拷贝操作。定义如下

 ```C
 #include <fcntl.h
 ssize_t splice( int fd_in, loff_t* off_in, int fd_out, loff_t* off_out, size_t len, unsigned int flags );
 ```

 - `fd_in` 是代输入数据的文件描述符，如果 `fd_in` 是一个普通管道文件描述符，那么 `off_in` 必须被设置为 `NULL`。如果不是一个管道文件描述符，( 如 sockdet ), 那么 `off_in` 表示从输入数据流的何处开始读取数据。此时， 若 `off_in` 被设置为 `NULL`, 则表示从输入流的当前偏移位置读入；若不为 `NULL`, 则它将指出具体的便宜位置。
 - `fd_out/off_out` 含义与 `fd_in/off_in` 相同，不过用于输出数据流
 - `len` 指定移动数据的长度
 - `flag` 控制数据如何移动。

 使用 `splice` 函数， `fd_in`, `fd_out` 必须至少有一个是管道文件描述符。调用成功返回移动字节的数量，返回 0 ，表示没有数据需要移动。

 零拷贝回射服务实现：

 **splice.c**

 使用方式：

 ```bash
 gcc -o splice splice.c
 ./splice (host_ip number) (port number)
 ```

 我们通过 `splice` 函数将客户端的内容读入到 `pipefd[1]` 中，然后再使用 `splice` 函数从 `pipefd[0]` 中读出改内容到客户端，从而实现了简单高效的回射服务。整个过程未执行 `recv / send ` 操作，因此也未设计用户空间和内核空间之间的数据拷贝。

 ## tee 函数

 `tee` 函数在两个管道文件描述符之间==复制数据==，也是零拷贝操作。它不消耗数据，因此源文件描述符上的数据仍然可以用于后续的读操作。`tee` 函数原型如下：

 ```c
 #include <fcntl.h
 ssize_t tee( int fd_in, int fd_out, size_t len, unsigned int flags )
 ```

 含义与 `splice` 相同 ( 但 `fd_in` 和 `fd_out` 必须都是管道文件描述符) 。

 **tee.cpp** 代码实现了 `Linux` 下 `tee` 程序(同时输出数据到终端和文件的程序)的基本功能

 使用方式如下：

 ```bash
 g++ -o tee tee.cpp
 ./tee test.txt
 功能实现
 ```

## fcntl 函数

`fcntl` (file control), 提供了对文件描述符的各种控制操作。另一个常见的控制文件描述符和行为的系统调用是 `ioctl`, 而且 `ioctl` 比 `fcntl` 能够执行更多的控制。定义如下：

```C
#include <fcntl.h>
int fcntl( int fd, int cmd, ... );
```

- `fd` 被操作的文件描述符
- `cmd` 指定执行何种类型的操作。根据操作类型的不同，该函数可能还需要第三个可选参数 `arg`

在网络变成中， `fcntl` 函数通常用来将一个文件描述符设置为非阻塞的。

```C
int setnonblocking( int fd ){
    int old_option = fcntl( fd, F_GETFL );		/* 获取文件描述符旧的状态标志 */
    int new_option = old_option | O_NONBLOCK;	/* 设置为非阻塞状态 */
    fcntl( fd, F_SETFL, new_option );
    return old_option;						  /* 返回文件描述符旧的状态标志，以便日后回复该状态标志 */
}
```

## 第7章 Linux 服务器程序规范

本章综述了以下关于 linux 的进程，用户，组之类的概念。

下面的代码 **daemonize()** 表示了如何在代码中让一个进程以守护进程的方式运行。

```c
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
```