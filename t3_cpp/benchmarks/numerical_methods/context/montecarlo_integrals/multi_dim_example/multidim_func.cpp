#include "multidim_func.h"

double f1 :: eval (const vector<double>& x) const{ //x[0] = x, x[1] = y
    return (5 * cos(4*x[1]) * sin(7*x[0])) / pow(x[0],2) + 10;
}

