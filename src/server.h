
#ifndef SERVER_H
#define SERVER_H

#include <vector>
#include "utils.h"

class TcpServer;
class IConnect;

class HttpSender {
private:
    IConnect *conn;
public:
    HttpSender() {conn=NULL;};
    void set_connect(IConnect *n_conn) {this->conn = n_conn;};
    HttpSender *status(const char *status);
    HttpSender *header(const char *key, ISlice &value);
    void done(ISlice &body);
    void done();
};

class IConnect {
private:
    char _socket_status;  //  1 - read, 2 - write, -1 - closed
    int _link;
public:
    HttpSender send;
    Buffer send_buffer;
    bool keep_alive;
    int fd;
    TcpServer *server;
    IConnect(int fd, TcpServer *server) : fd(fd) {
        _link = 0;
        _socket_status=1;
        this->server=server;
        send.set_connect(this);
    };
    virtual ~IConnect() {
        fd = 0;
        send.set_connect(NULL);
    };
    virtual void on_recv(char *buf, int size) {};
    virtual void on_send();
    virtual void on_error() {};
    
    void write_mode(bool active);
    void read_mode(bool active);
    void close() {_socket_status = -1;};
    inline bool is_closed() {return _socket_status == -1;};
    
    int raw_send(const void *buf, uint size);

    int get_link() { return _link; }
    void link() { _link++; };
    void unlink();
};

class TcpServer {
private:
    int _port;
    Slice _host;
    int serverfd;
    int epollfd;
    IConnect** connections; // fixme

    void listen_socket();
    void init_epoll();
    void loop();
    
    void _close(int fd);
public:
    int log;
    std::vector<IConnect*> dead_connections;

    TcpServer() {
        log = 0;
    };

    void start(Slice host, int port);
    void unblock_socket(int fd);
    void set_poll_mode(int fd, int status);  // 1 - read, 2- write, -1 closed
    
    virtual IConnect* on_connect(int fd, uint32_t ip) {return new IConnect(fd, this);};
    virtual void on_disconnect(IConnect *conn) {};
};

#endif /* SERVER_H */