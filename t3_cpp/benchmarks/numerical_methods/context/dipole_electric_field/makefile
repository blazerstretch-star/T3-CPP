LIBS:=`root-config --libs`
INCS:=`root-config --cflags`

main: main.o particle.o position.o vector_field.o material_point.o
	g++ -std=c++11 main.o particle.o position.o vector_field.o material_point.o ${LIBS} -o main

main.o: main.cpp particle.h position.h vector_field.h material_point.h
	g++ -c -std=c++11 main.cpp -o main.o ${INCS} 

%.o: %.cpp %.h
	g++ -c -std=c++11 $< -o $@

clean:
	rm *.o
	rm main
