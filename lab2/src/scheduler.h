#ifndef SCHEDULER_H
#define SCHEDULER_H

/*
The fundemental unit: a process
 */
class Process{
private:
  int arrival_time = -1;
  int total_time = -1;
  int cpu_burst = -1;
  int io_burst = -1;
  int priority = -1;
  int pid;
  int finishing_time;
  int cpu_waiting;
  
  int time_running = 0;
  int time_blocked = 0;

  int time_in_previous_state = 0;


public:
  int last_event_time;
  //Setters
  void set_arrival_time(int AT);
  void set_total_time(int TT);
  void set_cpu_burst(int CB);
  void set_io_burst(int IO);
  void set_pid(int p);
  void set_priority(int P);
  void set_finishing_time(int FT);
  void add_time_running(int r);
  void add_time_blocked(int b);
  void add_cpu_waiting(int CPU);
  void set_time_prev_state(int TPS);
  //Getters
  int get_arrival_time();
  int get_total_time();
  int get_cpu_burst();
  int get_io_burst();
  int get_priority();
  int get_finishing_time();
  int get_time_prev_state();
  int get_time_blocked();
  int get_time_running();
  int get_pid();
  int get_cpu_waiting();
  
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
  void print_info();
};

class FIFO: public Scheduler{
public:
  //Will override these virtual functions
  void add_process(Process * proc);
  Process * get_process();
  int get_quantum();
};

#endif

