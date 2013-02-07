/** Copyright (C) 2013 wilburlang@gmail.com
 */
#include "common/config.h"
#include "common/gtest/gtest.h"
#include "common/net/tcp_address.h"
#include "common/net/poller.h"
#include "common/net/reactor.h"
#include "common/net/tcp_connector.h"
#include "common/net/tcp_listener.h"
#include "common/net/tcp_connection.h"
#include "common/net/helper.h"
#include <iomanip>

TEST(NET, tcp_address)
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

TEST(NET, tcp_address_mask)
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


class PollerTimer : public cxx::net::event_sink {
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
    l.listen(10);

    cxx::sys::threadcontrol::sleep(5000);
}

int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}

