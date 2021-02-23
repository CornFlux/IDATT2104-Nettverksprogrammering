#include <iostream>
#include <vector>
#include <thread>
#include <cmath>
#include <list>
#include <mutex>
#include <condition_variable>

//Kommando for å kompilere og kjøre programmet
//g++ oblig3.cpp -o oblig3 -pthreads -std=c++1y && ./oblig3

using namespace std;

bool primtall(int tall);

class PrimtallFinder
{
    public:
        int number_of_threads, lower_bound, upper_bound;
        list<int> primes;
        list<int> workload;
        vector<thread> threads;
        mutex mutex_write, mutex_read;
        condition_variable CV;

    public:
        PrimtallFinder()
        {
            cout << "Hvor mange tråder vil du bruke?" << endl;
            cin >> number_of_threads;
            cout << "Nedre grense:" << endl;
            cin >> lower_bound;
            cout << "Øvre grense:" << endl;
            cin >> upper_bound;
            bool antall_tall = upper_bound > lower_bound ? false : true;
            if (antall_tall || lower_bound < 0 || upper_bound < 0 || number_of_threads < 1)
            {
                cout << "Øvre grense må være større en nedre grense, og begge må være større enn null. Antall tråder må være større enn 0" << endl;
                exit(1);
            }
            start();
        }

    private:

        void build_workload()
        {
            if (lower_bound == 0 || lower_bound == 1) //Om nedre grense er 0 eller 1 kan sette den til to
            {
                lower_bound = 2;
            }
            if (lower_bound == 2 && lower_bound <= upper_bound)
            {                
                primes.emplace_back(lower_bound++); //Legger inn to og setter nedre grense til 3
            }
            else if (lower_bound % 2 == 0)
            {
                lower_bound++; //Om nedre grense var et partall (og ikke 2) inkrementerer vi den
            }

            for (int i = lower_bound; i <= upper_bound; i += 2)
            {
                workload.emplace_back(i); //Legger alle oddetall i en felles liste
            }
        }

        void start()
        {
            build_workload(); //Bygger en liste med alle oddetallene i intervallet for å optimaleisere mer, da låsen på workload (mutex_read) vil være mye mindre brukt
            
            for (int i = 0; i < number_of_threads; i++)
            {
                threads.emplace_back([&workload = workload, &mutex_read = mutex_read, &mutex_write = mutex_write, &primes = primes]
                {
                    int number;
                    while (1) //Kjøerer helt til thread'en manuelt går ut, når arbeidet (workload) er tomt
                    {   
                        {
                            lock_guard<mutex> read_lock(mutex_read);
                            if (workload.empty()) //Sjekker at det er noe å gjøre, hvis ikke kan vi risikere å få en feil ved '.front()' kallet under
                            {
                                return; //Om det ikke er mer arbeid så går vi ut av loop'en og thread'en er ferdig
                            }
                            number = workload.front(); //Henter tall
                            workload.pop_front(); //Fjerner tallet vi hentet
                        }
                        if (primtall(number)) //Sjekker om tallet er et primtall
                        {
                            lock_guard<mutex> write_lock(mutex_write);
                            primes.emplace_back(number); //Om det var et primtall, legger vi det i listen over primtall
                        }             
                    }
                });
            }
            for (auto &t : threads) //Venter på at alle thread'ene skal gjøre seg ferdig
            {
                t.join();
            }

            //Sorterer listen
            primes.sort();

            //Skriver ut listen til slutt        
            int count = 1;
            for (auto it = primes.begin(); it != primes.end(); it++) 
            {
                cout << "primtall nr. " << count++ << ": " << *it << endl;
            }
        }
};


int main()
{
    cout << "Start" << endl;
    PrimtallFinder();
    cout << "Ferdig" << endl;
    return 0;
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