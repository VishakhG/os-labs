/*
  Vishakh Gopu
  lab-3: Memory Management
  4-20-17


  This file implements a memory manager that takes instructions one 
  by one that consist of an operation (read || write) and a virtual page adress.
  If the page is physical memory or there is plenty or room in physical memory
  then nothing happens/a free frame is assigned. Otherwise we use an interface
  to a paging algorithm to get a free frame. Several specific algorithms, all
  implemented as classes implement this interface:

  1.FIFO
  2.SECOND CHANCE
  3.AGING
  4.CLOCK
  5.RANDOM

  NOTES:
  - I convert from a binary to decimal number using my own function in AGING,
  This might be a bit slower than other solutions but I felt it simplified things.

  - Second chance could inherit from FIFO, but there were enough differences that I thought
  it wasn't worth it.
*/


#include <vector>
#include <cstdio>
#include <algorithm>
#include <fstream>
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

/*
********************************************************************************
A simple class to hold instructions
********************************************************************************
*/

class Instruction{
public:
  int rw = -1;
  int pg = -1;

  //Set the read first then the page
  void set_incrementally(int val){
    if(rw == -1)
      rw = val;
    else
      pg = val;
  }
};


/*
********************************************************************************
A page table entry.
Consists of a bit-field that holds the relevant bits. 
********************************************************************************
*/

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


/*
********************************************************************************
A Frame has an inverse map to the corresponding
virtual page, and a UUID. 
********************************************************************************
*/

class Frame{
public:
  int page_map = 0;
  int id = 0;
};


/*
********************************************************************************
Global variables and functions that are shared less messily this way.
********************************************************************************
*/

//PAGE TABLE
std::vector<PTE> page_table(64);

//FREE LIST
std::vector<Frame*> free_list;

//FRAME TABLE
std::vector<Frame*> frametable;

//INSTRUCTIONS
std::vector<Instruction> instruction_list;

//NUM FRAMES
int NUM_FRAMES = 32;

//Print the frames
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

//Print the page tables
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
********************************************************************************
Consolidates misc utilites that are needed for the simulation.
Read files ect.
********************************************************************************
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
};

//We want to keep this global too
SimUtils utils;


/*
********************************************************************************
The pager interface (abstract class)
********************************************************************************
*/

//Abstract class that defines our interface
class Pager{
public:
  virtual Frame *allocate_frame(Frame **frame_old) = 0;
};


/*
********************************************************************************
FIFO pager, returns the frame that was used least recently. 
********************************************************************************
*/

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

/*
********************************************************************************
The Random pager. 
Returns a random frame from the frametable
********************************************************************************
*/

class Random: public Pager{
public:
  Frame* allocate_frame(Frame **frame_old){
    int random_idx =  utils.random_burst(NUM_FRAMES);
    Frame *replaced = frametable[random_idx];
    *frame_old = replaced;
        
    return replaced;
        
  }
};


/*
********************************************************************************
The Second Chance pager.
Like FIFO but if the page about to be evicted has been referenced recently
then give it another chance by putting it at the back of the queue again.
********************************************************************************
*/

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


/*
********************************************************************************
The Not recently used algorith,
Bins the available frames into categories based on the referenced and 
modified bits and then picks a frame at random from the lowest precedence class.
********************************************************************************
*/


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


/*
********************************************************************************
Clock algorithm - based on virtual pages.
Moves a hand around the page table and does the same as NRU.
********************************************************************************
*/

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


/*
********************************************************************************
Clock based on physical frames
Moves a hand around the frametable and does what NRU does.
********************************************************************************
*/

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


/*
********************************************************************************
The superclass for aging pagers.
The aging pager maintains a counter for each page (phys/virtual) and shifts
it to the right after a page fault. Then it prepends the referenced bit 
of the corresponding PTE. 
********************************************************************************
*/

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


/*
********************************************************************************
Aging based on the virtual pages.
********************************************************************************
*/

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


/*
********************************************************************************
Aging based on the physical frames
********************************************************************************
*/

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
********************************************************************************
A small class holding all the simulation statistics
********************************************************************************
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

 */

/*
********************************************************************************
Some global flags that need to be shared
********************************************************************************
*/ 

bool O_opt = false;
bool F_opt = false ;
bool P_opt = false;
bool S_opt = false;
bool f_opt = false;
bool a_opt = false;
bool p_opt = false;
std::string algo_spec = "";


/*
********************************************************************************
The simulation itself.
We take instructions and move the simulation forward by checking to 
see whether the virtual page is present and if not requesting a frame from a 
pager.
********************************************************************************
*/ 

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
                    
	  //UNMAP
	  print_info(1, old_map, frame_id);
	  stats.UNMAP ++;
	  stats.TOTALCOST += 400;
	  if(old_pte -> entry.modified == 1){
	    //PAGEOUT
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
                
	//MAP
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
};


/*
********************************************************************************
Main function.
We handle command line options in any order and print 
the summaries if needed.
********************************************************************************
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
