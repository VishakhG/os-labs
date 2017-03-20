#ifndef SCHEDULER_H
#define SCHEDULER_H

/*
The fundemental unit: a process
 */
class Process{
private:
  int start_time;
  int arrival_time;
  int total_time;
  int cpu_burst;
  int io_burst;
  int priority;

  int time_running;
  int time_blocked;


public:
  //Setters
  void set_arrival_time(int AT);
  void set_total_time(int TT);
  void set_cpu_burst(int CB);
  void set_io_burst(int IO);

  void add_time_running(int w);
  void add_time_blocked(int b);

  //Getters
  int get_arrival_time();
  int get_total_time();
  int get_cpu_burst();
  int get_io_burst();
  
  int get_time_blocked();
  int get_time_running();
  
  void set_incrementally(int val);


  
};


/*
A base class, Scheduler which all specific schedulers 
extend.
*/
class Scheduler{
protected:
  //Every schedule has a place to keep processes
  std::vector<Process *> runqueue;
  int quantum;

public:
  //A specific process is free to define these as they wish
 
  virtual void add_process(Process * proc)=0;
  virtual Process* get_process()=0;
  bool not_empty();
  int get_quantum();
};

class FIFO: public Scheduler{
public:
  //Will override these virtual functions
  void add_process(Process * proc);
  Process * get_process();
  int get_quantum();
};

#endif

