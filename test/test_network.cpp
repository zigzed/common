/** Copyright (C) 2013 wilburlang@gmail.com
 */
#include "common/config.h"
#include "common/gtest/gtest.h"
#include "common/net/tcp_address.h"
#include "common/net/poller.h"
#include "common/net/iothread.h"
#include "common/net/reactor.h"
#include "common/net/tcp_connector.h"
#include "common/net/tcp_listener.h"
#include "common/net/tcp_connection.h"
#include "common/net/helper.h"
#include <iomanip>

TEST(NET, DISABLED_tcp_address)
{
    cxx::net::tcp_address addr;
    addr.resolve("127.0.0.1:8000", true, true);
    std::cout << "address1: " << addr.string() << "\n";

    addr.resolve("*:8000", true, true);
    std::cout << "address2: " << addr.string() << "\n";

    addr.resolve("eth0:8000", true, true);
    std::cout << "address3: " << addr.string() << "\n";

    addr.resolve("eth0:8000", true, false);
    std::cout << "address4: " << addr.string() << "\n";

    addr.resolve("eth0:8000", false, false);
    std::cout << "address5: " << addr.string() << "\n";

    addr.resolve("wlan0:8000", true, true);
    std::cout << "address6: " << addr.string() << "\n";
}

TEST(NET, DISABLED_tcp_address_mask)
{
    cxx::net::tcp_address_mask addr;
    addr.resolve("127.0.0.1/24", true);
    std::cout << "address1: " << addr.string() << "\n";

    addr.resolve("*/16", true);
    std::cout << "address2: " << addr.string() << "\n";

    addr.resolve("eth0:8000", true);
    std::cout << "address3: " << addr.string() << "\n";

    addr.resolve("eth0:8000", false);
    std::cout << "address4: " << addr.string() << "\n";

    addr.resolve("eth0:8000", false);
    std::cout << "address5: " << addr.string() << "\n";

    addr.resolve("wlan0:8000", true);
    std::cout << "address6: " << addr.string() << "\n";
}


class PollerTimer : public cxx::net::poller_event {
public:
    PollerTimer(cxx::net::poller* p) : p_(p) {}
    void on_readable() {}
    void on_writable() {}
    void on_expire(int id) {
        cxx::datetime ts(cxx::datetime::now());
        cxx::datetime::calendar c(ts);
        std::cout << "timer: " << id << " expired: " << c.hour << ":" << c.min
                  << ":" << c.sec << "." << std::setw(3) << std::setfill('0') << c.msec << "\n";
        if(id == 2) {
            p_->def_timer(id);
            p_->stop();
        }
    }
private:
    cxx::net::poller* p_;
};

TEST(NET, poller_create)
{
    cxx::net::poller* p = cxx::net::poller::create();
    PollerTimer timer(p);
    p->add_timer(1, 1000, &timer);
    p->add_timer(2, 1500, &timer);
    p->start();

    cxx::sys::threadcontrol::sleep(2000);
    p->destroy();
}

class TestConnector : public cxx::net::connection_event {
public:
    TestConnector() : count_(0) {}
    void on_connected(const char *addr, cxx::net::fd_t fd) {
        printf("connected: %s\n", addr);
    }
    void on_accepted(const char *addr, cxx::net::fd_t fd) {
        printf("accepted: %s\n", addr);
    }
    void on_closed(const char *addr, cxx::net::fd_t fd) {
        printf("closed: %s\n", addr);
    }
    void on_connect_fail(const char *addr, int err) {
        ++count_;
        printf("%6d:%3d connecting failed: %s\n", count_, err, addr);
    }
    void on_disconnected(const char *addr, cxx::net::fd_t fd) {
        printf("disconnected: %s\n", addr);
    }
private:
    int count_;
};

TEST(tcp_connector, simple)
{
    cxx::net::reactor       r(1);
    r.start();

    cxx::net::tcp_address   a;
    a.resolve("192.168.9.100:8000", true, true);

    TestConnector           t;
    cxx::net::options       o;
    o.conivl = 10;
    cxx::net::tcp_connector c(&r, a, o);
    c.attach(&t);

    cxx::sys::threadcontrol::sleep(5000);
    r.stop();

}

class TestListener : public cxx::net::connection_event {
public:
    TestListener() : count_(0) {}
    void on_connected(const char *addr, cxx::net::fd_t fd) {
        printf("connected: %s\n", addr);
    }
    void on_accepted(const char *addr, cxx::net::fd_t fd) {
        ++count_;
        cxx::net::ip::closesocket(fd);
        printf("%6d: accepted: %s\n", count_, addr);
    }
    void on_closed(const char *addr, cxx::net::fd_t fd) {
        printf("closed: %s\n", addr);
    }
    void on_connect_fail(const char *addr, int err) {
        printf("connecting failed: %s\n", addr);
    }
    void on_disconnected(const char *addr, cxx::net::fd_t fd) {
        printf("disconnected: %s\n", addr);
    }
private:
    int count_;
};

TEST(tcp_listener, simple)
{
    cxx::net::reactor       r(1);
    r.start();

    cxx::net::tcp_address   a;
    a.resolve("*:8000", true, true);

    // 注意这里的初始化顺序，因为在 tcp_listener 的析构函数中会调用 connection_event::on_closed(),
    // 所以需要 TestListener 在 tcp_listener 之后析构
    TestListener            t;
    cxx::net::options       o;
    o.backlog = 10;
    cxx::net::tcp_listener  l(&r, a, o);
    l.attach(&t);
    l.listen();

    cxx::sys::threadcontrol::sleep(5000);
    r.stop();
}

class TestListener2 : public cxx::net::connection_event, public cxx::net::poller_event {
public:
    TestListener2(cxx::net::reactor* r)
        : reactor_(r), poller_(NULL), client_(NULL)
    {}
    ~TestListener2() {
        poller_->del_fd(handle_);
        client_->close();
        delete client_;
    }
    void on_connected(const char *addr, cxx::net::fd_t fd) {
        printf("connected2: %s\n", addr);
    }
    void on_accepted(const char *addr, cxx::net::fd_t fd) {
        ENFORCE(client_ == NULL);
        cxx::net::options opt;
        client_ = new cxx::net::tcp_connection(fd, addr, opt);
        poller_ = reactor_->choose()->get_poller();
        handle_ = poller_->add_fd(fd, this);
        poller_->add_fd(handle_, cxx::net::poller::readable());

        printf("accepted2: %s\n", addr);
    }
    void on_closed(const char *addr, cxx::net::fd_t fd) {
        printf("closed2: %s\n", addr);
    }
    void on_connect_fail(const char *addr, int err) {
        printf("connecting failed2: %s\n", addr);
    }
    void on_disconnected(const char *addr, cxx::net::fd_t fd) {
        printf("disconnected2: %s\n", addr);
    }

    void on_readable() {
        char buffer[2048];
        int rd = client_->recv(buffer, 2048);
        client_->send(buffer, rd);
    }
    void on_writable() {}
    void on_expire(int id) {}
private:
    cxx::net::reactor*          reactor_;
    cxx::net::poller*           poller_;
    cxx::net::tcp_connection*   client_;
    cxx::net::poller::handle_t  handle_;
};

class TestConnector2 : public cxx::net::connection_event, public cxx::net::poller_event  {
public:
    TestConnector2(cxx::net::reactor* r, int len)
        : reactor_(r), poller_(NULL), client_(NULL), length_(len)
    {}
    ~TestConnector2() {
        poller_->del_fd(handle_);
        client_->close();
        delete client_;
    }

    void on_connected(const char *addr, cxx::net::fd_t fd) {
        ENFORCE(client_ == NULL);
        cxx::net::options opt;
        client_ = new cxx::net::tcp_connection(fd, addr, opt);
        poller_ = reactor_->choose()->get_poller();
        handle_ = poller_->add_fd(fd, this);
        poller_->add_fd(handle_, cxx::net::poller::writable());
        poller_->add_fd(handle_, cxx::net::poller::readable());

        on_writable();
        printf("connected3: %s\n", addr);
    }
    void on_accepted(const char *addr, cxx::net::fd_t fd) {
        printf("accepted3: %s\n", addr);
    }
    void on_closed(const char *addr, cxx::net::fd_t fd) {
        printf("closed3: %s\n", addr);
    }
    void on_connect_fail(const char *addr, int err) {
        printf("%3d connecting failed3: %s\n", err, addr);
    }
    void on_disconnected(const char *addr, cxx::net::fd_t fd) {
        printf("disconnected3: %s\n", addr);
    }

    void on_readable() {
        char buffer[2048];
        int rd = client_->recv(buffer, 2048);
        rd = client_->send(buffer, rd);
    }
    void on_writable() {
        char buffer[2048];
        int rd = client_->send(buffer, length_);
    }
    void on_expire(int id) {}
private:
    cxx::net::reactor*          reactor_;
    cxx::net::poller*           poller_;
    cxx::net::tcp_connection*   client_;
    cxx::net::poller::handle_t  handle_;
    int                         length_;
};

void perf_test(int packet_size)
{
    printf("performance, size: %d\n", packet_size);
    {
        cxx::net::reactor   r(1);
        r.start();

        cxx::net::tcp_address   a1;
        a1.resolve("*:8000", true, true);

        TestListener2           t1(&r);
        cxx::net::options       o1;
        o1.backlog = 10;
        cxx::net::tcp_listener  l(&r, a1, o1);
        l.attach(&t1);
        l.listen();

        cxx::net::tcp_address   a2;
        a2.resolve("127.0.0.1:8000", true, true);

        TestConnector2          t2(&r, packet_size);
        cxx::net::options       o2;
        o2.conivl = 100;
        cxx::net::tcp_connector c(&r, a2, o2);
        c.attach(&t2);

        cxx::sys::threadcontrol::sleep(1000*5);
        r.stop();
        l.detach(&t1);
        c.detach(&t2);
        l.close();
        c.close();
    }
    printf("performance, size: %d done\n", packet_size);
}

TEST(tcp, performance)
{
    perf_test(8);
    perf_test(16);
    perf_test(32);
    perf_test(64);
    perf_test(128);
    perf_test(256);
    perf_test(512);
    perf_test(1024);
    perf_test(2048);
}

int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}

