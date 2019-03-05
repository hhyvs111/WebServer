#include "Server.h"
#include "base/Logging.h"
#include "Util.h"
#include <functional>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

//主服务器
Server::Server(EventLoop *loop, int threadNum, int port)
:   loop_(loop),
    threadNum_(threadNum),
    eventLoopThreadPool_(new EventLoopThreadPool(loop_, threadNum)),
    started_(false),
    acceptChannel_(new Channel(loop_)),
    port_(port),
    listenFd_(socket_bind_listen(port_))
{
    //将acceptChannel的fd设置为监听fd，每一个Channel对应一个fd，但是不能销毁它
    acceptChannel_->setFd(listenFd_);
    handle_for_sigpipe();
    //设置监听套接字为非阻塞
    if (setSocketNonBlocking(listenFd_) < 0)
    {
        perror("set socket non block failed");
        abort();
    }
}

void Server::start()
{
    //server为Unique ptr 唯一的一个事件循环线程池
    eventLoopThreadPool_->start();
    //acceptChannel_->setEvents(EPOLLIN | EPOLLET | EPOLLONESHOT);
    //设置ET模式,并
    acceptChannel_->setEvents(EPOLLIN | EPOLLET);
    //epoll调用新的连接

    //将监听的Channel回调函数注册为server的handNewConn，别的Channel不会注册这个函数

    //当新连接请求建立时，可读事件触发，此时该事件对应的callback
    acceptChannel_->setReadHandler(bind(&Server::handNewConn, this));
    acceptChannel_->setConnHandler(bind(&Server::handThisConn, this));
    //将Channel放入主事件循环，这里的是调用了epoll_add
    //每个EventLoop都有一个epoll，那么主循环里的epoll就一个listenfd
    loop_->addToPoller(acceptChannel_, 0);
    started_ = true;
}

void Server::handNewConn()
{
    struct sockaddr_in client_addr;
    memset(&client_addr, 0, sizeof(struct sockaddr_in));
    socklen_t client_addr_len = sizeof(client_addr);
    int accept_fd = 0;
    //这里只循环一次吧？如果此时没连接的话

    //监听套接字是非阻塞的，那么这个循环还是会继续进行啊。那么可能是一段时间后就会停了，直到被唤醒才调用吧，应该是这样的。
    while((accept_fd = accept(listenFd_, (struct sockaddr*)&client_addr, &client_addr_len)) > 0)
    {
        //round bin循环获得loop，可能多个连接多个loop
        EventLoop *loop = eventLoopThreadPool_->getNextLoop();
        LOG << "New connection from " << inet_ntoa(client_addr.sin_addr) << ":" << ntohs(client_addr.sin_port);
        // cout << "new connection" << endl;
        // cout << inet_ntoa(client_addr.sin_addr) << endl;
        // cout << ntohs(client_addr.sin_port) << endl;
        /*
        // TCP的保活机制默认是关闭的
        int optval = 0;
        socklen_t len_optval = 4;
        getsockopt(accept_fd, SOL_SOCKET,  SO_KEEPALIVE, &optval, &len_optval);
        cout << "optval ==" << optval << endl;
        */
        // 限制服务器的最大并发连接数
        if (accept_fd >= MAXFDS)
        {
            close(accept_fd);
            continue;
        }
        // 设为非阻塞模式,因为epoll采用ET的话必须要设置为非阻塞，不然效率会很低。
        if (setSocketNonBlocking(accept_fd) < 0)
        {
            LOG << "Set non block failed!";
            //perror("Set non block failed!");
            return;
        }

        setSocketNodelay(accept_fd);
        //setSocketNoLinger(accept_fd);
        //这里主要是解析http，因为做的是一个WebServer。
        shared_ptr<HttpData> req_info(new HttpData(loop, accept_fd));
        req_info->getChannel()->setHolder(req_info);
        //这里是封装了一个function让queueInLoop去调用
        loop->queueInLoop(std::bind(&HttpData::newEvent, req_info));
    }
    //循环完后设置Events为IN，为什么连接完后还要设置一个事件？
    acceptChannel_->setEvents(EPOLLIN | EPOLLET);
}