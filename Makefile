OBJS = asembler.o parser.o symboltable.o symboltableentry.o forwardreference.o section.o incalculable.o expression.o globalsymbol.o exceptions.o

asembler: $(OBJS)
	g++ -o $@ $(OBJS)

%.o: %.cpp %.h
	g++ -c -o $@ $<

clean:
	rm *.o