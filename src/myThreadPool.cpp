#include "myThreadPool.h"


myThreadPool::myThreadPool(int minThread) 
                          : maxThread(8),
                            minThread(minThread), 
                            m_idleThread(minThread), 
                            m_coutThread(minThread), 
                            m_stop(false) {

    std::cout << "线程池中总线程数量为：" << m_coutThread << "，空闲线程数量为：" << m_idleThread << std::endl;

    managerThread_ptr = new std::thread(&myThreadPool::managerThread, this);

    for (int i = 0; i < m_idleThread; ++i) {
        std::thread workerThread_t(&myThreadPool::workerThread, this);
        m_workThread.insert(std::make_pair(workerThread_t.get_id(), std::move(workerThread_t)));
    }
}

myThreadPool::~myThreadPool() {
    m_stop = true;
    m_condition.notify_all();
    {
        std::lock_guard<std::mutex> lock(m_thread_mutex);
        for (auto& it : m_workThread) {
            if (it.second.joinable()) {
                std::cout << "线程：" << it.first << " 将要退出。。。" << std::endl;
                it.second.join();   // 主线程要一直等待子线程退出
            }
        }
    }

    if (managerThread_ptr && managerThread_ptr->joinable()) {
        managerThread_ptr->join();
        delete managerThread_ptr;
        managerThread_ptr = nullptr;
    }
}

void myThreadPool::addTask(std::function<void()> func) {
    {
        std::lock_guard<std::mutex> lock_queue_task(m_queueTask_mutex);
        m_queueTask.emplace(func);
    }
    // 在任务队列中添加一个任务后，需要唤醒一个工作线程
    m_condition.notify_one();
}

void myThreadPool::managerThread() {
    while (!m_stop) {
        size_t queue_size;
        size_t current_treads;
        size_t idle_threads;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        {
            std::unique_lock<std::mutex> lock(m_queueTask_mutex);
            queue_size = m_queueTask.size();
        }

        {
            std::unique_lock<std::mutex> lock(m_thread_mutex);
            current_treads = m_workThread.size();
        }

        idle_threads = m_idleThread.load();
        // 扩容条件：任务队列太多 && 线程数量没有达到最大值
        if (queue_size > idle_threads && current_treads < maxThread) {
            int add_cout = std::min((int)(queue_size - idle_threads), (int)(maxThread - current_treads));
            std::lock_guard<std::mutex> thread_lock(m_thread_mutex);
            for (int i = 0; i < add_cout; ++i) {
                std::thread wroker(&myThreadPool::workerThread, this);
                m_workThread.insert(std::make_pair(wroker.get_id(), std::move(wroker)));
                ++m_idleThread;
                ++m_coutThread;
            }
        }
    }
}

void myThreadPool::workerThread() {
    // 只要没有收到stop的信号，也就是线程池没有销毁，每个工作线程就一直循环执行任务队列中的任务
    while (!m_stop) {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(m_queueTask_mutex);
            // 任务队列为空时，暂时让出互斥锁lock，一直等待m_queueTask任务队列非空
            m_condition.wait(lock, [this] {
                return !m_queueTask.empty() || m_stop;
            });

            // 如果主线程发出线程池停止的信号，那么每个工作线程就必须判断m_stop和队列是否为空
            if (m_stop && m_queueTask.empty())
                return;

            task = std::move(m_queueTask.front());
            m_queueTask.pop();
        }
        --m_idleThread;
        // 执行任务并且捕获异常
        try {
            task();
        } catch (const std::exception& e) {
            std::cout << "出现异常：" << e.what() << std::endl;
        }
        ++m_idleThread;
    }
    
}
