#include <iostream>
#include <vector>
#include <thread>
#include <cmath>
#include <list>
#include <mutex>
#include <condition_variable>
#include <chrono>

//g++ main.cpp -o main -pthreads -std=c++1y && ./main

using namespace std;

bool primtall(int tall);
void do_work(int tall, list<int>* primes);

//Sterkt inspirert av (forfatter "Jts") https://stackoverflow.com/questions/35827459/assigning-a-new-task-to-a-thread-after-the-thread-completes-in-c
static mutex list_write_mutex;

class Worker
{
    private:
        condition_variable CV;
        list<function<void()>> queue;
        mutex mutex_lock;
        thread _thread;
        bool running;
    
    public:
        Worker(bool start) : running(start)
        {
            if (start)
            {
                private_start();
            }
        }

        Worker() : running(false)
        {

        }

        ~Worker()
        {
            stop();
        }

        template<typename... Args>
        void push_task(Args&&... args)
        {
            {
                lock_guard<mutex> lock(mutex_lock);
                queue.push_back(bind(forward<Args>(args)...));
            }

            CV.notify_all();
        }

        void start()
        {
            {
                lock_guard<mutex> lock(mutex_lock);
                if (running == true)
                {
                    return;
                }
                running = true;
            }

            private_start();
        }

        void stop()
        {
            {
                lock_guard<mutex> lock(mutex_lock);
                if (running == false)
                {
                    return;
                }
                running = false;
            }

            CV.notify_all();
            _thread.join();
        }

    private:
        void private_start()
        {
            _thread = thread([this]
            {
                while(true)
                {
                    decltype(queue) local_queue;
                    {
                        unique_lock<mutex> lock(mutex_lock);
                        CV.wait(lock, [&]
                        {
                            return !queue.empty() + !running; //Returnerer om vi skal fortsette å vente (false: vent mer, true: ferdig med å vente)
                                                              //Vi venter ikke om køen ikke er tom og/eller running-veriabelen er satt til false
                                                              //Det vil si at vi kun venter om vi skal kjøre (running = true) og vi ikke har noe arbeid (venter på arbeid, køen er tom)
                        }); //Vi holder ikke lock'en mens vi venter, vi vil gi den opp og ta den tilbake når CV får et signal

                        if (!running) //Kjører om Worker er satt til running = false
                        {
                            //Hvis vi ikke skal kjøre (running = false), må vi rydde opp, fullføre listen og tømme den
                            for (auto &func : queue)
                            {
                                func();
                            }

                            queue.clear();
                            return; //Går ut av den evige while løkken og tråden er ferdig
                        }

                        swap(queue, local_queue); //Bytter køene, tømmer Worker køen og gjør alt som lå der her
                                                  //Om de ikke blir byttet vil vi ikke gjøre noe arbeid før Worker blir stoppet, altså running = false
                    }

                    for (auto &func : local_queue) //Gjør alt arbeidet som lå i Worker køen
                    {
                        func();
                    }
                }
            });
        }

};

class Main
{
    public:
        int ant_threads, nedre_grense, ovre_grense, antall_tall;
        list<int> tall_funnet;

    Main()
    {
        cout << "Hvor mange tråder vil du bruke?" << endl;
        cin >> ant_threads;
        cout << "Nedre grense:" << endl;
        cin >> nedre_grense;
        cout << "Øvre grense:" << endl;
        cin >> ovre_grense;
        antall_tall = ovre_grense > nedre_grense ? ovre_grense - nedre_grense : -1;
        if (antall_tall == -1 || nedre_grense < 0 || ovre_grense < 0)
        {
            cout << "Øvre grense må være større en nedre grense, og begge må være større enn null" << endl;
            exit(1);
        }
        
        //Sjekker om nedre grense er et partall, og inkrementerer deretter for optimaliseringens skyld
        if (nedre_grense % 2 == 0)
        {
            if (primtall(nedre_grense))
            {
                tall_funnet.emplace_back(nedre_grense++);
            }
            else if (nedre_grense == 0)
            {
                nedre_grense += 3;
                tall_funnet.emplace_back(2);
            }
            else
            {
                nedre_grense++;
            }
        }

        Worker workers[ant_threads];


        cout << "Tid" << endl;
        chrono::high_resolution_clock::time_point timeStart = chrono::high_resolution_clock::now();
        if (ant_threads == 1) //Om vi kun har en thread så starter vi den med en gang,
                              //da main thread'en vil klare å 'mate' queue'en dens med nok tall fort nok
        {
            for (auto &worker : workers)
            {
                worker.start(); //Starter alle worker'ne
            }
        }
        //Her begynner den generelle sjekkingen
        int index = 0;
        for (int i = nedre_grense; i < ovre_grense; i += 2)
        {
            workers[index++ % ant_threads].push_task(do_work, i, &tall_funnet);
        }
        
        if (ant_threads > 1) //Om vi har flere enn én thread lar vi main thread'en fordele oppgavene uforstyrret,
                             //da den vil slite med å 'mate' flere threads om gangen, som gjør den til en bottleneck,
                             //da vi får mye unødvendig arbeid til kø venting og overhead som kommer av at thread'ene ikke har noe å gjøre
        {
            for (auto &worker : workers)
            {
                worker.start(); //Starter alle worker'ne
            }
        }

        for (auto &worker : workers)
        {
            worker.stop();
        }

        chrono::high_resolution_clock::time_point timeEnd = chrono::high_resolution_clock::now();
        cout << "Tid" << endl;
        
        chrono::duration<double, milli> time_span = timeEnd - timeStart;
        
        cout << "Total time: " << time_span.count() / 1000 << "s" << endl;
        

        //Sorterer listen
        tall_funnet.sort();

        //Skriver ut listen til slutt        
        int count = 1;
        for (auto it = tall_funnet.begin(); it != tall_funnet.end(); it++) 
        {
            cout << "primtall nr. " << count++ << ": " << *it << endl;
        }
        
    }
};


int main()
{
    Main();
    
    cout << "finish" << endl;
    return 0;
}

void do_work(int tall, list<int>* primes)
{
    if (primtall(tall))
    {
        lock_guard<mutex> lock(list_write_mutex);
        primes->emplace_back(tall);
    }
}

bool primtall(int tall)
{
    if (tall % 2 == 0)
    {  
        if (tall == 2)
        {
            return true;
        }
        return false;
    }
    int rot = pow(tall, 0.5) + 1;
    for (int i = 3; i <= rot; i += 2)
    {
        if (tall % i == 0)
        {
            return false;
        }
    }
    return true;
}