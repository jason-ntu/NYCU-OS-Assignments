PROG := sched_demo_311551137

.PHONY: all clean

all: 1 2 3 4 5 6

compile:
	@gcc ./$(PROG).c -o $(PROG)

1: compile
	@echo "\nCase ${@}:"
	sudo ./$(PROG) -n 1 -t 0.5 -s NORMAL -p -1

2: compile
	@echo "\nCase ${@}:"
	sudo ./$(PROG) -n 2 -t 0.5 -s FIFO,FIFO -p 10,20

3: compile
	@echo "\nCase ${@}:"
	sudo ./$(PROG) -n 3 -t 1.0 -s NORMAL,FIFO,FIFO -p -1,10,30

4: compile
	@echo "\nCase ${@}:"
	sudo ./$(PROG) -n 4 -t 0.5 -s NORMAL,FIFO,NORMAL,FIFO -p -1,10,-1,30

5: compile
	@echo "\nCase ${@}:"
	sudo ./$(PROG) -n 5 -t 0.5 -s NORMAL,NORMAL,NORMAL,NORMAL,NORMAL -p -1,-1,-1,-1,-1

6: compile
	@echo "\nCase ${@}:"
	sudo ./$(PROG) -n 6 -t 0.5 -s FIFO,FIFO,FIFO,FIFO,FIFO,NORMAL -p 5,10,15,20,30,-1

test: compile
	sudo ./sched_test.sh ./sched_demo ./$(PROG)

clean:
	rm -f sched_demo_311551137