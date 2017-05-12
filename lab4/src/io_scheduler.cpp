/*
****************************************************************************************
Lab 4: IO Scheduler 
Vishakh Gopu

This lab implements an IO scheduler that takes requests to write to disk and
handles them when possible.

Implemented are the following schedulers:

FIFO: 
Handle the requests as they come, first come first serve

SSTF:
Handle the request in the queue closest to the current head position first

Scan:
Handle next request in the direction the head is already going.
Switch directions once you hit the end.

Cscan:
Same as scan but wrap over to the begining of the queue once 
reaching the end.

Fscan:
Same as scan but there are two arrays, one where requests are placed
when IO is going on, and another that is fully handled before switching

****************************************************************************************
*/


#include <vector>
#include <cstdio>
#include <algorithm>
#include <fstream>
#include <string.h>
#include <cmath>
#include <stdint.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <deque>
#include <iostream>
#include <getopt.h>

/*
****************************************************************************************
The instruction class. 
Represents an IO instruction.
****************************************************************************************
*/

class Instruction{
public:
  int ts = -1;
  int loc = -1;
    
  //Set the read first then the page
  void set_incrementally(int val){
    if(ts == -1)
      ts = val;
    else
      loc = val;
  }
};


/*
****************************************************************************************
Some definitions that are easier kept global 
****************************************************************************************
*/

std::deque<Instruction> instruction_list;
int TIMESTAMP = 0;
int TALLY = 0;
int N_INSTR;
bool TESTING = false;
std::string algo_spec;


/*
****************************************************************************************
Some utilities that are handy for the simulation
****************************************************************************************
*/

class SimUtils{
public:
  void read_instructions(char const *instr_path){
    FILE * file;
    char tokenseq [1000];
        
    file = fopen(instr_path, "r");
        
    if(file == NULL){
      perror("File does not exist");
    }
        
    else{
      //Whole file
      while(fgets(tokenseq, sizeof(tokenseq), file)){
	char *tok;
	bool skipped = false;
	tok = strtok (tokenseq," \t\n");
	Instruction instr_i;
                
	//Skip comment
	if(*tok == '#'){
	  tok = NULL;
	  skipped = true;
	}
                
	//Single line
	while(tok != NULL){
	  instr_i.set_incrementally((int)atoi(tok));
	  tok = strtok (NULL," \t\n");
	}
                
	if(!skipped){
	  TALLY ++;
	  N_INSTR++;
	  instruction_list.push_front(instr_i);
	}
	skipped = false;
      }
    }
  }
};


/*
***************************************************************************
// IO Schedulers
***************************************************************************
*/

/*
****************************************************************************************
The abstract class scheduler.
Represents the general IO scheduler interface that specific
schedulers must implement.
****************************************************************************************
*/

class Scheduler{
protected:
  std::vector<Instruction> queue;
  int current_pos = 0;
  int target_pos = 0;
  int TOTAL_MOVEMENT = 0;

  bool check_again = false;
  bool first_request = true;
  bool target_acquired = false;
  
  double TOTAL_TURNAROUND =0;
  double TOTAL_WAITTIME = 0;
  int MAX_WAIT = 0;
  Instruction CURRENT_INSTRUCTION;


public:
  virtual void add_to_queue(Instruction io_instr) = 0;

  virtual void take_action() = 0;

  virtual int find_closest() = 0;

  bool isFinished(){
    return TALLY == 0;
  }
    
  void move_to_target(){
    if(target_pos > current_pos){
      current_pos ++;
      TOTAL_MOVEMENT++;
    }

    else if (target_pos < current_pos){
      current_pos --;
      TOTAL_MOVEMENT++;
    }
    else
      take_action();
  }
    
  int get_next_target(){
    int res = 0;
    target_acquired = false;
    if(!queue.empty()){
      res = find_closest();
      target_acquired = true;
      increment_wait_time();
    }
    return res;
  }


  void get_next_target_update(){
    target_acquired = true;
    increment_wait_time();
  }


  void set_current(Instruction instr){
    CURRENT_INSTRUCTION = instr;
  }


  void increment_wait_time(){
    int diff = TIMESTAMP - CURRENT_INSTRUCTION.ts;
    if(diff > MAX_WAIT)
      MAX_WAIT = diff;
    TOTAL_WAITTIME += diff;
  }


  void increment_total_tt(){
    int diff = TIMESTAMP - CURRENT_INSTRUCTION.ts;
    TOTAL_TURNAROUND += diff;
  }


  void print_array(){
    printf("\n");
    for(std::vector<Instruction>::iterator it = queue.begin(); it != queue.end(); ++it) {
      printf(" %d ", (*it).loc);
    }
    printf("\n");
  }


  void print_stats(){
    printf("SUM: %d %d %.2lf %.2lf %d\n", TIMESTAMP - 1,
	   TOTAL_MOVEMENT, TOTAL_TURNAROUND / N_INSTR, TOTAL_WAITTIME/ N_INSTR, MAX_WAIT);
  }
};



/*
***************************************************************************
FIFO scheduler.
***************************************************************************
*/


class FIFO: public Scheduler{
public:
  void add_to_queue(Instruction io_instr){
    if(queue.empty())
      queue.push_back(io_instr);
    else
      queue.insert(queue.begin(), io_instr);
  }
    

  int find_closest(){
    CURRENT_INSTRUCTION = queue.back();
    int res = CURRENT_INSTRUCTION.loc;
    queue.pop_back();
    return res;
  }
    
  void take_action(){
    // Just started
    if(first_request){
      target_pos = get_next_target();
      first_request = false;
    }

    else if (check_again){
      target_pos = get_next_target();
      check_again = false;
    }

    //else if you have reached
    else if(target_pos == current_pos && TALLY != 0){
      increment_total_tt();
      TALLY --;
      target_pos = get_next_target();
            
    }
        
    //If theres a target
    if(target_acquired)
      move_to_target();
    else if(TALLY != 0)
      check_again = true;
  }
};


/*
***************************************************************************
// Shortest seek time first
***************************************************************************
*/

class SSTF:public Scheduler{
public:
  void add_to_queue(Instruction io_pos){
    queue.push_back(io_pos);
  }
    
  int find_closest(){
    std::vector<Instruction>::iterator res_it = queue.begin();
    int res = (queue.at(0)).loc;
    int min_gap = abs(res - current_pos);
        
    for(std::vector<Instruction>::iterator it = queue.begin(); it != queue.end(); ++it) {
      int diff = abs((*it).loc - current_pos);
      if(diff < min_gap){
	min_gap = diff;
	res_it = it;
      }
    }

    CURRENT_INSTRUCTION = (queue.at(res_it - queue.begin()));
    res = CURRENT_INSTRUCTION.loc;
    queue.erase(res_it);
        
    return res;
  }
    
  void take_action(){
    // Just started
    if(first_request){
      target_pos = get_next_target();
      current_pos = 0;
      first_request = false;
    }
    else if (check_again){
      target_pos = get_next_target();
      check_again = false;
    }
    //else if you have reached
    else if(target_pos == current_pos && TALLY !=0){
      increment_total_tt();
      TALLY --;
      if(TESTING)
	print_array();
      target_pos = get_next_target();
    }

    //If theres a target
    if(target_acquired)
      move_to_target();
    else if(TALLY != 0)
      check_again = true;
  }
};

/*
***************************************************************************
// General class for scanning algorithms, cscan and scan both extend.
//Fscan became too complex and I ran out of time so its stand alone
***************************************************************************
*/

class ScannerAlgo: public Scheduler{
protected:
  int dir = 0;
  bool zero = false;

public:
  virtual void handle_swap() = 0;
  void take_action(){
    if(first_request){
      target_pos = get_next_target();
      first_request = false;
    }

    else if (check_again){
      target_pos = get_next_target();
      check_again = false;
    }

    else if (target_pos == current_pos && TALLY != 0){
      TALLY --;
      handle_swap();
      increment_total_tt();
            
      if(zero){
	target_acquired = false;
	if(!queue.empty()){
	  target_pos = (queue.at(0)).loc;
	  CURRENT_INSTRUCTION = queue.at(0);
	  queue.erase(queue.begin());
	  get_next_target_update();
	}
      }
                
      else{
	target_pos = get_next_target();
      }
    }
        
    if(target_acquired)
      move_to_target();
        
    else if (TALLY != 0)
      check_again = true;
  }
    
  void add_to_queue(Instruction io_instr){
    std::vector<Instruction>::iterator ins_pos = queue.begin();
        
    if(queue.empty())
      queue.push_back(io_instr);
        
    else if(io_instr.loc > queue.back().loc)
      queue.push_back(io_instr);
        
    else{
      for(std::vector<Instruction>::iterator it = queue.begin(); it != queue.end(); ++it) {
	if(io_instr.loc > (*it).loc)
	  ins_pos = it + 1;
      }
      queue.insert(ins_pos, io_instr);
    }
  }
    

  int find_closest(){
    std::vector<Instruction>::iterator res_pos;
    int res = 0;
        
    if(dir == 0){
      res = find_closest_for();
    }

    else{
      res = find_closest_rev();
    }
    return res;
  }

  int find_closest_for(){
    std::vector<Instruction>::iterator res_it = queue.begin();
        
    int res = (queue.at(0)).loc;
        
    for (std::vector<Instruction>::iterator it = queue.begin(); it != queue.end(); ++it ){
      if((*it).loc >= current_pos){
	res_it = it;
	break;
      }
    }
        
    CURRENT_INSTRUCTION = (queue.at(res_it - queue.begin()));
    res = CURRENT_INSTRUCTION.loc;
    queue.erase(res_it);
        
    return res;
  }
  
  int find_closest_rev(){
    std::vector<Instruction>::reverse_iterator res_it = queue.rbegin();

    int res = queue.back().loc;
        
    for (std::vector<Instruction>::reverse_iterator it = queue.rbegin(); it != queue.rend(); ++it ){
      if((*it).loc <= current_pos){
	res_it = it;
	res = (*it).loc;
	break;
      }
    }

    std::vector<Instruction>::iterator it_forward =  --(res_it.base());
        
    CURRENT_INSTRUCTION = (queue.at(it_forward - queue.begin()));
    res = CURRENT_INSTRUCTION.loc;
    queue.erase(it_forward);
        
    return res;
  }
};


class Scan:public ScannerAlgo{
  void handle_swap(){
    if (dir == 1){
      if(current_pos < (queue.front()).loc){
	dir = 0;
      }
    }
    else if (dir == 0){
      if(current_pos > (queue.back()).loc)
	dir = 1;
    }
  }

};

/*
***************************************************************************
/Cscan
***************************************************************************
*/
class cscan: public ScannerAlgo{
  void handle_swap(){
    if(current_pos > queue.back().loc){
      zero = true;
    }
    else
      zero = false;
  }
    
};


/*
***************************************************************************
// Fscan
***************************************************************************
*/

class Fscan: public Scheduler{
protected:
  std::vector<Instruction> add_queue;
  int dir = 0;
  bool zero = false;
    
public:
  void handle_swap(){
    if(current_pos > queue.back().loc){
      dir = 1;
    }
    else if (current_pos < queue.front().loc){
      dir = 0;
    }
  }

  void handle_array(){
    if(queue.empty()){
      queue = add_queue;
      add_queue.clear();
      dir = 0;
    }

  }

  void take_action(){
    if(first_request){
      handle_array();
      target_pos = get_next_target();
      first_request = false;
    }
        
    else if (check_again){
      handle_array();
      target_pos = get_next_target();
      check_again = false;
    }
	
    else if (target_pos == current_pos && TALLY != 0){
      increment_total_tt();
      TALLY --;
      handle_array();
      handle_swap();
      target_pos = get_next_target();
    }
        
    if(target_acquired)
      move_to_target();
    else if (TALLY != 0)
      check_again = true;
  }
    
    
  void add_to_queue(Instruction io_instr){
    std::vector<Instruction>::iterator ins_pos = add_queue.begin();
        
    if(add_queue.empty())
      add_queue.push_back(io_instr);
        
    else if(io_instr.loc > add_queue.back().loc)
      add_queue.push_back(io_instr);
        
    else{
      for(std::vector<Instruction>::iterator it = add_queue.begin(); it != add_queue.end(); ++it) {
	if(io_instr.loc > (*it).loc)
	  ins_pos = it + 1;
      }
            
      add_queue.insert(ins_pos, io_instr);
    }
  }
    
  int find_closest(){
    std::vector<Instruction>::iterator res_pos;
    int res = 0;
        
    if(dir == 0){
      res = find_closest_for();
    }
        
    else{
      res = find_closest_rev();
    }
    return res;
  }
    
  int find_closest_for(){
    std::vector<Instruction>::iterator res_it = queue.begin();
        
    int res = (queue.at(0)).loc;
        
    for (std::vector<Instruction>::iterator it = queue.begin(); it != queue.end(); ++it ){
      if((*it).loc >= current_pos){
	res_it = it;
	break;
      }
    }
        
    CURRENT_INSTRUCTION =(queue.at(res_it - queue.begin()));
    res = CURRENT_INSTRUCTION.loc;
    queue.erase(res_it);
        
    return res;
  }
    
    
  int find_closest_rev(){
    std::vector<Instruction>::reverse_iterator res_it = queue.rbegin();
        
    int res = queue.back().loc;
        
    for (std::vector<Instruction>::reverse_iterator it = queue.rbegin(); it != queue.rend(); ++it ){
      if((*it).loc <= current_pos){
	res_it = it;
	break;
      }
    }
    //Kind of hacky I know
    std::vector<Instruction>::iterator it_forward =  --(res_it.base());

    CURRENT_INSTRUCTION = (queue.at(it_forward - queue.begin()));
    res = CURRENT_INSTRUCTION.loc;
    queue.erase(it_forward);
        
    return res;
  }
    
};


/*
****************************************************************************************
The main simulation class. Handles an io request one at a time.
Checks if any actions need to be taken.
****************************************************************************************
*/

class Simulation{
private:
  SimUtils utils;
  Scheduler * scheduler;
  Instruction current_instruction;
  int last_io_event;
  int simulation_complete = false;

public:
  Simulation(const char* instr_path){
    utils.read_instructions(instr_path);
    current_instruction = instruction_list.back();
    instruction_list.pop_back();
    TIMESTAMP = current_instruction.ts;
    initialize_algo();
        
  }
    
  void initialize_algo(){
    if(algo_spec == "f")
      scheduler = new Fscan;
    if(algo_spec == "i")
      scheduler = new FIFO;
    if(algo_spec == "s")
      scheduler = new Scan;
    if(algo_spec == "c")
      scheduler = new cscan;
    if(algo_spec == "j")
      scheduler = new SSTF;
  }
  void run_simulation(){
        
    while(!scheduler -> isFinished() || !instruction_list.empty()){
      if(!instruction_list.empty() || current_instruction.ts >= TIMESTAMP){
	if(current_instruction.ts == TIMESTAMP){
	  scheduler -> add_to_queue(current_instruction);

	  if(!instruction_list.empty()){
	    current_instruction = instruction_list.back();
	    instruction_list.pop_back();
	  }
	}
      }
      scheduler -> take_action();
      TIMESTAMP ++;
        
    }
            
        
    scheduler -> print_stats();
  }
};

/*
****************************************************************************************
Main class. Reads arguments from the command line and
calls the simulation.
****************************************************************************************
*/

int main(int argc, char ** argv) {
  char* path1 = NULL;
  int c;
  opterr = 0;
    
  while ((c = getopt (argc, argv, "s:")) != -1)
    switch (c)
      {
      case 's':
	algo_spec = std::string(optarg);
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
    
  path1 = argv[optind];
    
  Simulation sim(path1);
  sim.run_simulation();
  return 0;
}
