#!/bin/sh

./runit_genclass.sh ./myoutputs ../src/mmu.o 
./gradeit.sh ./outputs ./myoutputs
