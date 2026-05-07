#include "solver.h"


int sign (double x){
    // FUNCTION_ID: nummethod_func001 - START
	if (x < 0) return -1;
	else return 1;
    // FUNCTION_ID: nummethod_func001 - END
}

void bisection :: SetN (unsigned int N){
    n_iter = N;
}


double bisection::root_finder(double a, double b, const funcbase* f){
    // FUNCTION_ID: nummethod_func002 - START
    if (a > b) {
        double temp = a;
        a =  b;
        b = temp;
    }
double y_a = f->eval(a);
double y_b = f->eval(b);
double c = a + 0.5 * (b-a); 
double y_c = f->eval(c);  
int i = 0; 
    
if (sign (y_a) * sign(y_b) > 0 ) {
            cerr << "no unique solution in the given interval " << endl;
            exit(1);
        }

if (c == 0) return c;   
while (abs (a-b)>=get_precision() || i<n_iter ){ 
	
    if (sign (y_a) * sign(y_c) == -1){
		b = c;
		y_a = f->eval(a);
		y_b = f->eval(b);
        
		c = a + 0.5 * (b-a);
		y_c = f->eval(c);
	}
	else { 
		a = c;
		y_a = f->eval(a);
		y_b = f->eval(b);
        
		c = a + 0.5 * (b-a);
		y_c = f->eval(c);
    }
	i++;
	}
	
 return c;
    // FUNCTION_ID: nummethod_func002 - END
}
