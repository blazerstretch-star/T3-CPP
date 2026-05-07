LIBS:=`root-config --libs`
INCS:=`root-config --cflags`

main: main.o func.o integral.o
	g++ main.o func.o integral.o ${LIBS} -o main

main.o: main.cpp func.h integral.h
	g++ -c main.cpp -o main.o ${INCS} 

%.o: %.cpp %.h
	g++ -c $< -o $@

clean:
	rm *.o
	rm main
