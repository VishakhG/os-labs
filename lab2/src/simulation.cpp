#include <iostream>
#include <vector>
#include <iostream>
#include <cstdio>
#include <stdio.h>
#include "scheduler.h"
#include "des.h"
#include <algorithm> 

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
  FIFO fifo;
  DES des;
  SimUtils utils;

  int CURRENT_TIME = 1;
  Process * CURRENT_PROCESS = NULL;
  const char * process_path;
  const char * random_path;
  bool CALL_SCHEDULER = false;
  int TOTAL_CPU_TIME = 0;
  int TOTAL_IO_TIME = 0;
  


public:
  Simulation(const char * proc_path, const char *rand_path){
    process_path = proc_path;
    random_path = rand_path;
  }
  

  void setup_simulation(){
    utils.read_random_numbers(random_path);
    utils.populate_queues(&fifo, &des, process_path);
  }

  
  int get_next_event_time(){
    return (des.peek_event()) -> get_time_stamp();
  }
  

  void run_simulation(){
    while(des.not_empty()){
      Event* evt = des.get_event();
      CURRENT_TIME = evt -> get_time_stamp();
      int pid = (evt -> get_process()) -> get_pid();
      int last_trans_time = evt -> get_process() -> last_event_time;
      
      int TPI = CURRENT_TIME -  last_trans_time;
      if(evt -> get_state() == 1){
	evt -> get_process() -> add_cpu_waiting(TPI);
      }
      
      evt -> get_process() -> last_event_time = CURRENT_TIME;

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
	  printf("\n process %d is done!", (evt -> get_process()) -> get_pid());
	  (evt->get_process()) -> set_finishing_time(CURRENT_TIME);
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
	    if (fifo.not_empty()){
	      CURRENT_PROCESS = fifo.get_process();
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
    std::string f_string = "\n Process %d transitioning from " +  trans_from + " to " + 
	trans_to + " at time %d ";
    
    printf(f_string.c_str(), pid, CURRENT_TIME);

}
  

  //Must be coming from blocked or preemption
  //Must add to the run queue
  void handle_trans_to_ready(Event* c_event){
    fifo.add_process(c_event -> get_process());
    CALL_SCHEDULER = true;
  }


  //Create event for either preemption or blocking
  void handle_trans_to_run(Event* c_event){
    Event * e_i = new Event;
    Process * proc_i = c_event -> get_process();
     int random_burst_i = utils.random_burst(proc_i -> get_cpu_burst());
     int cpu_burst_i = std::min(random_burst_i, (proc_i -> get_total_time()) - (proc_i -> get_time_running()));
     printf("CPU_Burst %d", cpu_burst_i);
     int quantum_i = fifo.get_quantum();
     int trans;
     
     if(cpu_burst_i > quantum_i){
       proc_i -> add_time_running(quantum_i);
       e_i -> set_time_stamp(CURRENT_TIME + quantum_i);
       TOTAL_CPU_TIME += quantum_i;
       trans = 4;
       }
     
     else{
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
    proc_i -> add_time_blocked(io_burst_i);
    e_i -> set_time_stamp(CURRENT_TIME + io_burst_i);
    e_i -> set_event_process(proc_i);
    e_i -> set_transition(1);
    e_i -> set_state(3);
    des.insert_event(e_i);
    TOTAL_IO_TIME += io_burst_i;
    CALL_SCHEDULER = true;
  }
  
  //Add to run_queue;
  void handle_trans_to_preempt(Event * c_event){
    fifo.add_process(c_event -> get_process());
    CALL_SCHEDULER = true;
  }
  
  void print_simulation_details(){
    for(std::vector<Process *>::iterator it = done_list.begin(); it != done_list.end(); ++it) {
      int AT = (*it) -> get_arrival_time();
      int TC = (*it) -> get_total_time();
      int CB = (*it) -> get_cpu_burst();
      int IO = (*it) -> get_io_burst();
      int PRIO = (*it) -> get_priority();
      int FT = (*it) -> get_finishing_time();
      int TT = FT - AT;
      int IT = (*it) -> get_time_blocked();
      int CW = (*it) -> get_cpu_waiting();
      int pid = (*it) -> get_pid();

      printf("\n %d %d %d %d %d %d | %d  %d  %d %d ",  pid, AT, TC,CB,IO,PRIO,FT,TT,IT,CW);
    }
    int CPU_UTIL = TOTAL_CPU_TIME ;
    int IO_UTIL = TOTAL_IO_TIME;
    printf("\n %d %d %d", CURRENT_TIME, CPU_UTIL, IO_UTIL);
  }

//END CLASS   
};

/************************************************/
// The main function definition

/************************************************/

int main(void){
  Simulation sim("/Users/Vishakh/devel/os-labs/lab2/assignment/input2", 
		 "/Users/Vishakh/devel/os-labs/lab2/assignment/rfile");
  sim.setup_simulation();
  sim.run_simulation();
  sim.print_simulation_details();
  return 0;
};
