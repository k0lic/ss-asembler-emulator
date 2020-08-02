OBJS = asembler.o parser.o symboltable.o symboltableentry.o section.o incalculable.o expression.o globalsymbol.o exceptions.o relocationrecord.o littleendian.o

asembler: $(OBJS)
	g++ -Wall -o $@ $(OBJS)

%.o: %.cpp %.h
	g++ -c -Wall -o $@ $<

clean:
	rm *.o