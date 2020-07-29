asembler: asembler.o parser.o symboltable.o symboltableentry.o
	g++ -o $@ asembler.o parser.o symboltable.o symboltableentry.o

parser.o: parser.h parser.cpp
	g++ -c -o $@ parser.cpp

symboltable.o: symboltable.h symboltable.cpp
	g++ -c -o $@ symboltable.cpp

symboltableentry.o: symboltableentry.h symboltableentry.cpp
	g++ -c -o $@ symboltableentry.cpp

clean:
	rm *.o