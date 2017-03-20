#include <iostream>
#include <vector>
#include <iostream>
#include <cstdio>
#include <stdio.h>
#include "scheduler.h"
#include "des.h"

/************************************************/
/*
Some utilites that might come 
in handy while running the simulation.
 */
/************************************************/

class SimUtils{
private:
  int RAND_COUNTER = 0;
  std::vector<int> rand_vals;

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
	char *tok;
	tok = strtok (tokenseq," \t\n");
	proc_i->set_incrementally((int)atoi(tok));
	//Single line
	while(tok != NULL){
	  proc_i->set_incrementally((int)atoi(tok));
	  tok = strtok (NULL," \t\n");
	}
	
	Event *e_i = new Event;
	e_i -> set_state(0);
	e_i -> set_event_process(proc_i);
	e_i -> set_transition(1);
	e_i -> set_time_stamp(1);
	des -> insert_event(e_i);
	sched -> add_process(proc_i);
      }
    }
  }

  
  int random_burst(int burst){
    return 1 + (rand_vals[RAND_COUNTER ++] % burst);
  }

//END_CLASS
};

/************************************************/
/*
The Simulation class. 
This class has an eventqueue, a scheduler
and simply gets events, adds events,
and requests a process from the scheduler.
 */
/************************************************/

class Simulation{
private:
  FIFO fifo;
  DES des;
  SimUtils utils;
  int CURRENT_TIME = 1;
  Process * CURRENT_PROCESS;
  const char * process_path;
  bool CALL_SCHEDULER = false;
public:
  Simulation(const char * proc_path){
    process_path = proc_path;
  }
  
  
  void setup_simulation(){
    utils.populate_queues(&fifo, &des, process_path);
    printf("We have populate the event queue and runqueue");
  }

  
  int get_next_event_time(){
    return des.peek_event() -> get_time_stamp();
  }
  
  void run_simulation(){
    while(des.not_empty()){
      Event*  evt = des.get_event();
      printf("Transition is %d", evt -> get_transition());
      
      switch(evt -> get_transition()){

      case 1:{
	printf("\nTRANS_TO_READY");
	handle_trans_to_ready();
      } break;

      case 2:{
	printf("\nTRANS_TO_RUN");

	if(CURRENT_PROCESS -> get_time_running() < CURRENT_PROCESS -> get_total_time()){     
	  handle_trans_to_run();
	}

      }	break;

      case 3:{
	printf("\nTRANS_TO_BLOCK");

	handle_trans_to_block();
      }	break;

      case 4:{
	printf("\nTRANS_TO_PREEMPT");
	handle_trans_to_preempt();
      }break;
      }
      
      CURRENT_TIME = evt -> get_time_stamp();
      delete evt;
     
      if(CALL_SCHEDULER){
       if(des.not_empty() && (get_next_event_time() == CURRENT_TIME)) {
	   continue;
       }
       
       CALL_SCHEDULER = false;

       if (fifo.not_empty()){
	 CURRENT_PROCESS = fifo.get_process();
       }

       Event * run_event = new Event;
       run_event -> set_state(2);
       run_event -> set_transition(2);
       run_event -> set_time_stamp(CURRENT_TIME + CURRENT_PROCESS -> get_cpu_burst());
       des.insert_event(run_event);
      }
    }
  }
  

  //Must be coming from blocked or preemption
  //Must add to the run queue
  void handle_trans_to_ready(){
    CALL_SCHEDULER = true;
  }
  

  //Create event for either preemption or blocking
  void handle_trans_to_run(){
     Event * e_i = new Event;
     int cpu_burst_i = CURRENT_PROCESS -> get_io_burst();
     int quantum_i = fifo.get_quantum();
     int trans;
     if(cpu_burst_i > quantum_i){
       CURRENT_PROCESS -> add_time_running(quantum_i);
       e_i -> set_time_stamp(CURRENT_TIME + quantum_i);
       trans = 4;
     }
     else{
       CURRENT_PROCESS -> add_time_running(cpu_burst_i);
       e_i -> set_time_stamp(CURRENT_TIME + cpu_burst_i);
       trans = 3;
     }
     e_i -> set_transition(trans);

     des.insert_event(e_i);

  }
  

  //Create an event for when its ready again
  void handle_trans_to_block(){
    Event* e_i = new Event;
    int io_burst_i = CURRENT_PROCESS -> get_io_burst();
    e_i -> set_time_stamp(CURRENT_TIME + io_burst_i);
    e_i -> set_event_process(CURRENT_PROCESS);
    e_i -> set_transition(1);
    des.insert_event(e_i);
    CALL_SCHEDULER = true;
  }
  
  //Add to run_queue;
  void handle_trans_to_preempt(){
    CALL_SCHEDULER = true;
  }
  

  void print_simulation_details(){
    ;//Need to print simulation details
  }

//END CLASS   
};

/************************************************/
// The main function definition

/************************************************/

int main(void){
  Simulation sim("/Users/Vishakh/devel/os-labs/lab2/assignment/input2");
  sim.setup_simulation();
  sim.run_simulation();
  sim.print_simulation_details();
  return 0;
};
