#pragma once

#include <future>
#include <memory>
#include <vector>
#include <sstream>

#include "platform.h"

struct SqlTask {
    UrlCallback callback;
    const std::string url;
    std::vector<char> content;

    SqlTask(SqlTask&& _other) : 
        callback(std::move(_other.callback)),
        url(std::move(_other.url)),
        content(std::move(_other.content)) {
    }

    SqlTask(const std::string& _url, const UrlCallback& _callback) : 
        callback(_callback),
        url(_url) {
    }
};

class SqlWorker {
    public:
        void perform(std::unique_ptr<SqlTask> _task);
        void reset();
        bool isAvailable() { return !bool(m_task); }
        bool hasTask(const std::string& _url);
        void join();

        SqlWorker();
        ~SqlWorker();

    private:
        std::unique_ptr<SqlTask> m_task;
   //     std::stringstream m_stream;
   //     CURL* m_curlHandle = nullptr;

        std::future<bool> m_future;
};

