all: 1 2 3

compile:
	@gcc ./sched_demo_311551137.c -o sched_demo_311551137

1: compile
	@echo "\nCase ${@}:"
	./sched_demo_311551137 -n 1 -t 0.5 -s NORMAL -p -1

2: compile
	@echo "\nCase ${@}:"
	./sched_demo_311551137 -n 2 -t 0.5 -s FIFO,FIFO -p 10,20

3: compile
	@echo "\nCase ${@}:"
	./sched_demo_311551137 -n 3 -t 1.0 -s NORMAL,FIFO,FIFO -p -1,10,30
