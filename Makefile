AS_OBJS = bin/asembler.o bin/parser.o bin/symboltable.o bin/symboltableentry.o bin/section.o bin/incalculable.o bin/expression.o bin/globalsymbol.o bin/exceptions.o bin/relocationrecord.o bin/littleendian.o bin/elfwriter.o
EM_OBJS = bin/emulator.o bin/symboltable.o bin/symboltableentry.o bin/section.o bin/relocationrecord.o bin/elfwriter.o bin/objectfile.o bin/symboladdendum.o bin/littleendian.o bin/processorcontext.o bin/processorlogic.o bin/exceptions.o

.PHONY: both clean

both: bin/asembler bin/emulator

bin/asembler: $(AS_OBJS)
	g++ -Wall -o $@ $(AS_OBJS)

bin/emulator: $(EM_OBJS)
	g++ -Wall -o $@ $(EM_OBJS)

bin/asembler.o: source/asembler.cpp
	g++ -c -Wall -o $@ $<

bin/emulator.o: source/emulator.cpp
	g++ -c -Wall -o $@ $<

bin/%.o: source/%.cpp source/%.h
	g++ -c -Wall -o $@ $<

clean:
	rm *.o
	rm *.bin