#include "webServer.h"
#include <iostream>

WebServer::WebServer(std::atomic<bool>& visualsEnabled, int port)
    : visualsEnabled_(visualsEnabled), port_(port)
{
    // Serve the www/ directory for all static assets (index.html, CSS, JS, etc.)
    if (!svr_.set_mount_point("/", "./www")) {
        std::cerr << "WebServer: failed to mount ./www — directory may not exist." << std::endl;
    }

    // GET /api/status — returns current enabled state
    svr_.Get("/api/status", [this](const httplib::Request&, httplib::Response& res) {
        bool enabled = visualsEnabled_.load(std::memory_order_relaxed);
        res.set_content(enabled ? "{\"enabled\":true}" : "{\"enabled\":false}", "application/json");
    });

    // POST /api/on — enable LED visuals
    svr_.Post("/api/on", [this](const httplib::Request&, httplib::Response& res) {
        visualsEnabled_.store(true, std::memory_order_relaxed);
        res.set_content("{\"enabled\":true}", "application/json");
    });

    // POST /api/off — pause LED visuals (process keeps running)
    svr_.Post("/api/off", [this](const httplib::Request&, httplib::Response& res) {
        visualsEnabled_.store(false, std::memory_order_relaxed);
        res.set_content("{\"enabled\":false}", "application/json");
    });
}

WebServer::~WebServer() {
    stop();
}

void WebServer::start() {
    server_thread_ = std::thread([this]() {
        std::cout << "WebServer: listening on 0.0.0.0:" << port_ << std::endl;
        svr_.listen("0.0.0.0", port_);
        std::cout << "WebServer: stopped." << std::endl;
    });
}

void WebServer::stop() {
    svr_.stop();
    if (server_thread_.joinable()) {
        server_thread_.join();
    }
}
