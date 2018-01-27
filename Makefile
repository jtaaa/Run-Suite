CXX = g++
CXXFLAGS = -std=c++14 -Wall -MMD -g $(CMDOPTIONS)
EXEC = runsuite
OBJECTS = runsuite.o
DEPENDS = ${OBJECTS:.o=.d}

${EXEC}: ${OBJECTS}
	${CXX} ${OBJECTS} -o ${EXEC}

-include ${DEPENDS}

.PHONY: clean

clean:
	rm ${OBJECTS} ${DEPENDS} ${EXEC}