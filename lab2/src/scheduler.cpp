#include <iostream>
#include <vector>
#include <iostream>
#include <cstdio>
#include <stdio.h>

/*
This is the process class.
The fundemental unit of the scheduler.
*/


class Process{
private:
  int start_time = -1;
  int arrival_time = -1;
  int total_time = -1;
  int cpu_burst = -1;
  int io_burst = -1;
  
public:
 
  // Setters

  void set_arrival_time(int AT){
    arrival_time = AT;
  }

  void set_total_time(int TT){
    total_time = TT;
  }

  void set_cpu_burst(int CB){
    cpu_burst = CB;
  }

  void set_io_burst(int IO){
    io_burst = IO;
  }
  
  // Getters

  int get_arrival_time(){
    return arrival_time;
  }

  int get_total_time(){
    return total_time;
  }

  int get_cpu_burst(){
    return cpu_burst;
  }

  int get_io_burst(){
    return io_burst;
  }
  // Set one after another depending on what hasn't been set
  void set_incrementally(int val){
    if(start_time == false){
      start_time = val;
    }
    else if (arrival_time == false){
      arrival_time = val;
    }
    else if (total_time == false){
      total_time = val;
    }

    else if (cpu_burst == false){
      cpu_burst = val;
    }

    else if (io_burst == false){
      io_burst = val;
    }
  }

};


/*
The scheduler base class.
Every specific scheduler extends this base class.
*/

class Scheduler{
protected:

  //Every schedule has a place to keep processes
  std::vector<Process *> runqueue;

  //Make it large by default so it works with non-preemptive
  int quantum = 10000000;

public:

  //A specific process is free to define these as they wish

  virtual void add_process(Process* proc);
  virtual Process* get_process();

  int get_quantum(){
    return quantum;
  }
  
};


/*
The FIFO scheduler
*/

class FIFO: public Scheduler{
private:
  std::vector<Process *> runqueue;
  

public:
  void add_process(Process * proc){
    //Insert at end of queue
    runqueue.insert(proc, 0);
  }

  Process * get_process(){
    //Just pop from the back
    return runqueue.pop();
  }
  
};

