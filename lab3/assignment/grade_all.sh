#!/bin/sh

./runit_genclass.sh ./myoutputs ../src/vmm.o  && 
./gradeit.sh ./outputs ./myoutputs
