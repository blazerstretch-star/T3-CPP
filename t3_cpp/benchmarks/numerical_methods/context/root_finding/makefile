main: main.o func.o solver.o
	g++ $^ -o $@

main.o: main.cpp func.h solver.h
	g++ -c $< -o $@

%.o: %.cpp %.h
	g++ -c $< -o $@

clean:
	rm *.o
	rm main
