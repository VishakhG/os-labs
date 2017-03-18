#include <iostream>
#include <vector>
#include <iostream>
#include <cstdio>
#include <stdio.h>
#include <scheduler.h>
#include <des.h>


/*
Some utilites that might come 
in handy while running the simulation.
 */
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

  void populate_run_queue(Scheduler * sched, char const *input_path){
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
	proc_i->push_val((int)atoi(tok));
	//Single line
	while(tok != NULL){
	  //printf("%s \n", tok);
	  proc_i->push_val((int)atoi(tok));
	  tok = strtok (NULL," \t\n");
	}
	sched->add_process(proc_i);
      }
    }
  }

  int random_burst(int burst){
    return 1 + (rand_vals[rand_counter ++] % burst);
  }

};


/*
The Simulation class. 
This class has an eventqueue, a scheduler
and simply gets events, adds events,
and requests a process from the scheduler.
 */

class Simulation{
private:
  Scheduler scheduler;
  std::vector<Event*> queue;

  int CURRENT_TIME;
  Process * CURRENT_PROCESS;
  
public:
  void setup_simulation(){
    ;//Need to populate the event queue intially
  };

  void run_simulation(){
    ;// Need to run simulation
  }

  void print_simulation_details(){
    ;//Need to print simulation details
  }
  
};



int main(void){
  Simulation sim;
  sim.setup_simulation();
  sim.run_simulation();
  sim.print_simulation_details();
  
  return 0;
};
