#include <functional>
#include <list>
#include <mutex>
#include <thread>
#include <vector>
#include <condition_variable>
#include <iostream>

using namespace std;

class Workers
{
    private:
        int numberOfWorkers;
        list<function<void()>> workload;
        mutex work_lock;
        condition_variable cv;
        vector<thread> workers;
        atomic<bool> running;

    public:
        Workers(int);
        ~Workers();
        void start();
        void post(void());
        void post_timeout(void(), int);
        void join();

    private:
        void stop();
        void run(thread&);
};
