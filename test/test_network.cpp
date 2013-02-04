/** Copyright (C) 2013 wilburlang@gmail.com
 */
#include "common/config.h"
#include "common/gtest/gtest.h"
#include "common/net/tcp_address.h"
#include "common/net/poller.h"

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
        assert(id == 1);
        p_->stop();
    }
private:
    cxx::net::poller* p_;
};

TEST(NET, poller_create)
{
    cxx::net::poller* p = cxx::net::poller::create();
    PollerTimer timer(p);
    p->add_timer(1, 1000, &timer);
    p->start();
    p->stop();
    delete p;
}

int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}

