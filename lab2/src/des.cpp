#include <iostream>
#include <vector>
#include <iostream>
#include <cstdio>
#include <stdio.h>
#include "scheduler.h"
#include "des.h"

void Event::set_state(int s){
  state = s;
}

void Event::set_time_stamp(int ts){
  time_stamp = ts;
}

void Event::set_transition(int t){
  transition = t;
}

void Event::set_event_process(Process * proc){
  e_proc = proc;
}

int Event::get_state(){
  return state;
}

int Event::get_transition(){
  return transition;
}

int Event::get_time_stamp(){
  return time_stamp;
}



