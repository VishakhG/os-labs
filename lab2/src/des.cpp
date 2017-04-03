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

void Event::set_previous_state(int s){
  previous_state = s;
}

int Event::get_state(){
  return state;
}

int Event::get_transition(){
  return transition;
}

int Event::get_previous_state(){
  return previous_state;
}
int Event::get_time_stamp(){
  return time_stamp;
}

Process * Event::get_process(){
  return e_proc;
}

Event *  DES::get_event(){
  Event * r_event = e_queue.back();
  e_queue.pop_back();
  return r_event;
}


void DES::insert_event(Event * e){

  std::vector<Event *>:: iterator position = e_queue.end();

  if(e_queue.empty()){
    e_queue.push_back(e);
  }

  else{
      
    for(std::vector<Event *>::iterator it = e_queue.begin(); it != e_queue.end(); ++it) {
      if (!goes_after(e, *it)){
	position = it;
	break;
      }
    }
    
   // printf("inserting id %d at %d with ts %d ", e -> get_process() -> get_pid(),(int)( position - e_queue.begin()), e-> get_time_stamp());
    
    e_queue.insert(position, e);
    

    }
  }

bool DES::not_empty(){
  return !e_queue.empty();
};

Event * DES::peek_event(){
  return e_queue.back();
}

//Compare whether an event(e1) goes after another (e2)
//Return true if it does, false if it doesn't
bool DES::goes_after(Event * e1, Event * e2){
  int ts_1 = e1 -> get_time_stamp();
  int ts_2 = e2 -> get_time_stamp();

  int pid1 = e1->get_process() -> get_pid();
  int pid2 = e2->get_process() -> get_pid();
  
  if (pid1 == pid2){
      int trans1 = e1 -> get_transition();
      int trans2 = e2 -> get_transition();

      if(e1 -> get_process() ->  get_total_time() - e1 -> get_process()-> get_time_running()  == 0 )
	return true;
      else if (e2 -> get_process() -> get_total_time() - e1 ->get_process()-> get_time_running()  == 0 )
	return false;
      else if(trans1 ==  3)
	return true;
      else if(trans2 ==  3)
	return false;
      else if (trans1 == 4)
	return true;
      else
	return false;
      
    }

  //Obviously check if the timestamp is less
  if (ts_1 < ts_2){
    return true;
  }
  else if (ts_1 > ts_2){
    return false;
  }
  
  else{
    return false;
  }
}
