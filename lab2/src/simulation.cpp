#include <iostream>
#include <vector>
#include <iostream>
#include <cstdio>
#include <stdio.h>
#include "scheduler.h"
#include "des.h"
#include <algorithm> 
#include <fstream>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>

/************************************************/
/*
Some utilites that might come 
in handy while running the simulation.
 */
/************************************************/

class SimUtils{
private:
  //Keep track of random number offset
  int RAND_COUNTER = 1;
  std::vector<int> rand_vals;
  int pid = 0;
  
public:
  //Load the random numbers
  void read_random_numbers(char const *rfile_path){
    FILE * file;
    char tokenseq [100];
    file = fopen(rfile_path, "r");

    if(file == NULL){
      perror("File does not exist");
    }

    else{

      //Whole file
      while(fgets(tokenseq, sizeof(tokenseq), file)){
	char *tok;
	tok = strtok (tokenseq," \t\n");
	
	//Single line
	while(tok != NULL){
	  rand_vals.push_back((int)atoi(tok));
	  tok = strtok (NULL," \t\n");
	}
      }
    }
  }

  void populate_queues(Scheduler * sched, DES * des, char const *input_path){
    FILE * file;
    char tokenseq [100];
    
    file = fopen(input_path, "r");
    
    if(file == NULL){
      perror("File does not exist");
    }
    
    else{
      //Whole file
      while(fgets(tokenseq, sizeof(tokenseq), file)){

	Process *proc_i = new Process;
	proc_i -> set_pid(pid++);
	char *tok;
	tok = strtok (tokenseq," \t\n");

	//Single line
	while(tok != NULL){
	  proc_i->set_incrementally((int)atoi(tok));
	  tok = strtok (NULL," \t\n");
	}
	
	Event *e_i = new Event;
	//Random priority 1-4
	proc_i -> set_priority(random_burst(4));
	//Last state
	e_i -> set_state(0);
	e_i -> set_event_process(proc_i);
	e_i -> set_transition(1);
	int arrived_i = proc_i -> get_arrival_time();
	e_i -> set_time_stamp(proc_i -> get_arrival_time());
	
	//Insert into DES layer
	des -> insert_event(e_i);
      }
    }
  }


  //Return a random burst 
  int random_burst(int burst){
    if (RAND_COUNTER > rand_vals.size())
      RAND_COUNTER = 1;
    return (1 + (rand_vals.at(RAND_COUNTER++) % burst));
  }


 //END of SimUtils class
};


/************************************************
The Simulation class. 
This class has an eventqueue, a scheduler
and simply gets events, adds events,
and requests a process from the scheduler.
***********************************************/

class Simulation{
private:
  //Put process here when its done so we can keep track of it
  std::vector<Process *> done_list;
  //TODO 
  Scheduler* fifo;
  DES des;
  SimUtils utils;
  int quantum; 
  const char* sched_type;
  int CURRENT_TIME = 1;
  Process * CURRENT_PROCESS = NULL;
  const char * process_path;
  const char * random_path;
  bool CALL_SCHEDULER = false;
  int TOTAL_CPU_TIME = 0;
  int TOTAL_IO_TIME = 0;
  int LAST_BLOCKED = 0;
  int VERBOSE = 0;

public:
  Simulation(const char * proc_path, const char *rand_path,  const char* st, int qtm, int v){
    process_path = proc_path;
    random_path = rand_path;
    sched_type = st;
    quantum = qtm;
    VERBOSE = v;
  }
  

  void setup_simulation(){
    if(*sched_type == 'F') {
      fifo = new FIFO;
    }
    else if (*sched_type == 'L'){
      fifo = new LCFS;
    }
    else if (*sched_type == 'R'){
      fifo = new FIFO;
    }
    else if (*sched_type == 'S'){
      fifo = new SJF;
    }
    else{
      fifo = new PRIO;
    }
   utils.read_random_numbers(random_path);
    utils.populate_queues(fifo, &des, process_path);
  }

  
  int get_next_event_time(){
    return (des.peek_event()) -> get_time_stamp();
  }
  

  void run_simulation(){
    while(des.not_empty()){

      Event* evt = des.get_event();

      CURRENT_TIME = evt -> get_time_stamp();
      Process* proc_e = evt -> get_process();
      
      int pid = proc_e -> get_pid();
      int last_trans_time = proc_e -> last_event_time;
      
      int TPI = CURRENT_TIME - last_trans_time;

      if(evt -> get_state() == 1){
        proc_e -> add_cpu_waiting(TPI);
      }

      proc_e -> last_event_time = CURRENT_TIME;

      switch(evt -> get_transition()){

      case 1:{
	print_transition(evt, 1);
	handle_trans_to_ready(evt);
      } break;

      case 2:{
	print_transition(evt, 2);
	
	handle_trans_to_run(evt);
 
      }break;

      case 3:{

	print_transition(evt, 3);
	if( (evt -> get_process()) -> get_time_running() < (evt->get_process()) -> get_total_time()){     
	  handle_trans_to_block(evt);
	}
	//Done running but we want to keep it around for printing data

	else{
	  CURRENT_PROCESS = NULL;
	  CALL_SCHEDULER = true;
	  (evt->get_process()) -> set_finishing_time(CURRENT_TIME);
	  if(VERBOSE == 1) 
	    printf("\nprocess %d DONE", evt->get_process()->get_pid());
	  done_list.push_back(evt -> get_process());
	}



      }break;

      case 4:{
	print_transition(evt, 4);
	handle_trans_to_preempt(evt);
      }break;
      }

      delete evt;


      if(CALL_SCHEDULER){

	if(des.not_empty() && (get_next_event_time() == CURRENT_TIME)){
	 continue;  
       }


	CALL_SCHEDULER = false;
       
	if (CURRENT_PROCESS == NULL){
	    if (! fifo -> empty()){
	      CURRENT_PROCESS = fifo -> get_process();
	    }

	    else{
	      continue;
	    }
       

	    Event * run_event = new Event;
	    run_event -> set_transition(2);
	    run_event -> set_state(1);
	    run_event -> set_time_stamp(CURRENT_TIME);
	    run_event -> set_event_process(CURRENT_PROCESS);
	    des.insert_event(run_event);
	  }
      }
    }
  }
  

  void print_transition(Event * evt, int transition){
    Process * proc_i = evt -> get_process();
    int pid = proc_i -> get_pid();
    int transition_from = evt -> get_state();
    std::string trans_to;
    std::string trans_from;
    switch(transition){
    case 1: 
      trans_to = "READY";
      break;
    case 2:
      trans_to = "RUN";
      break;
    case 3:
      trans_to = "BLOCKED";
      break;
    case 4:
      trans_to = "PREEMPT";
      break;
    }
    switch(transition_from){
    case 0: 
      trans_from = "CREATED";
      break;
    case 1:
      trans_from = "READY";
      break;
    case 2:
      trans_from = "RUNNING";
      break;
    case 3:
      trans_from = "BLOCKED";
      break;
    }
    
    std::string f_string = "\n %d %d "  +  trans_from + " -> " +   trans_to + " prio = %d" ;

    if(VERBOSE == 1){
      printf(f_string.c_str(), CURRENT_TIME,pid,  proc_i->dynamic_priority);
    }
}
  

  //Must be coming from blocked or preemption
  //Must add to the run queue
  void handle_trans_to_ready(Event* c_event){
    Process * proc_i = c_event -> get_process();

      if(c_event -> get_state() == 3)
	proc_i -> dynamic_priority = proc_i -> get_priority() - 1;

    fifo -> add_process(proc_i);
    CALL_SCHEDULER = true;
  }


  //Create event for either preemption or blocking
  void handle_trans_to_run(Event* c_event){
    Event * e_i = new Event;
    Process * proc_i = c_event -> get_process();
    int cpu_burst_i;
     
     if (proc_i -> cb_remaining == 0){
       int random_burst_i = utils.random_burst(proc_i -> get_cpu_burst());
       cpu_burst_i = std::min(random_burst_i, (proc_i -> get_total_time()) - (proc_i -> get_time_running()));
     }

     else{
       cpu_burst_i = std::min(proc_i -> cb_remaining, (proc_i -> get_total_time()) - (proc_i -> get_time_running()));
     }

     if(VERBOSE == 1){
       printf(" CB %d rem = %d", cpu_burst_i, proc_i -> get_total_time() - (proc_i -> get_time_running()));
     }
     int quantum_i = quantum;
     int trans;

     //Preempt
     if(cpu_burst_i > quantum_i){
       proc_i -> cb_remaining = std::max(cpu_burst_i - quantum_i, 0);
       proc_i -> add_time_running(quantum_i);
       e_i -> set_time_stamp(CURRENT_TIME + quantum_i);
       TOTAL_CPU_TIME += quantum_i;

       trans = 4;
       
       }
     
     else{
       proc_i -> cb_remaining = std::max(proc_i -> cb_remaining - cpu_burst_i, 0);
       proc_i -> add_time_running(cpu_burst_i);
       e_i -> set_time_stamp(CURRENT_TIME + cpu_burst_i);
       TOTAL_CPU_TIME += cpu_burst_i;
       trans = 3;
      }

     e_i -> set_transition(trans);
     e_i -> set_state(2);
     e_i -> set_event_process(proc_i);
     des.insert_event(e_i);
  }
  




  //Create an event for when its ready again
  void handle_trans_to_block(Event * c_event){
    CURRENT_PROCESS = NULL;
    Event* e_i = new Event;
    Process * proc_i = c_event -> get_process();
    int io_burst_i = utils.random_burst((proc_i -> get_io_burst()));
    //printf("io burst %d", io_burst_i);
    proc_i -> add_time_blocked(io_burst_i);
    // proc_i -> dynamic_priority = proc_i -> get_priority() -1;

    int e_time = CURRENT_TIME + io_burst_i;
    e_i -> set_time_stamp(e_time);
    e_i -> set_event_process(proc_i);
    e_i -> set_transition(1);
    e_i -> set_state(3);
    
    if(e_time > LAST_BLOCKED){

      if(LAST_BLOCKED > 0){
	TOTAL_IO_TIME += (e_time - std::max(CURRENT_TIME, LAST_BLOCKED));
      }

      else{
	TOTAL_IO_TIME += io_burst_i;
      }

      LAST_BLOCKED = e_time;
    }
    
    if(VERBOSE == 1)
      printf(" IB = %d", io_burst_i);

    des.insert_event(e_i);
    CALL_SCHEDULER = true;
  }
  
  //Add to run_queue;
  void handle_trans_to_preempt(Event * c_event){
    CURRENT_PROCESS = NULL;
    Process * proc_i = c_event -> get_process();
    if ((proc_i -> dynamic_priority) > 0){
      proc_i -> dynamic_priority -= 1;
    }

    else{
      proc_i -> dynamic_priority = proc_i -> get_priority() - 1;
      proc_i -> dynamic_reset = true;
    }

    
    fifo -> add_process(proc_i);
    CALL_SCHEDULER = true;
  }

  void sort_by_id(){
    std::vector<Process *> temp  (done_list.size());

    for(std::vector<Process *>::iterator it = done_list.begin(); it != done_list.end(); ++it) {
     int  position = (*it) -> get_pid();
     temp[position] = *it;
    }
    
    done_list = temp;
  }
  
  void print_simulation_details(){
    sort_by_id();
    int turnaround_count = 0;
    int cpu_waiting_count = 0;
    std::string sched_label; 
    bool print_quantum = false;
    if (*sched_type == 'F'){
      sched_label = "FCFS";
    }
    else if (*sched_type == 'L'){
      sched_label = "LCFS";
    }
    else if (*sched_type == 'R') {
      sched_label = "RR";
      print_quantum = true;
    }
    else if (*sched_type == 'S'){
      sched_label = "SJF";
    }
    else{
      sched_label = "PRIO";
      print_quantum = true;
    }
    if(print_quantum){
      printf("%s %i\n", sched_label.c_str(), quantum);
    }
    else{
      printf("%s\n", sched_label.c_str());
    }
    for(std::vector<Process *>::iterator it = done_list.begin(); it != done_list.end(); ++it) {
      int AT = (*it) -> get_arrival_time();
      int TC = (*it) -> get_total_time();
      int CB = (*it) -> get_cpu_burst();
      int IO = (*it) -> get_io_burst();
      int PRIO = (*it) -> get_priority();
      int FT = (*it) -> get_finishing_time();
      int TT = FT - AT;
      turnaround_count += TT;
      int IT = (*it) -> get_time_blocked();
      int CW = (*it) -> get_cpu_waiting();
      cpu_waiting_count += CW;
      int pid = (*it) -> get_pid();

      printf("%04d: %4d %4d %4d %4d %1d | %5d %5d %5d %5d\n",  pid, AT, TC,CB,IO,PRIO,FT,TT,IT,CW);
    }

    double CPU_UTIL = ((double)TOTAL_CPU_TIME / (double)CURRENT_TIME) * 100.0;
    double IO_UTIL = ((double)TOTAL_IO_TIME / (double)CURRENT_TIME) * 100.0;

    int n_proc = done_list.size();

    double average_turnaround = (double)turnaround_count / (double)n_proc;
    double average_waiting = (double)cpu_waiting_count / (double) n_proc;
    double throughput = (n_proc / (double)CURRENT_TIME) * 100;

    
    printf("SUM: %d %.2lf %.2lf %.2lf %.2lf %.3lf\n", 
	   CURRENT_TIME, CPU_UTIL, IO_UTIL, average_turnaround, average_waiting, throughput);
  }

//END CLASS   
};

int main(int argc, char ** argv){
  int vflag = 0;
  char* path1 = NULL;
  char* path2 = NULL;
  std::string sched;
  int quantum = 1000000;
  int c;
  int index;
  opterr = 0;

  

  while ((c = getopt (argc, argv, "vs:")) != -1)
    switch (c)
      {
      case 'v':
        vflag = 1;
        break;
      case 's':
        sched = std::string(optarg);
        break;
      case '?':
        if (optopt == 's')
          fprintf (stderr, "Option -%d requires an argument.\n", optopt);
        else if (isprint (optopt))
          fprintf (stderr, "Unknown option `-%c'.\n", optopt);
        else
          fprintf (stderr,
                   "Unknown option character `\\x%x'.\n",
                   optopt);
        return 1;
      default:
        abort ();
      }
  
// printf ("aflag = %d, sched = %s\n",
 //       vflag, sched.c_str());
  
  path1 = argv[optind];
  path2 = argv[optind+1];

  //printf("%s %s", path1, path2);


  char* sched_type = &sched[0];

  if(sched.length() > 1){
    quantum = std::stoi(sched.substr(1, sched.length()));
  }
  

  Simulation sim(path1, path2, sched_type, quantum, vflag);
  sim.setup_simulation();
  sim.run_simulation();
  sim.print_simulation_details();
  return 0;
};
