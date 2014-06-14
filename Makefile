CC = mpicc
#CC = /opt/mpich2-gnu/bin/mpicc
CFLAGS = -g -O3
LDFLAGS = -lm
EXE = fsim

MAIN = src/main.c
SRCS = src/IODriver.c src/realTimeClock.c src/frog.c src/cell.c src/clock.c src/pool.c src/actorPatternFramework.c src/frog-functions.c  src/ran2.c
OBJS = $(patsubst src/%.c, bin/%.o, $(SRCS))
HEADERS = $(patsubst src/%.c, include/%.h, $(SRCS))



all: executable

bin/%.o: src/%.c Makefile
	$(CC) $(CFLAGS) $(LDFLAGS) -I include $< -c -o $@


executable: $(OBJS) $(MAIN) Makefile
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) $(MAIN) -I include -o bin/$(EXE)


.PHONY: clean test doxygen
clean:
	rm -rf bin/*.o bin/$(EXE) 

test:
	mpiexec -n 120 ./bin/$(EXE)
