#ifndef DES_H
#define DES_H

/*
The event, the fundemental unit of the discrete event simulation.
 */


class Event{
 private:
  int state;
  int transition;
  int time_stamp;
  //Process associated with an event
  Process * e_proc;

 public:
  //Setters
  void set_state(int s);
  void set_time_stamp(int ts);
  void set_transition(int t);
  void set_event_process(Process * proc);

  //Getters
  int  get_state();
  int  get_transition();
  int get_time_stamp();
  
};

