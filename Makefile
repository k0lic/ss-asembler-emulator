AS_OBJS = asembler.o parser.o symboltable.o symboltableentry.o section.o incalculable.o expression.o globalsymbol.o exceptions.o relocationrecord.o littleendian.o elfwriter.o
EM_OBJS = emulator.o symboltable.o symboltableentry.o section.o relocationrecord.o elfwriter.o objectfile.o symboladdendum.o littleendian.o exceptions.o

asembler: $(AS_OBJS)
	g++ -Wall -o $@ $(AS_OBJS)

emulator: $(EM_OBJS)
	g++ -Wall -o $@ $(EM_OBJS)

%.o: %.cpp %.h
	g++ -c -Wall -o $@ $<

clean:
	rm *.o