scheduler_sim.out:structures.o  simulation.o pagers.o
	g++ -std=c++11 -g  structures.o pagers.o simulation.o -o mmu_sim.out

simulation.o: pagers.h  structures.h  simulation.cpp
	g++ -std=c++11 -c simulation.cpp

structures.o: structures.cpp structures.h  pagers.h
	g++ -std=c++11  -c  structures.cpp

pagers.o: pagers.cpp pagers.h
	g++ -std=c++11  -c pagers.cpp

clean:
	rm -f *.o rm -f *.out
