AS_OBJS = asembler.o parser.o symboltable.o symboltableentry.o section.o incalculable.o expression.o globalsymbol.o exceptions.o relocationrecord.o littleendian.o elfwriter.o
EM_OBJS = emulator.o symboltable.o symboltableentry.o section.o relocationrecord.o elfwriter.o objectfile.o symboladdendum.o littleendian.o processorcontext.o processorlogic.o exceptions.o

.PHONY: both clean

both: asembler emulator

asembler: $(AS_OBJS)
	g++ -Wall -o $@ $(AS_OBJS)

emulator: $(EM_OBJS)
	g++ -Wall -o $@ $(EM_OBJS)

asembler.o: asembler.cpp
	g++ -c -Wall -o $@ $<

emulator.o: emulator.cpp
	g++ -c -Wall -o $@ $<

%.o: %.cpp %.h
	g++ -c -Wall -o $@ $<

clean:
	rm *.o
	rm *.bin