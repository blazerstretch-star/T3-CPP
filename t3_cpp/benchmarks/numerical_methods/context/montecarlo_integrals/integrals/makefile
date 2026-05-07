LIBS:=`root-config --libs`
INCS:=`root-config --cflags`

main: main.o randomgen.o int_MC.o func.o
	g++ -std=c++11 main.o randomgen.o int_MC.o func.o ${LIBS} -o main

main_analysis: main_analysis.o
	g++ -std=c++11 main_analysis.o ${LIBS} -o main_analysis

main.o: main.cpp randomgen.h int_MC.h func.h
	g++ -c -std=c++11 main.cpp -o main.o ${INCS} 

main_analysis.o: main_analysis.cpp
	g++ -c -std=c++11 main_analysis.cpp -o main_analysis.o ${INCS}

%.o: %.cpp %.h
	g++ -c -std=c++11 $< -o $@

clean:
	rm *.o
	rm main
	rm main_analysis
