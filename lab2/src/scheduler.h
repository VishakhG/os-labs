#ifndef SCHEDULER_H
#define SCHEDULER_H

/*
****************************
The process class.

This class contains the various
data members of a process and some functions
to set and get those members
***************************
 */

class Process{
private:
  int arrival_time = -1;
  int total_time = -1;
  int cpu_burst = -1;
  int io_burst = -1;
  int priority = -1;
  int pid = 0;
  int finishing_time = 0;
  int cpu_waiting = 0;
  
  int time_running = 0;
  int time_blocked = 0;

  int time_in_previous_state = 0;


public:
  int last_event_time = 0;
  int dynamic_priority = 0;
  int cb_remaining = 0;
  int dynamic_reset = false;

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
  std::vector<Process *> expiredqueue;
  int quantum;

public:
  //A specific scheduler must define these functions
  virtual void add_process(Process * proc) = 0;
  virtual Process* get_process() = 0;
  bool empty();
  int get_quantum();
  void print_info();
};

class FIFO: public Scheduler{
public:
  //Will override these virtual functions
  void add_process(Process * proc);
  Process * get_process();

};

//LCFS, needs to implement add_process
class LCFS: public Scheduler{
  Process * get_process();
  void add_process(Process * proc);
  int get_quantum();
};

//Exactly the same as FIFO
class RR: public FIFO{
  //Same as fifo except preemption
};

//Needs to have an expired list handler and 
//Active list handler
class PRIO: public Scheduler{
private:
  public:
  void add_expired(Process * proc);
  void add_process(Process * proc);
  void add_active(Process * proc);
  bool not_empty();
  Process * get_process();
  bool goes_after(Process * p1, Process * p2);
};

//Define a different add_process 
class SJF : public Scheduler{
  public:
  void add_process(Process *proc);
  Process * get_process();
  bool goes_after(Process* p1, Process * p2);
};


#endif

