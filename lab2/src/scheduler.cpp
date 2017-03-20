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

void Process::set_cpu_burst(int CB){
  cpu_burst = CB;
}

void Process::set_io_burst(int IO){
  io_burst = IO;
}
  
// Getters

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

int Process::get_time_running(){
  return time_running;
}
// Set one after another depending on what hasn't been set
void Process::set_incrementally(int val){
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

bool Scheduler::not_empty(){
  return ! runqueue.empty();
}

/*
The FIFO scheduler
*/
void FIFO::add_process(Process * proc){
    //Insert at end of queue
     runqueue.push_back(proc);
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
  


