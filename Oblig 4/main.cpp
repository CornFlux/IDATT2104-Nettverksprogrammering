//g++ main.cpp -o main -pthreads -std=c++1y && ./main
#include <iostream>
#include "Workers.cpp"


using namespace std;


int main()
{
    Workers worker_threads(4), event_loop(1);

    worker_threads.start();
    event_loop.start();

    worker_threads.post([]
    {
        //Task A
        cout << "Hello from task A" << endl;
    });

    worker_threads.post([]
    {
        //Task B
        cout << "Hello from task B" << endl;
    });

    event_loop.post([]
    {
        //Task C
        cout << "Hello from task C" << endl;
    });

    event_loop.post([]
    {
        //Task D
        cout << "Hello from task D" << endl;
    });

    worker_threads.post_timeout([]
    {
        cout << "5 sek" << endl;
    }, 5000);

    event_loop.post_timeout([]
    {
        cout << "2 sek" << endl;
    }, 2000);

    cout << "Alt arbeid er lagt inn" << endl;

    worker_threads.join();
    event_loop.join();
    

    cout << "Da er vi ferdig!" << endl;

    return 0;
}