#include <iostream>
#include <vector>
#include <iostream>
#include <cstdio>
#include <stdio.h>

using namespace std;

class Process{
private:
  int start_time = false;
  int arrival_time = false;
  int total_time = false;
  int cpu_burst = false;
  int io_burst = false;
  
public:
  //Setters
  void set_arrival_time(int AT){
    arrival_time = AT;
  }

  void set_total_time(int TT){
    total_time = TT;
  }

  void set_cpu_burst(int CB){
    cpu_burst = CB;
  }

  void set_io_burst(int IO){
    io_burst = IO;
  }
  
  //Getters
  int get_arrival_time(){
    return arrival_time;
  }

  int get_total_time(){
    return total_time;
  }

  int get_cpu_burst(){
    return cpu_burst;
  }

  int get_io_burst(){
    return io_burst;
  }

  void push_val(int val){
    if(start_time == false){
      start_time = val;
    }
    else if (arrival_time == false){
      arrival_time = val;
    }
    else if (total_time == false){
      total_time = val;
    }

    else if (cpu_burst == false){
      cpu_burst = val;
    }

    else if (io_burst == false){
      io_burst = val;
    }
  }

};

class ProcessList{
private:
  std::vector<Process> proc_list;

public:
  void read_processes(char const *input_path){
    FILE * file;
    char tokenseq [100];
    
    file = fopen(input_path, "r");
    
    if(file == NULL){
      perror("File does not exist");
    }
    
    else{
      //Whole file
      while(fgets(tokenseq, sizeof(tokenseq), file)){
	Process proc_i;	
	char *tok;
	tok = strtok (tokenseq," \t\n");
	proc_i.push_val((int)atoi(tok));
	//Single line
	while(tok != NULL){
	  //printf("%s \n", tok);
	  proc_i.push_val((int)atoi(tok));
	  tok = strtok (NULL," \t\n");
	}
	proc_list.push_back(proc_i);
      }
    }
  }

  void print_processes(){
    for(std::vector<Process>::iterator it = proc_list.begin(); it != proc_list.end(); ++it) {
      printf("%i is the arrival time \n", (*it).get_arrival_time());
      printf("%i is the total CPU time \n", (*it).get_total_time());
      printf("%i is the CPU burst time \n", (*it).get_cpu_burst());
      printf("%i is the IO burst time \n", (*it).get_io_burst());
    }
  }
};

int main(void) {
  Process proc;
  proc.set_io_burst(10);

  ProcessList pr_list;
  pr_list.read_processes("/Users/Vishakh/devel/os-labs/lab2/assignment/input1");
  pr_list.print_processes();

}
