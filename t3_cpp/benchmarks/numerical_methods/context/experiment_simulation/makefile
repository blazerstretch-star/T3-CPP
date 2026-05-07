LIBS:=`root-config --libs`
INCS:=`root-config --cflags`

main: main.o randomgen.o prism.o
	g++ -std=c++11 main.o randomgen.o prism.o ${LIBS} -o main

main.o: main.cpp randomgen.h prism.h
	g++ -c -std=c++11 main.cpp -o main.o ${INCS} 

%.o: %.cpp %.h
	g++ -c -std=c++11 $< -o $@

clean:
	rm *.o
	rm main
