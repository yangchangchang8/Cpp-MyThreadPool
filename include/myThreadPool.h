#include <iostream>
#include <functional>
#include <queue>
#include <thread>
#include <mutex>
#include <atomic>
#include <map>
#include <utility>
#include <condition_variable>

class myThreadPool {
  private:
    int minThread;
    int maxThread;

    std::atomic<int> m_idleThread;
    std::atomic<int> m_coutThread;

    std::atomic<bool> m_stop;

    std::queue<std::function<void()>> m_queueTask;
    std::mutex m_queueTask_mutex;
    std::condition_variable m_condition;

    std::thread* managerThread_ptr;
    std::mutex m_thread_mutex;
    std::map<std::thread::id, std::thread> m_workThread;

    void managerThread();
    void workerThread();

  public:
    void addTask(std::function<void()>);
    myThreadPool(int minThread); 
    ~myThreadPool();
};
