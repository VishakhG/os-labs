#include <iostream>
#include <vector>
#include <iostream>
#include <cstdio>
#include <stdio.h>
#include <scheduler.h>

using namespace std;

class Event{
private:
  int state;
  int time_stamp;
  int transition;

  Process * e_proc;

public:
  void set_state(int s){
    state = c_state;
  }

  void set_time_stamp(int ts){
    time_stamp = ts;
  }

  void set_transition(int t){
    transition = t;
  }

  void set_event_process(Process * proc){
    e_proc = proc;
  }

  int get_state(){
    return state;
  }

  int get_transiton(){
    return transition;
  }

  int get_time_stamp(){
    return time_stamp;
  }
};


