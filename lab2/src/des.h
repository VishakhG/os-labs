#ifndef DES_H
#define DES_H

/*
The event, the fundemental unit of the discrete event simulation.
 */


class Event{
 private:
  int state = -1;
  int transition;
  int time_stamp;
  int previous_state;
  //Process associated with an event
  Process * e_proc;


 public:
  //Setters
  void set_state(int s);
  void set_time_stamp(int ts);
  void set_transition(int t);
  void set_event_process(Process * proc);
  void set_previous_state(int s);

  //Getters
  int  get_state();
  int  get_transition();
  int get_previous_state();
  int  get_time_stamp();
  Process * get_process();
};
#endif

class DES{
private:
  std::vector<Event*> e_queue;

  
  public:
  Event * get_event();
  Event * peek_event();
  void insert_event(Event * e);
  bool goes_after(Event* e, Event* e2);
  bool not_empty();
};
