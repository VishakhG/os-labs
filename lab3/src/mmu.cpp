//
//  main.cpp
//  mmu
//
//  Created by Vishakh gopu on 4/15/17.
//  Copyright © 2017 Vishakh gopu. All rights reserved.
//

#include <iostream>
#include <vector>
#include <iostream>
#include <cstdio>
#include <stdio.h>
#include <algorithm>
#include <fstream>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <iostream>
#include <getopt.h>
#include <cmath>
#include <stdint.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <deque>

class Instruction{
public:
  int rw = -1;
  int pg = -1;
    
  void set_incrementally(int val){
    if(rw == -1)
      rw = val;
    else
      pg = val;
  }
};



class PTE {
public:
  struct packed_struct {
    unsigned int present:1;
    unsigned int modified:1;
    unsigned int referenced:1;
    unsigned int paged_out:1;
    unsigned int frame_ref:7;
  } entry;
    
  bool isPresent(){
    bool out = (entry.present == 1);
    return out;
  }
    
  void zero(){
    entry.present = 0;
    entry.modified = 0;
    entry.referenced = 0;
    entry.paged_out = 0;
    entry.frame_ref = 0;
        
  }
};


class Frame{
public:
  int page_map = 0;
  int id = 0;
};



/*
  Some global vars for the simulation
*/

//PAGE TABLE
std::vector<PTE> page_table(64);

//FREE LIST
std::vector<Frame*> free_list;

//FRAME TABLE
std::vector<Frame*> frametable;

//INSTRUCTIONS
std::vector<Instruction> instruction_list;


int NUM_FRAMES = 32;

void print_frame_table(){
  std::vector<Frame*> out(NUM_FRAMES);
  
  for(std::vector<Frame *>::iterator it = frametable.begin(); it != frametable.end(); ++it){
    int pos = (*it) -> id;
    out[pos] = (*it);
  }

  for(std::vector<Frame *>::iterator it = out.begin(); it != out.end(); ++it){
    int page_map_i = (*it) -> page_map;
    if(page_map_i == -1)
      printf("* ");
    else
      printf("%d ", page_map_i);
       
           
  }
   printf("\n");
}

void print_page_table(){
    
  for(std::vector<PTE>::iterator it = page_table.begin(); it != page_table.end(); ++it) {
    int counter = (int)(it - page_table.begin());
        
    if(it -> entry.present == 1){
      std::string referenced = "-";
      std::string modified = "-";
      std::string swapped = "-";
            
      if(it -> entry.referenced == 1)
	referenced = "R";
      if(it -> entry.modified == 1)
	modified = "M";
      if(it -> entry.paged_out == 1)
	swapped = "S";
            
      printf("%d:%s%s%s ", counter, referenced.c_str(), modified.c_str(), swapped.c_str());
            
    }
    else{
      if(it -> entry.paged_out == 1)
	printf("# ");
      else
	printf("* ");
    }
  }
  printf("\n");
};

/*
  Simulation Utilities
*/

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
                
	if(!skipped)
	  instruction_list.push_back(instr_i);
	skipped = false;
      }
    }
  }
    
    
    
  //Return a random burst
  int random_burst(int burst){
    if (RAND_COUNTER > (rand_vals.size()-1))
      RAND_COUNTER = 1;
    return ((rand_vals.at(RAND_COUNTER++) % burst));
  }
    
  //END of SimUtils class
};


SimUtils utils;


/*
  PAGERS
*/

class Pager{
public:
  virtual Frame *allocate_frame(Frame **frame_old) = 0;
};

class FIFO:public Pager{
private:
  std::vector<Frame *> queue;
  bool first_request = true;

public:
  void initialize_queue(){
    for(std::vector<Frame*>::iterator it = frametable.begin(); it != frametable.end(); ++it) {
      queue.insert(queue.begin(), *it);
    }
  }
  
   Frame* allocate_frame(Frame **frame_old){
     if(first_request){
       initialize_queue();
       first_request = false;
     }

    Frame *replaced = queue.back();
    queue.pop_back();
    queue.insert(queue.begin(), replaced);
    *frame_old = replaced;
    return replaced;
        
  }
};

class Random: public Pager{
public:
  Frame* allocate_frame(Frame **frame_old){
    int random_idx =  utils.random_burst(NUM_FRAMES);
    Frame *replaced = frametable[random_idx];
    *frame_old = replaced;
        
    return replaced;
        
  }
};

class SecondChance:public Pager{
private:
  std::vector<Frame *> queue;
  bool first_request = true;

public:
  void initialize_queue(){
    for(std::vector<Frame*>::iterator it = frametable.begin(); it != frametable.end(); ++it) {
      queue.insert(queue.begin(), *it);
    }
  }
  
  Frame* sub_allocate_frame(){
    if(first_request){
       initialize_queue();
       first_request = false;
     }

    Frame *replaced = queue.back();
    queue.pop_back();
    queue.insert(queue.begin(), replaced);
        
    int pg_i = replaced -> page_map;
        
    if(page_table[pg_i].entry.referenced == 1){
      page_table[pg_i].entry.referenced = 0;
      replaced = NULL;
    }
        
    return replaced;
        
  }
    
  Frame* allocate_frame(Frame **frame_old){
    Frame * res = NULL;
    while(res == NULL){
      res = sub_allocate_frame();
    }
        
    *frame_old = res;
    return res;
  }
    
    
    
};


class NRU: public Pager{
private:
  int CLOCK_INTERRUPT = 10;
  int CLEAR_REF_COUNTER = 0;
  //0
  std::vector<int> noref_nomod;
  //1
  std::vector<int> noref_mod;
  //2
  std::vector<int> ref_nomod;
  //3
  std::vector<int> ref_mod;
    
  //The highest category with a frame in it
  int lowest_cat = 0;
    
public:
  void find_frames(){
    for(std::vector<PTE>::iterator it = page_table.begin(); it != page_table.end(); ++it) {

      int ref_i = it -> entry.referenced;
      int mod_i = it -> entry.modified;
      int present_i = it -> entry.present;
        
      if(present_i == 1){
	if((ref_i == 1) && (mod_i == 1))
	  ref_mod.push_back(it -> entry.frame_ref);
	else if((ref_i == 1) && (mod_i == 0))
	  ref_nomod.push_back(it -> entry.frame_ref);
	else if((ref_i == 0) && (mod_i == 1))
	  noref_mod.push_back(it -> entry.frame_ref);
	else if((ref_i == 0) && (mod_i == 0))
	  noref_nomod.push_back(it -> entry.frame_ref);

	if(CLEAR_REF_COUNTER  == CLOCK_INTERRUPT){
	  it -> entry.referenced = 0;
	}
      }
        
        

    }
    
    if(!ref_mod.empty())
      lowest_cat = 3;
    if (!ref_nomod.empty())
      lowest_cat = 2;
    if (!noref_mod.empty())
      lowest_cat = 1;
    if (!noref_nomod.empty())
      lowest_cat = 0;

    
};
    
  Frame * random_sample(std::vector<int> sample_space){
    Frame * sample = NULL;
        
    int frame_id = sample_space[utils.random_burst(static_cast<int>(sample_space.size()))];
        
    for(std::vector<Frame*>::iterator it = frametable.begin(); it != frametable.end(); ++it) {
      int c_id = (*it) -> id;
      if (c_id == frame_id)
	sample = *it;
    }
        
    return sample;
  }
    
//  void clear_reference_counters(){
//    for(std::vector<PTE>::iterator it = page_table.begin(); it != page_table.end(); ++it) {
//      if(it -> entry.present == 1)
//	it -> entry.referenced = 0;
//    }
//  }
    
  void clear_counters(){
    noref_nomod.clear();
    noref_mod.clear();
    ref_nomod.clear();
    ref_mod.clear();
  }
  Frame * allocate_frame(Frame **frame_old){
    CLEAR_REF_COUNTER ++;
    find_frames();
    
    Frame *res = NULL;
        
    switch(lowest_cat) {
    case 0 : res = random_sample(noref_nomod);
      break;
    case 1 : res = random_sample(noref_mod);
      break;
    case 2 : res = random_sample(ref_nomod);
      break;
    case 3: res = random_sample(ref_mod);
    }
        

    if(CLEAR_REF_COUNTER  == CLOCK_INTERRUPT){
      CLEAR_REF_COUNTER = 0;
    }

    *frame_old = res;
    clear_counters();
    return res;
  }
    
};

class clock_virtual: public Pager {
public:
  int CLOCK_HAND = 0;
  PTE *pte_i;
  Frame * allocate_frame(Frame **frame_old){
    bool page_evicted = false;
    Frame * res = NULL;
    while(!page_evicted){
            
      if(CLOCK_HAND > 63)
	CLOCK_HAND = 0;
            
      pte_i = &page_table[CLOCK_HAND];
            
      if(pte_i -> entry.present == 0){
	CLOCK_HAND ++;
      }
            
      else if(pte_i -> entry.referenced == 1 ){
	pte_i -> entry.referenced = 0;
	CLOCK_HAND ++;

      }
            
      else {
	res = frametable[pte_i -> entry.frame_ref];
	//printf("ITSOUT CLOCK %d frame %d", CLOCK_HAND, pte_i -> entry.frame_ref );
	CLOCK_HAND ++;
	page_evicted = true;
                
      }
    }
        
    
    *frame_old = res;
    return res;
  }
};



class clock_physical: public Pager{
public:
  int CLOCK_HAND = 0;
  Frame * allocate_frame(Frame **frame_old){
    Frame * res = NULL;
    bool frame_evicted = false;
        
    while(!frame_evicted){
      if(CLOCK_HAND > NUM_FRAMES -1)
	CLOCK_HAND = 0;
            
      Frame * frame_i = frametable[CLOCK_HAND];
      PTE *pte_i = &page_table[frame_i -> page_map];
            
            
      if(pte_i -> entry.referenced == 1){
	pte_i -> entry.referenced = 0;
	CLOCK_HAND ++;
                
      }
      else{
	res = frame_i;
	CLOCK_HAND ++;
	frame_evicted = true;
      }
            
    };
    *frame_old = res;
    return res;
  }
    
};


class aging: public Pager{
protected:
  std::vector<std::deque<int>*> aging_counters;
    
public:
  Frame * allocate_frame(Frame **frame_old){
    Frame * res = NULL;
    res = frametable[find_minimum_frame()];
    *frame_old = res;
    clear_pte_refs();
    return res;
  }
    
  void clear_pte_refs(){
    for(std::vector<PTE>::iterator it = page_table.begin(); it != page_table.end(); ++it) {
      if(it -> entry.present == 1)
	it -> entry.referenced = 0;
    }
  }
    
  void update_counter(std::deque<int> * counter, int refval){
    counter->push_front(refval);
    counter -> pop_back();
  }
    
    

  uint64_t vector_to_int(std::deque<int> * counter){
    uint64_t out = 0;
    int place = 0;
        
    for(std::deque<int>::reverse_iterator rit = counter->rbegin(); rit != counter -> rend(); ++rit){
      out += ((std::pow(2, place)) * (*rit));
      place ++;
    }
    
    return out;
  }
    
    
  virtual int find_minimum_frame() = 0;
};


class aging_virtual: public aging{
public:
  aging_virtual(){
    for(int i = 0; i < 64; ++i ){
      std::deque<int> *temp_i = new std::deque<int>(32);
      for(int i = 0; i < 32; ++i){
	temp_i -> at(i) = 0;
      }

      aging_counters.push_back(temp_i);
    }
        
  }
    

  int find_minimum_frame(){
    uint64_t min = 0;
    int frame_pos = 0;
    int start_gap = 0;


    for (int i=0; i<64; ++i){
      int ref_i = page_table[i].entry.referenced;
      update_counter(aging_counters[i], ref_i);
      start_gap ++;
      
      if(page_table[i].entry.present == 1){
	min = vector_to_int(aging_counters[i]);
	frame_pos = (page_table[i].entry.frame_ref);
	break;
      }
                
    }
        

        
    for(std::vector<std::deque<int> *>::iterator it = aging_counters.begin() + start_gap; it != aging_counters.end(); ++it) {
      
      int n = (int)(it - aging_counters.begin());
      
      int ref_i = page_table[n].entry.referenced;
            
      //if(page_table[n].entry.present == 1)
      update_counter(*it, ref_i);

      uint64_t temp = vector_to_int(*it);
      
      if(temp < min){
	int present_i = page_table[n].entry.present;
                
	if(present_i == 1){
                    
	  min = temp;
	  frame_pos = (page_table[n].entry.frame_ref);
                    
	}
                
      }
    }

    return  frame_pos;
  }
    
};



class aging_physical: public aging{
public:
  aging_physical(){
    for(int i = 0; i < NUM_FRAMES; ++i ){
      std::deque<int> *temp_i = new std::deque<int>(32);
      for(int i = 0; i < 32; ++i){
	temp_i -> at(i) = 0;
      }
      aging_counters.push_back(temp_i);
    }
        
  }
    
  int find_minimum_frame(){
    //Update first one
    int ref_i = page_table[frametable[0] -> page_map].entry.referenced;
    update_counter(aging_counters[0], ref_i);   

    uint64_t min = vector_to_int(aging_counters[0]);
    int frame_pos = 0; 
        
    for(std::vector<std::deque<int> *>::iterator it = aging_counters.begin() + 1; it != aging_counters.end(); ++it) {
      

      int n = (int)(it - aging_counters.begin());
      int ref_i = page_table[frametable[n] -> page_map].entry.referenced;
      update_counter(*it, ref_i);            
      
      uint64_t temp = vector_to_int(*it);
      if(temp < min){
	min = temp;
	frame_pos = n;

      }
    }
    
    std::fill(aging_counters[frame_pos] -> begin(), aging_counters[frame_pos] -> end(), 0);
    
    return frame_pos;
  }
};

/*
  SIMULATION
*/

class Sim_stats{
  public:
  uint64_t UNMAP = 0;
  uint64_t MAP = 0;
  uint64_t PAGEIN = 0;
  uint64_t PAGEOUT = 0;
  uint64_t ZERO  = 0;
  uint64_t TOTALCOST = 0;
};

/*
  Some options that will be needed to determine what gets printed
*/

bool O_opt = false;
bool F_opt = false ;
bool P_opt = false;
bool S_opt = false;
bool f_opt = false;
bool a_opt = false;
bool p_opt = false;
std::string algo_spec = "";

class Simulation{
private:
  Pager * algo;
  int line_counter = 0;
  Sim_stats stats;

public:
  void print_stats(){
    printf("SUM %d U=%llu M=%llu I=%llu O=%llu Z=%llu ===> %llu\n",
	   line_counter, 
	   stats.UNMAP, 
	   stats.MAP, 
	   stats.PAGEIN, 
	   stats.PAGEOUT,
	   stats.ZERO,
	   stats.TOTALCOST
	   );    
  };
  void print_info(int code, int arg1, int arg2){
    if(O_opt){
      switch (code) {
	//INST
      case 0:
	printf("==> inst: %d %d \n", arg1, arg2);
	break;
	//UNMAP
      case 1:
	printf("%d: UNMAP %d %d \n", line_counter, arg1, arg2);
	break;
	//OUT
      case 2:
	printf("%d: OUT %d %d \n", line_counter, arg1, arg2);

	break;
	//IN
      case 3:
	printf("%d: IN %d %d \n", line_counter, arg1, arg2);
	break;
	//MAP
      case 4:
	printf("%d: MAP %d %d \n", line_counter, arg1, arg2);
	break;
	//ZERO
      case 5:
	printf("%d: ZERO %d \n",line_counter, arg2);

	break;
      default:
	break;
      }

    }
  };
    
  void initialize_algo(){
    if(algo_spec == "f")
      algo = new FIFO;
    if(algo_spec == "N")
      algo = new NRU;
    if(algo_spec == "r")
      algo = new Random;
    if(algo_spec == "s")
      algo = new SecondChance;
    if(algo_spec == "c")
      algo = new clock_physical;
    if(algo_spec == "X")
      algo = new clock_virtual;
    if(algo_spec == "a")
      algo = new aging_physical;
    if(algo_spec == "Y")
      algo = new aging_virtual;
  }
 
  Simulation(char const *i_path, char const *rand_path){
    //Initialize free frame list
    
    initialize_algo();
    
    utils.read_instructions(i_path);
    utils.read_random_numbers(rand_path);
        
    for(int i = 0; i < NUM_FRAMES; ++i ){
      Frame *frame_i = new Frame;
      frame_i -> id = i;
      free_list.insert(free_list.begin(), frame_i);
    }
        
  }
    
  // Check if there are free frames available
  Frame * allocate_from_free(int pg_num){
    Frame * frame_i = free_list.back();
    free_list.pop_back();
        
    frametable.push_back(frame_i);
        
        
    //Set the inverse mapping
    frame_i -> page_map = pg_num;
        
    //return pointer to the free frame
    return frame_i;
        
  };
    
    
  Frame *get_frame(Frame **old_frame, int pg_num){
    Frame * res ;
    if(!free_list.empty()){
            
      res = allocate_from_free(pg_num);
      *old_frame = NULL;
    }
        
    else{
      //TODO make sure oldframe is changed;
      res = algo -> allocate_frame(old_frame);
    }
        
    return res;
  };
    

    
  void run_simulation(){
        
    while(line_counter <= (instruction_list.size() - 1)){

      Instruction instr_i;
      instr_i = instruction_list[line_counter];
            
      int rw_i = instr_i.rw;
      int pg_i = instr_i.pg;
            
      int frame_id = 0;
            
      print_info(0, rw_i, pg_i);
            
      PTE *new_pte = &page_table[pg_i];
            
            
      if(! page_table[pg_i].isPresent()){
                
	Frame * old_frame;
	Frame * new_frame;
                

	new_frame = get_frame(&old_frame, pg_i);
                
	frame_id = new_frame -> id;
                
                
	if(old_frame != NULL){

	  int old_map = old_frame -> page_map;
	  PTE *old_pte = &page_table[old_map];
	  old_pte -> entry.present = 0;
	  old_pte ->entry.frame_ref = 33;
                    
	  //printf("%d: UNMAP %d %d \n", line_counter, old_map, frame_id);
	  print_info(1, old_map, frame_id);
	  stats.UNMAP ++;
	  stats.TOTALCOST += 400;
	  if(old_pte -> entry.modified == 1){
	    //printf("%d: OUT %d %d \n", line_counter, old_map, frame_id);
	    stats.PAGEOUT ++;
	    stats.TOTALCOST += 3000;

	    print_info(2, old_map, frame_id);
	    old_pte -> entry.paged_out = 1;
	    old_pte -> entry.modified = 0;
	  }
                    
	  old_frame -> page_map = -1;
                   
                    
	}
                
                
	//If it was paged out before then page in
	if(new_pte -> entry.paged_out == 1){
	  //printf("%d: IN %d %d \n", line_counter, pg_i, frame_id);
	  stats.PAGEIN ++;
	  stats.TOTALCOST += 3000;
	  print_info(3, pg_i, frame_id);
	}
	//Else just zero
	else{
	  //printf("zero \n");
	  stats.ZERO ++;
	  stats.TOTALCOST += 150;
	  print_info(5, -1, frame_id);
	}
                
	//printf("%d: MAP %d %d \n",line_counter, pg_i, frame_id);
	stats.MAP ++;
	stats.TOTALCOST += 400;
	print_info(4, pg_i, frame_id);
                
	new_pte -> entry.frame_ref = frame_id;
	new_frame -> page_map = pg_i;
	new_pte -> entry.present = 1;
                
      }
            
      //Definitly referenced
      new_pte -> entry.referenced = 1;

            
      //If its a write then its modified
            
      if(rw_i == 1)
	new_pte -> entry.modified = 1;
      stats.TOTALCOST += 1;      
      line_counter ++;
    }
  }
  //END of Simulation class
};

/*
  MAIN
*/

int main(int argc, char ** argv) {
    
  char* path1 = NULL;
  char* path2 = NULL;

  int c;
  int index;
  opterr = 0;
    
  while ((c = getopt (argc, argv, "a:f:o:")) != -1)
    switch (c)
      {
      case 'a':
	algo_spec = std::string(optarg);
	break;
      case 'f':
	NUM_FRAMES = int(atoi(optarg));
	break;
      case 'o':
	for(int i=0; optarg[i]!='\0'; i++)
	  {
	    if(optarg[i] == 'O')
	      O_opt = true;
	    else if(optarg[i] == 'F')
	      F_opt = true;
	    else if(optarg[i] == 'P')
	      P_opt = true;
	    else if(optarg[i] == 'S')
	      S_opt = true;
	    else if(optarg[i] == 'p')
	      p_opt = true;
	    else if (optarg[i] == 'f')
	      f_opt = true;
	    else if (optarg[i] == 'a')
	      a_opt = true;
	  }
	break;
      case '?':
	if (optopt == 'o')
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
  path2 = argv[optind + 1];

  Simulation sim(path1, path2);

  sim.run_simulation();
  if(P_opt)
    print_page_table();

  if(F_opt){
    print_frame_table();
  }

  if(S_opt){
    sim.print_stats();
  }
  
  
  return 0;
}
