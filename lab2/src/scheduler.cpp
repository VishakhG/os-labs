#include <iostream>
#include <vector>

using namespace std;

class Process{
private:
  int start_time;
  int arrival_time;
  int total_time;
  int cpu_burst;
  int io_burst;

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

};

class ProcessList{
  
 private:
  std::vector<Process> proc_list;

 public:
  void set_process(Process proc){
    proc_list.push_back(proc);
  }

  int get_process(){
    Process proc = proc_list.at(0);
    int io_time = proc.get_io_burst();
    return io_time;
  }

};




int main(void) {
  Process proc;
  proc.set_io_burst(10);

  ProcessList pr_list;
  pr_list.set_process(proc);
  printf("%i", pr_list.get_process());
}
