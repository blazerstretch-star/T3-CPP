#include <iostream>
#include <cmath>
#include <iomanip>

#include "func.h"
#include "solver.h"

using namespace std;

int main (){

    
    double precision = pow(10,-6);
    double n_iter = 1000;
    int prec = -log10 ( precision );

	f_tan* myfunction = new f_tan;
	bisection* mysolver = new bisection(n_iter, prec); 


    for (int i = 1; i<=20; i++){
	cout << "solution in (" << i <<"π, "<<i << "π + π/2 ):  " << setprecision(prec) << mysolver->root_finder(i*M_PI, i*M_PI+M_PI/2, myfunction) << endl;
    }
    delete myfunction;
    delete mysolver;
    
	return 0;
}
