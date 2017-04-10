#ifndef PAGERS_H
#define PAGERS_H

/*
********************************

Class of page replacement algorithms.


********************************
*/

// base abstract class, pager API
class Pager{

};


class NRU: public Pager {

};


class Random: public Pager{

};


class FIFO: public Pager{
  
};


class Second_chance: public Pager{

};

class Clock: public Pager{

};

class Aging: public Pager{

};


#endif
