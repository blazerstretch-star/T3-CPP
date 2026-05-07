#include "int_MC.h"
#include "TApplication.h"
#include "TH1F.h"
#include "TCanvas.h"
#include "TGraph.h"
#include <fstream>
#include <iostream>
#include <string>

using namespace std;
int main (){
 
    
    fsin* mysin = new fsin; 
    int n_punti= 100;
    int n_integrali = 10000;
    integral_mc* myint = new integral_mc(1); 
    
    for(int k = 2; k<=6; k++ ){
        string filename_HOM = "data/f_10E"+  to_string(k) + "HOM.dat";
        ofstream out_HOM (filename_HOM);
        
        string filename_mean = "data/f_10E"+  to_string(k) + "Mean.dat";
        ofstream out_mean (filename_mean);
        
        if( !out_HOM.good() || !out_mean.good() ){
            cerr << "error opening file" << endl;
            return -1;
        }
        
        cout << endl << "computing with " << n_punti << endl;

        for (int i = 0; i< n_integrali; i++){
            out_HOM << myint->int_HoM(0, M_PI, 1, mysin, n_punti) << endl;
            out_mean << myint->int_mean(0, M_PI, mysin, n_punti) << endl;

               if (i % 1000 == 0) cout << "computed " << i << " integrals" << endl;
               }
        
        n_punti = n_punti*10;
        out_HOM.close();
        out_mean.close();
    }

    delete mysin;
    delete myint;
    return 0;
}
