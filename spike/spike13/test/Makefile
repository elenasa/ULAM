BINDIR = ../bin

default: all

all: test
	@echo "done";

test: testf tests

pure:
	valgrind --leak-check=full ${BINDIR}/spike12f input/a.ulam
	valgrind --leak-check=full ${BINDIR}/spike12s

testf: clean
	@echo "test f:";
	${BINDIR}/spike13 input/a.ulam 2>/tmp/null
	cat input/results
	diff answer.txt input/results

tests:
	@echo "test s:";
	${BINDIR}/spike12s

clean:
	rm -f input/results
	rm -f *~
