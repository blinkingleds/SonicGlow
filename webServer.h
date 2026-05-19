#pragma once

#include <atomic>
#include <thread>
#include "httplib.h"

class WebServer {
public:
    WebServer(std::atomic<bool>& visualsEnabled, int port = 8080);
    ~WebServer();

    void start();
    void stop();

private:
    std::atomic<bool>& visualsEnabled_;
    int port_;
    httplib::Server svr_;
    std::thread server_thread_;
};
