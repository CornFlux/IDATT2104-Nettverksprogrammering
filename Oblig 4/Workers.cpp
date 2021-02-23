#include "Workers.h"


using namespace std;

Workers::Workers(int numberOfWorkers) : running(false), numberOfWorkers(numberOfWorkers)
{
    this->workers = vector<thread>(this->numberOfWorkers);
}

Workers::~Workers()
{
    if (running)
    {
        this->join();
    }
}

void Workers::start()
{
    this->running = true; //Må settes til 'true' da om det ikke er noe arbeid enda så vil alle gå ut av loopen sin
    {
        lock_guard<mutex> lock(this->work_lock);
        for (auto &t : workers)
        {
            this->run(t);
        }
    }
    this->cv.notify_all();
}

void Workers::post(void f())
{
    {
        lock_guard<mutex> lock(this->work_lock);
        this->workload.emplace_back(f);
    }
    this->cv.notify_one();
}
/*
template<typename... Args>
        void push_task(Args&&... args)
        {
            {
                lock_guard<mutex> lock(mutex_lock);
                queue.push_back(bind(forward<Args>(args)...));
            }

            CV.notify_all();
        }
*/

void do_work_in_given_time(void f(), int milliseconds)
{
    this_thread::sleep_for(chrono::milliseconds(milliseconds)); //Venter et gitt antall millisekunder
    f(); //Kjører function
}

void Workers::post_timeout(void f(), int milliseconds)
{
    function<void()> func = bind(do_work_in_given_time, f, milliseconds); //Lager en function som venter et gitt antall millisekunder og deretter kjører en annen function
    {
        lock_guard<mutex> lock(this->work_lock);
        this->workload.emplace_back(func); //Legger denne function i workload
    }
}

void Workers::join()
{
    this->stop();
    for (auto &t : workers)
    {
        t.join();
    }
}

void Workers::stop()
{
    this->running = false;
    this->cv.notify_all();
}

void Workers::run(thread& t)
{
    t = thread([&workload = workload, &work_lock = work_lock, &running = running, &cv = cv]
    {
        while (true)
        {
            function<void()> func;
            {
                unique_lock<mutex> lock(work_lock);
                cv.wait(lock, [&]
                {
                    return !workload.empty() + !running; //False: skal fortsette å vente
                });

                if (!running)
                {
                    if (workload.empty())
                    {  
                        return;
                    }
                    for (auto &f : workload) //Om vi skal avslutte så vil den første threaden som kommer hit gå inn å fullføre resten
                    {
                        f();
                    }
                    workload.clear();
                    return; //Går ut av evig-while
                }
                func = workload.front();
                workload.pop_front();
            }
            func();
        }
    });
}