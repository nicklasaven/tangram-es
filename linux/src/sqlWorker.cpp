#include "sqlWorker.h"



SqlWorker::SqlWorker() {
  
}

SqlWorker::~SqlWorker() {
    // wait for thread to finish
    if (m_future.valid()) { m_future.get(); }

    
}

void SqlWorker::perform(std::unique_ptr<SqlTask> _task) {

    m_task = std::move(_task);

    m_future = std::async(std::launch::async, [&]() {

     
        logMsg("Fetching SQL: %s", "\n");

    

        m_task->callback(std::move(m_task->content));
        m_task.reset();

        return true;
    });
}

void SqlWorker::reset() {
    m_task.reset();
}

void SqlWorker::join() {
    if (m_future.valid()) {
        m_future.get();
    }
}

bool SqlWorker::hasTask(const std::string& _url) {
    return (m_task && m_task->url == _url);
}
