#include <stdio.h>
#include <string.h>
#include <vector>
#include <iostream>
#include <cstdio>

// Arrays to hold metadata
std::vector<int> dcarr;// Definition codes for debugging
std::vector<int> ucarr; // UseCodes for debugging
std::vector<std::string> symbols; // defined symbols 
std::vector<int> symbolAddrGlobal; // Absolute Adresses corresponding to defined symbols
std::vector<int> symbolAddrRel;//Relative address of a symbol
std::vector<int> modAddress; // Absolute adresses of modules
std::vector<std::string> currentSymbols;
std::vector<int> currentSymbolAddress;
std :: vector<int> symbolModules;

 //Fill it up then remove

int pass1LastPos = 0;
/*
  Unified function to print out the proper errors given
  a code from 1 to 11 (as specified in the specs).
  
 */

//Pass it a code the line of the offending token and its offset
void parseerror(int errcode, int linenum, int offset){
 std::vector<std::string> errstr = {
    "NUM_EXPECTED", // Number expected 0 
    "SYM_EXPECTED", //Symbol expected  1
    "ADDR_EXPECTED", //Adressing expected which is A/E/I/R 2
    "SYM_TOO_LONG", //Symbol name is too long 3
    "TO_MANY_DEF_IN_MODULE", // >16 4
    "TO_MANY_USE_IN_MODULE", // > 16 5 
    "TO_MANY_INSTR" // total num_instr exceeds memory size 6
  };
 printf("Parse Error line %d offset %d: %s\n", linenum,  offset, errstr[errcode].c_str());
}


/*
Error handling and parsing code.
Checks various pathological cases and prints a warning.
Stops parsing for certain syntax errors.

1.Syntax error 
  -Valid counts, valid symbols, everything where its supposed to be 
  etcettera. These are show stoppers 
2.symbol defined multiple times
5. If an adress appearing in a definition exceeds the size of the module*
 */

int check_syntax_defCount(char *defCount, int line, int offset){
  int exit = 1;
  //Check that its a number
  //Check that its below 16

  if(isalpha(defCount[0])){
    parseerror(0, line, offset);
  }

  else if ((int) atoi(defCount) > 16){
    parseerror(4, line, offset);
  }

  else{
    exit = 0;
  }

  return exit;
}

int check_syntax_useCount(char *useCount, int line, int offset){
  int exit = 1;
  //Check that its a number
  //Check that its below 16

  if(isalpha(useCount[0])){
    parseerror(0, line, offset);
  }

  else if ((int) atoi(useCount) > 16){
    parseerror(5, line, offset);
  }

  else{
    exit = 0;
  }

  return exit;

}

int  check_syntax_codeCount(char *codeCount, int line, int offset){
  int exit = 1;
  
  if(isalpha(codeCount[0])){

    parseerror(0, line, offset);
  }

  else if((int)atoi(codeCount) > 512){
    parseerror(6, line, offset);
  }
  
  else{
    exit = 0;
  }
  
  return exit;
}

int check_syntax_symbol(char *symbol, int line, int offset){
  int exit = 1;
  //Check that its alpha numeric, alpha followed by numeric, not an int
  if(!isalpha(symbol[0]))
    parseerror(1, line, offset);

  else
    exit = 0;

  return exit;
}

int check_syntax_type(char *type, int line, int offset){
  int exit = 1;
  // Check that the type is a letter and its I, R, E or A Otherwise flip out
  if(strncmp(type, "I", 2) or strncmp(type, "R", 2) or strncmp(type, "E", 2) or strncmp(type, "A", 2)){
    parseerror(2, line, offset);
  }

  else{
    exit = 0;
  }
  return exit;
}

bool is_not_alnum(char c){
  if(isalnum(c))
    return false;
  else
    return true;
}

int final_check(int defCounter, int defCount, int useCounter, int useCount, int codeCounter,int codeCount, int line, int offset){
  int out = 1;
  line = line -1;
  if(defCounter < defCount && defCounter %2 == 0)
    parseerror(1, line, offset);
  else if(defCounter < defCount)
    parseerror(0, line, offset);
  else if(codeCounter < codeCount && codeCounter %2 == 0)
    parseerror(2, line, offset);
  else if (codeCount < codeCounter)
    parseerror(0, line, offset);
  else if(useCounter < useCount)
    parseerror(1, line, offset);
  else
    out = 0;
  return out;
}

void zeroLargeSymAddress(std::string sym, int  address, int moduleNum){
    for (std::vector<std::string>::const_iterator i = symbols.begin(); i != symbols.end(); ++i){
      int pos = i - symbols.begin();
      if( *i == sym and pos == moduleNum - 1)
	symbolAddrGlobal[pos] -= address;
		 
    }
}

void check_symbol_addresses(std::vector<int> &addr, std::vector <std::string> &sym, int moduleNum, int codeCount){
  for (std::vector<std::string>::const_iterator i = sym.begin(); i != sym.end(); ++i){
    int idx = i - sym.begin();
    int c_addr = addr[idx];
    std::string cSym = sym[idx];
    int mSize = (codeCount / 2);

    if(c_addr > mSize){
      printf("Warning: Module %d: %s too big %d (max=%d) assume zero relative\n", moduleNum, cSym.c_str(), c_addr, mSize -1);
      zeroLargeSymAddress(cSym, c_addr, moduleNum);
    }
  }
  
}

void symbol_print(std::string sym, int addr, std::vector<std::string> &seenSoFar, int idx){
   int counter= 0;
   int replacePos;
   for (std::vector<std::string>::const_iterator i = seenSoFar.begin(); i != seenSoFar.end(); ++i){

     if( *i == sym){
       if (counter == 0)
	 replacePos = i - seenSoFar.begin();
       counter ++;
     }
     
   }

   if (counter == 1){
     printf("Error: This variable is multiple times defined; first value used");
     symbolAddrGlobal[idx] -= symbolAddrGlobal[idx];
     symbolAddrGlobal[idx] += symbolAddrGlobal[replacePos];

   }

   else if (counter == 0)
     printf("\n%s=%d ", sym.c_str(), addr);

   else{
     symbolAddrGlobal[idx] -= symbolAddrRel[idx];
     symbolAddrGlobal[idx] += symbolAddrRel[replacePos];


   }
 }


/*
The first pass. 
Preliminary error handling and getting the absolute addresses for the modules and the symbols. 
We store the symbols in a symbol vector  and store the addresses in an address vector.
*/


//First Pass 
 int FirstPass(const char* path){
  FILE * file;
  char tokenseq [100];

  //Counters to go through the various sections
  int defCount = -1; 
  int defCounter = 0;
  
  int useCount = -1;
  int useCounter = 0;
  
  int codeCount = -1;
  int codeCounter = 0;
  
  int current_line =1;
  int prevAddress = 0;
  int moduleCount = 1;
  int moduleStart = 0;
  int moduleLen = 0;

  
  int pass1Exit = 0;
  int offset = 0;

  const char* finaltok = 0;
  modAddress.push_back(0); //First module is at 0

  file = fopen(path, "r");

  if(file == NULL){
    perror("File does not exist");
  }

  else{
    while(fgets(tokenseq, sizeof(tokenseq), file)){
      char * firsttok;
      char * tok;


      // printf ("Splitting string \"%s\" into tokens:\n", tokenseq);
      tok = strtok (tokenseq," \t\n");
      firsttok = &tokenseq[0];

      while (tok != NULL){
	finaltok = tok;
	offset = (tok - firsttok) + 1;
	//	printf ("Token: %s\n", tok);
	//   printf ("relative Position: %d\n", offset);
	//   printf ("absolute position: %d\n", current_line + offset);
	
       	if (!isalnum(*tok) && defCount != 0) {
	  std::string str(tok);
	  str.erase(std::remove_if(str.begin(), str.end(), is_not_alnum), str.end());
	  tok = (char*)str.c_str();
	}

	if(defCount == -1){
	  if(check_syntax_defCount(tok, current_line, offset)){
	     pass1Exit = 1;
	     break;
	  }
	  
	  defCount = 2 * ((int) atoi(tok));
	  dcarr.push_back(defCount /2);
	}

	   else if(defCounter < defCount){

	     if(defCounter % 2 == 0){
	       symbols.push_back(tok);
	       currentSymbols.push_back(tok);

	     }

	     else{
	       if(check_syntax_defCount(tok, current_line, offset)){
		 pass1Exit =1;
		 break;
	       }
	       symbolAddrGlobal.push_back((int)atoi(tok) + prevAddress);
	       symbolAddrRel.push_back((int)atoi(tok));

	       
	       currentSymbolAddress.push_back((int)atoi(tok));

	     }     
	     defCounter ++;
	   }
	   	   
	   else if(defCounter >= defCount){
	     if(useCount == -1){
	       if(check_syntax_useCount(tok, current_line, offset)){
		  pass1Exit = 1;
		  break;
	     }
	       useCount = (int)atoi(tok);
	     }

	     else if (useCounter < useCount){
	       if(check_syntax_symbol(tok, current_line, offset)){
		 pass1Exit = 1;
		 break;
	       }
	       
	       useCounter ++;

	     }

	     else if(useCounter >= useCount){

	       if(codeCount == -1){
		 if(check_syntax_codeCount(tok, current_line, offset)){
		   pass1Exit = 1;
		   break;
		   }
		 
		 codeCount = 2 * ((int)atoi(tok));

		 if(codeCount/2 + prevAddress > 512){
		   parseerror(6, current_line, offset);
		   pass1Exit = 1;
		   break;
		 }

	       }
	       
	       
	       else if(codeCounter < codeCount){
		 codeCounter ++;
	       }

	       else if(codeCounter >= codeCount){


		 ucarr.push_back(useCount);
		 
		   
		 modAddress.push_back(codeCount/2 + prevAddress);
		 prevAddress += codeCount/2;

	 	 if(check_syntax_codeCount(tok, current_line, offset)){
		    pass1Exit = 1;
		   break;
	       }

		 check_symbol_addresses(currentSymbolAddress,
					currentSymbols, moduleCount, codeCount);
		 currentSymbols.clear();
		 currentSymbolAddress.clear();
		 
		 defCounter = 0;
		 defCount = 2*((int)atoi(tok));

		 useCounter = 0;
		 useCount = -1;

		 moduleCount ++;
		 
		 codeCounter = 0;
		 codeCount = -1;
		 
		 // printf("New module starting at %d\n", defCount/2);
	       }
	     }
	   }
	
	tok = strtok (NULL," \t\n");
      }
     current_line += 1;

     if (*finaltok == '\n')
       pass1LastPos = offset;

     else
       pass1LastPos = offset + strlen(finaltok);
     
     			   
    
    
    }
    if(!pass1Exit ){
      check_symbol_addresses(currentSymbolAddress,
			     currentSymbols, moduleCount, codeCount);

    }
    
    if(!pass1Exit && codeCount/2 + prevAddress > 512){
		   parseerror(6, current_line - 1, offset);
		   pass1Exit = 1;
		   
    }


    else if (!pass1Exit && final_check(defCounter, defCount, useCounter, useCount, codeCounter, codeCount, current_line, pass1LastPos))
      pass1Exit = 1;
    

    modAddress.push_back(codeCount/2 + prevAddress);

    ucarr.push_back(useCount);
  }

 printf("\n");

 


 std::vector<std::string> encounteredSymbols;
 
 if(!pass1Exit){
   printf("Symbol Table \n");
   for (std::vector<std::string>::const_iterator i = symbols.begin(); i != symbols.end(); ++i){
     int pos = i - symbols.begin();
     int val = symbolAddrGlobal.at(pos);
     symbol_print(symbols[pos], val, encounteredSymbols, pos);
     encounteredSymbols.push_back(symbols[pos]);
   }
   printf("\n");
 }
 fclose(file);
 return pass1Exit;
}

/*
  Error handling for the second pass. We are checking for the following:
  3. Check if a a symbol is used in an E instruction but never defined *
  4. If a symbol is defined but never used. 
  7. A symbol appears in a use list but its not actually used in the module, not reffered to in an E type
  8. If absolute value of an address exceeds the size of the machine print an error and continue.
  9. If a relative address exceeds the size of the module*
  10. illegal immeadiate value is encountered, more than 4 numerical digits *
  11. illegal opcode , more than 4 digits*
 */

void errormessage(int code){
   std::vector<std::string> errstr = {
     "Error: Absolute address exceeds machine size; zero used", // 0
     "Error: Relative address exceeds module size; zero used", // 1
     "Error: External address exceeds length of uselist; treated as immediate", //2
     "Error: This variable is multiple times defined; first value used", //3
     "Error: Illegal immediate value; treated as 9999", //4
     "Error: Illegal opcode; treated as 9999" //5
  };
   
   printf("%s", errstr[code].c_str());
}

void errormessage(int code, const char* undefinedsym){

  printf("Error: %s is not defined; zero used", undefinedsym);
}



void checkSymbolChecklist(){
  //Check that the checklist and the symbollist contain the same
  // elements otherwise a symbol was defined but never used 
  printf("Nothing yet\n");
}


/*

Second Pass, we now use the metadata to allocate relative addresses and resolve external
references.

*/

std::vector<std::string> symbol_checklist;
std::vector<int> symbol_checklist_idx;
std::vector<std::string> uselist;
std::vector<std::string> uselist_checklist;

std::vector<std::string>cUseList; // Current uselist for external addresses
int returnSymbolLocation(const char* code, int line, int offset){
  int out = -1;

  for (std::vector<std::string>::const_iterator i = symbols.begin(); i != symbols.end(); ++i)
    if(code == *i){
      out = i - symbols.begin();
    }
 
  if(out != -1)
    out = symbolAddrGlobal.at(out);
  
  return out;
}


void handle_I(int instr_int, int outCount, const char* instr){
  int instr_len = std::to_string(instr_int).length();
  
  printf("%03i: %04i \n", outCount, instr_int);

}



void handle_A(int outCount, const char* instr, int operand, int opcode){
  if(operand > 512){
     printf("%03i: %d ", outCount, opcode*1000);
     errormessage(0);
     printf("\n");
  }
  else
    printf("%03i: %s \n", outCount, instr);
}


void handle_E(int operand, int outCount, const char* instr, int offset, int instr_int, int line, int moduleLen, int opcode){
  
  if(operand > (cUseList.size() - 1)){

    printf("%03i:  %s ", outCount, instr);
    errormessage(2);
    printf("\n");

  }
  
  else {
	
    const char * sym = cUseList.at(operand).c_str();
    uselist_checklist.push_back(sym);
    int symbol_location = returnSymbolLocation(sym, line, offset);

    if(symbol_location == -1){
      printf("%03i: %d ", outCount, opcode * 1000);
      errormessage(1, sym);
      printf("\n");
      
    }
    
    else if(operand > moduleLen){
      printf("%03i: %d ", outCount,  + returnSymbolLocation(sym, line, offset));
      errormessage(1);
      printf("\n");
    }

    else{
      printf("%03i: %d\n", outCount, (opcode * 1000) + returnSymbolLocation(sym, line, offset));

    }
  }
}

void handle_R(const char* code, const char* instr, int moduleStart, int moduleLen, int line, int offset, int outCount, int operand, int instr_int ,int opcode){

  if(operand > moduleLen){
    printf("%03i: %d ", outCount, (moduleStart + opcode * 1000));
    errormessage(1);
    printf("\n");
  }

  else
    printf("%03i: %d  \n", outCount, (instr_int + moduleStart));

}



void handle_instruction(char* code, const char* instr, int moduleStart, int moduleLen, int line, int offset, int outCount){
  int instr_int = (int)atoi(instr);
  int opcode = instr_int / 1000;
  int operand = instr_int  % 1000;

  int instr_len = std::to_string(instr_int).length();

  
  if (instr_len > 4){
    if(strncmp(code, "I", 2) == 0){
      printf("%03i: %s ", outCount, "9999");
      errormessage(4);
      printf("\n");
    }
    else{
      printf("%03i: %s ", outCount, "9999");
      errormessage(5);
      printf("\n");
    }
  }

  else if(strncmp(code, "R", 2) == 0){
    handle_R(code, instr, moduleStart, moduleLen, line, offset, outCount, operand, instr_int, opcode);
  }

  else if(strncmp(code, "I", 2) == 0){
    handle_I(instr_int, outCount, instr);
  }

  else  if(strncmp(code, "A", 2) == 0){
    handle_A(outCount, instr, operand, opcode);
      
  }
  
  else  if(strncmp(code, "E", 2) == 0){
    handle_E(operand, outCount, instr, offset, instr_int, line, moduleLen, opcode);
    
  }

}				    


int unused_symbol(const char* sym){
  int exit = 1;
  for (std::vector<std::string>::const_iterator i = uselist.begin(); i != uselist.end(); ++i){
    if(*i == sym)
      exit = 0;
  }
  return exit;
}

int unused_uselist(const char* sym){
  int exit = 1;
  for (std::vector<std::string>::const_iterator i = uselist_checklist.begin(); i != uselist_checklist.end(); ++i){
    if(*i == sym)
      exit = 0;
  }
  return exit;
}



void defined_not_used(){
  for (std::vector<std::string>::const_iterator i = symbol_checklist.begin(); i != symbol_checklist.end(); ++i){
    int pos = i - symbol_checklist.begin();
    int mod = symbol_checklist_idx[pos];
    
    if(unused_symbol(symbol_checklist[pos].c_str()))
      printf("Warning: Module %d: %s was defined but never used \n", mod +1 , symbol_checklist[pos].c_str());
  }
}

void uselist_not_used(int moduleNum){
  for (std::vector<std::string>::const_iterator i = cUseList.begin(); i != cUseList.end(); ++i){
     int pos = i - cUseList.begin();
     
     if(unused_uselist(cUseList[pos].c_str())){
       printf("Warning: Module %d: %s appeared in the uselist but was not actually used\n", moduleNum, cUseList[pos].c_str());
     }
   }
}

void SecondPass(const char* path)
{
  FILE * file;
  char tokenseq [100];

  int defCount = -1;
  int defCounter = 0;
  int useCount = -1;
  int useCounter = 0;
  int codeCount = -1;
  int codeCounter = 0;
  int current_line =1;
  int prevAddress = 0;
  int moduleCount = 0;
  int codeCounterAbs = 0;
  char code[2];
  
  file = fopen(path, "r");

  if(file == NULL){
    perror("File does not exist");
  }

  else{
    printf("Memory Map \n");
    while(fgets(tokenseq, sizeof(tokenseq), file)){
      char * firsttok;
      char * tok;
      
      int offset;
      int moduleStart;
      int moduleLen;
      int print_line_counter = 0;

      tok = strtok (tokenseq," \t\n");
      firsttok = &tokenseq[0];

      while (tok != NULL){
	offset = (tok - firsttok) + 1;

	if (!isalnum(*tok) && defCount != 0) {
	  std::string str(tok);
	  str.erase(std::remove_if(str.begin(), str.end(), is_not_alnum), str.end());
	  tok = (char*)str.c_str();
	}

	
	if(defCount == -1){
	  defCount = 2 * ((int) atoi(tok));
	  
	}

	else if(defCounter < defCount){
	  if(defCounter % 2 == 0 ){
	    symbol_checklist.push_back(tok);
	    symbol_checklist_idx.push_back(moduleCount);
	  }
	  
	  defCounter ++;
	}
	   	   
	else if(defCounter >= defCount){

	  if(useCount == -1){
	    useCount = (int)atoi(tok);
	  }

	  else if (useCounter < useCount){
	    useCounter ++;
	    uselist.push_back(tok);
	    cUseList.push_back(tok);
	  }

	  else if(useCounter >= useCount){

	    if(codeCount == -1){
	      codeCount = 2 * ((int)atoi(tok));
	    }
	       
	    else if(codeCounter < codeCount){
	      if(codeCounter % 2 == 0){
		strcpy(code, tok);
		codeCounter ++;
	      }

	      else{
		moduleStart = modAddress.at(moduleCount);
		moduleLen = modAddress.at(moduleCount + 1) - moduleStart;

		handle_instruction(code, tok, moduleStart, moduleLen, current_line, offset, codeCounterAbs);
		codeCounterAbs ++;
		codeCounter ++;
	      }


	    }

	    else if(codeCounter >= codeCount){
	      uselist_not_used(moduleCount + 1);
	      uselist_checklist.clear();
	      cUseList.clear();

	      prevAddress += codeCount / 2;
	      defCounter = 0;
	      defCount = 2*((int)atoi(tok));
	      moduleCount ++;
	      useCounter = 0;
	      useCount = -1;
	      
	      codeCounter = 0;
	      codeCount = -1;
	      

	      
	    }
	  }
	}
	tok = strtok (NULL," \t\n");
      }
      current_line += 1;
    }
    if(codeCounter <= codeCount)
      uselist_not_used(moduleCount + 1);
  }
  fclose(file);
}

int main(int argc, char *path[]){
  int badExit = FirstPass(path[1]);

  if(! badExit){
    SecondPass(path[1]);
  }
  defined_not_used();
}
