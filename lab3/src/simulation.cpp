#include <iostream>
#include <vector>
#include <iostream>
#include <cstdio>
#include <stdio.h>
#include <algorithm> 
#include <fstream>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>


#include "structures.h"
#include "pagers.h"

class SimUtils{
private:
  //Keep track of random number offset
  int RAND_COUNTER = 1;
  std::vector<int> rand_vals;
  int pid = 0;
  
public:
  //Load the random numbers
  void read_random_numbers(char const *rfile_path){
    FILE * file;
    char tokenseq [100];
    file = fopen(rfile_path, "r");

    if(file == NULL){
      perror("File does not exist");
    }

    else{
      //Whole file
      while(fgets(tokenseq, sizeof(tokenseq), file)){
	char *tok;
	tok = strtok (tokenseq," \t\n");
	
	//Single line
	while(tok != NULL){
	  rand_vals.push_back((int)atoi(tok));
	  tok = strtok (NULL," \t\n");
	}
      }
    }
  }

  //Return a random burst 
  int random_burst(int burst){
    if (RAND_COUNTER > rand_vals.size())
      RAND_COUNTER = 1;
    return (1 + (rand_vals.at(RAND_COUNTER++) % burst));
  }

 //END of SimUtils class
};


class Simulation{
  //END of Simulation class
};


int main(int argc, char ** argv){
  printf("Hello MMU");
};
