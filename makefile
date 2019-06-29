main.exe: main.o queue.o priority_queue.o simulator.o
	gcc -g -o main main.o queue.o priority_queue.o simulator.o
	rm *.o

main.o: include/cpu_scheduling_simulator.h src/main.c
	gcc -g -c src/main.c

simulator.o: include/cpu_scheduling_simulator.h src/simulator.c 
	gcc -g -c src/simulator.c

queue.o: include/cpu_scheduling_simulator.h src/queue.c
	gcc -g -c src/queue.c

priority_queue.o: include/cpu_scheduling_simulator.h src/priority_queue.c 
	gcc -g -c src/priority_queue.c

clean:
	rm *.o