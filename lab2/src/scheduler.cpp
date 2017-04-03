#include <iostream>
#include <vector>
#include <iostream>
#include <cstdio>
#include <stdio.h>
#include "scheduler.h"

/*
This is the process class.
The fundemental unit of the scheduler.
*/


void Process::set_arrival_time(int AT){
  arrival_time = AT;
}

void Process::set_total_time(int TT){
  total_time = TT;
}

void Process::set_finishing_time(int FT){
  finishing_time = FT;
}
void Process::set_cpu_burst(int CB){
  cpu_burst = CB;
}

void Process::add_cpu_waiting(int CPU){
  cpu_waiting += CPU;
}

void Process::set_time_prev_state(int TPS){
  time_in_previous_state = TPS;
}

void Process::set_priority(int P){
  priority = P;
  dynamic_priority = P - 1;
}

int Process::get_time_prev_state(){
  return time_in_previous_state;
}  

void Process::set_io_burst(int IO){
  io_burst = IO;
}

void Process::set_pid(int p){
  pid = p;
}
// Getters

int Process:: get_finishing_time(){
  return finishing_time;
}

int Process::get_cpu_waiting(){
  return cpu_waiting;
}

int Process::get_arrival_time(){
  return arrival_time;
}

int Process::get_total_time(){
  return total_time;
}

int Process::get_cpu_burst(){
  return cpu_burst;
}

int Process::get_io_burst(){
  return io_burst;
}

void Process::add_time_running(int r){
  time_running += r;
}

void Process::add_time_blocked(int b){
  time_blocked += b;
}

int Process::get_time_blocked(){
  return time_blocked;
}

int Process::get_pid(){
  return pid;
}
int Process::get_time_running(){
  return time_running;
}

int Process::get_priority(){
  return priority;
}
// Set one after another depending on what hasn't been set
void Process::set_incrementally(int val){
  if (arrival_time == -1){
    arrival_time = val;
    last_event_time = val;
  }
  else if (total_time == -1){
    total_time = val;
  }

  else if (cpu_burst == -1){
    cpu_burst = val;
  }

  else if (io_burst == -1){
    io_burst = val;
  }
}

bool Scheduler::empty(){
  return runqueue.empty() && expiredqueue.empty();
}

int Scheduler::get_quantum(){
  return quantum;
}

/*
The FIFO scheduler
*/
void FIFO::add_process(Process * proc){
  runqueue.insert(runqueue.begin(), proc);
}
  
Process * FIFO::get_process(){
  //Just pop from the back
  Process * proc_i = runqueue.back();
  runqueue.pop_back();
  return proc_i;
}


void LCFS::add_process(Process * proc){
  //Add at end
  runqueue.push_back(proc);
}


Process * LCFS::get_process(){
  //Get from end too
  Process * proc_i = runqueue.back();
  runqueue.pop_back();
  return proc_i;
}


Process * PRIO::get_process(){
  if(runqueue.empty()){
      runqueue = expiredqueue;
      expiredqueue.clear();
    }

  Process * proc_i = runqueue.back();
  runqueue.pop_back();
  return proc_i;
}


bool PRIO::goes_after(Process * p1, Process * p2){
  int prio1 = p1 -> dynamic_priority;
  int prio2 = p2 -> dynamic_priority;

  if (prio1 > prio2){
    return true;
  }

  else if (prio1 < prio2){
    return false;
  }
  else{
    return false;
  }
} 



void PRIO:: add_process(Process * proc){
  if(proc -> dynamic_reset){
    proc -> dynamic_reset = false;
    add_expired(proc);

  }
  else{
    add_active(proc);
  }

}

void PRIO::add_active(Process * proc){
  
  std::vector<Process *>:: iterator position = runqueue.end();

  if(runqueue.empty()){
    runqueue.push_back(proc);
  }

  else{
      
    for(std::vector<Process *>::iterator it = runqueue.begin(); it != runqueue.end(); ++it) {
      if (!goes_after(proc, *it)){
	position = it;
	break;
      }
    }
    
    runqueue.insert(position, proc);
  
  }
}


void PRIO::add_expired(Process * proc){
  std::vector<Process *>:: iterator position = expiredqueue.end();

  if(expiredqueue.empty()){
    expiredqueue.push_back(proc);
  }

  else{
      
    for(std::vector<Process *>::iterator it = expiredqueue.begin(); it != expiredqueue.end(); ++it) {
      if (!goes_after(proc, *it)){
	position = it;
	break;
      }
    }
    
    expiredqueue.insert(position, proc);
  
  }
}


void SJF::add_process(Process *proc){

  std::vector<Process *>:: iterator position = runqueue.end();

  if(runqueue.empty()){
    runqueue.push_back(proc);
  }

  else{
      
    for(std::vector<Process *>::iterator it = runqueue.begin(); it != runqueue.end(); ++it) {
      if (!goes_after(proc, *it)){
	position = it;
	break;
      }
    }
    
    runqueue.insert(position, proc);
  
  }


}


bool SJF::goes_after(Process * p1, Process * p2){
  int remaining1 = p1 -> get_total_time() - p1 -> get_time_running();
  int remaining2 = p2 -> get_total_time() - p2 -> get_time_running();

  if (remaining2 > remaining1){
    return true;
  }

  else if (remaining2 < remaining1){
    return false;
  }

  else{
    return false;
  }
} 

Process * SJF::get_process(){
  Process * proc_i = runqueue.back();
  runqueue.pop_back();
  return proc_i;
  
}
