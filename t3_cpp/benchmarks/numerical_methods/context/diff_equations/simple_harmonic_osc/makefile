LIBS:=`root-config --libs`
INCS:=`root-config --cflags`

main: main.o equadiff.o fvect.o
	g++ -std=c++11 main.o equadiff.o fvect.o ${LIBS} -o main

main.o: main.cpp equadiff.h fvect.h
	g++ -c -std=c++11 main.cpp -o main.o ${INCS} 

%.o: %.cpp %.h myalgebra.h
	g++ -c -std=c++11 $< -o $@

clean:
	rm *.o
	rm main
