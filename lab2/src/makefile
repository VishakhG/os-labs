scheduler_sim.out:des.o  simulation.o scheduler.o
	g++ -std=c++11 -g  des.o scheduler.o simulation.o -o scheduler_sim.out

simulation.o: scheduler.h  des.h  simulation.cpp
	g++ -std=c++11 -c simulation.cpp

des.o: des.cpp des.h  scheduler.h
	g++ -std=c++11  -c  des.cpp

scheduler.o: scheduler.cpp scheduler.h
	g++ -std=c++11  -c scheduler.cpp

clean:
	rm -f *.o rm -f *.out
