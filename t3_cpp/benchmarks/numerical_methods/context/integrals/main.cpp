#include "integral.h"
#include "TApplication.h"
#include "TGraph.h"
#include <iomanip>
#include "TCanvas.h"

int main(int argc, char** argv){
    if (argc!= 2){
        cerr << "Usage " << argv[0] << " <precision> " << endl;
        return 1;
    }
    
    fsin* mys = new fsin;
    double precision = atof(argv[1]);

    integral* myint = new integral(0, M_PI, mys, precision);
    cout << "Integral of sin in (0, π): "
         << fixed << setprecision(-log10(precision))
         << myint->trapezoidal() <<" +/- " << precision<< endl;

    delete mys;
    delete myint;

	return 0;
}
