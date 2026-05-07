#include "int_MC.h"
#include "TApplication.h"
#include "TH1F.h"
#include "TCanvas.h"
#include "TGraph.h"
#include <fstream>
#include <iostream>
#include <vector>

using namespace std;
int main (){
    
    double sigma = 0.01;
    double  n_points = 10000;
    int  n_integrals = 10000;
    f1* myfunction = new f1; 
    integral_mc* myint = new  integral_mc(2); 
    vector<double> a {1,2}; 
    vector<double> b {1,2};

    
    TApplication app ("myapp",0,0);
    vector <double> x(2);
    
    TH1F* h1 = new TH1F("h1", "integral distribution ", 100, 10, 11);
    for (int i = 0; i< n_integrals; ++i) h1->Fill(myint-> int_mean(a, b, myfunction,  n_points));
    int  N_given_precision = pow(h1->GetStdDev(),2) *  n_points / pow(sigma,2);
    h1->StatOverflows(kTRUE);
    
    cout << "to get a precision  " << sigma
         << " we need " <<  N_given_precision
         << " points" << endl;
    
    
    
    TCanvas* c = new TCanvas("c", "distribution with fixed precision con ", 600,400);
    TH1F* h = new TH1F("h", "integral distribution", 100, 10.2, 10.3);
    for (int i = 0; i< n_integrals; ++i) {
        h->Fill(myint-> int_mean(a, b, myfunction,  N_given_precision));
        if (i % 500 == 0) cout << "Ho calcolato " << i << " integrali" << endl;
    }
    h->StatOverflows(kTRUE);

    h->GetXaxis()->SetTitle("integral");
    h->GetYaxis()->SetTitle("occurrences");
    h->Draw();
    
    delete myfunction;
    delete myint;

    app.Run();
    return 0;
}
