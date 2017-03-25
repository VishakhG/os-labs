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
    printf("\nat%d",val);
    arrival_time = val;
    last_event_time = val;
  }
  else if (total_time == -1){
    printf("\ntt%d",val);
    total_time = val;
  }

  else if (cpu_burst == -1){
    printf("\ncb%d",val);
    cpu_burst = val;
  }

  else if (io_burst == -1){
    printf("\nio%d",val);
    io_burst = val;
  }
}

bool Scheduler::not_empty(){
  return ! runqueue.empty();
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

int  FIFO::get_quantum(){
  return quantum;
}
  


