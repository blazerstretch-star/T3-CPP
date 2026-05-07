#include "randomgen.h"
#include "TApplication.h"
#include "TH1F.h"
#include "TF1.h"
#include "TCanvas.h"
#include "TGraph.h"
#include <iomanip>

using namespace std;
int main (int argc, char** argv){
    
    if (argc!= 3) {
        cout <<"usage " << argv[0] << "<n starting points>, <n elements for each sum>"<< endl;
        cout << "suggested n starting points <= 1.000.000.000 " << endl;
        return 1;
    }
    TApplication app ("app", 0, 0);
    randomgen* mygen = new randomgen (1); 
    int N_bins = 100;
    int n_serie = atof(argv[1]); 
    int n_somma = atof(argv[2]); 
    int n_punti = n_serie /n_somma ; 
    
    TCanvas* c = new TCanvas ( "c" , "Distributions", 1200, 800 );
    c->Divide(2,2);
    
    TH1F* h1 = new TH1F ( "h1" , "Distribution sums", N_bins, 0, n_somma);
    TH1F* h2 = new TH1F ( "h2" , "Distribution numbers", N_bins, 0.,1. );
    
    h1->StatOverflows(kTRUE);
    h2->StatOverflows(kTRUE);
    
    double sum = 0.;
    
    for (int k = 0; k< n_punti;k++){
        sum = 0.;
        for (int i = 0; i< n_somma; i++){ 
            double temp = mygen->rand();
            h2->Fill(temp);
            sum = sum + temp;
        }
        h1->Fill(sum);
    }
    h1->GetXaxis()->SetRangeUser(h1->GetMean()-4*h1->GetStdDev(),h1->GetMean()+4*h1->GetStdDev());
    
    
    TGraph* errore = new TGraph;
    TH1F* h_test = new TH1F ( "htest" , "Distribution sums test", N_bins, 0.,1. );
    h_test->StatOverflows(kTRUE);
    
    double  n_sum_error = 15;
    for(int j = 0; j<10; j++){ 
        for (int k = 0; k<n_punti; k++){ 
            sum = 0.;
            for (int i = 0; i<  n_sum_error; i++){ 
                double temp = mygen->rand();
                sum = sum + temp;
            }
            h_test->Fill(sum);
        }
        errore->SetPoint(j,  n_sum_error, pow(h_test->GetStdDev(),2));
         n_sum_error*=1.1;
        h_test->Reset();
    }
    
    
    TF1* myfitfunction=new TF1 ("fitf", "pol1", 0,  n_sum_error);
    errore->Fit("fitf");
    double p0 = myfitfunction->GetParameter(0);
    double p1 = myfitfunction->GetParameter(1);
    cout << endl << endl;
    cout << "trend observed error : " << setprecision(3) << p0 << " + "  <<p1 << " N"<< endl;
    cout << "trend expected error : " << setprecision(3) << pow(h2->GetStdDev(),2) << " N"<< endl;
    
    c->cd(3);
    errore->GetXaxis()->SetTitle("# elements for sum");
    errore->GetYaxis()->SetTitle("variance of sum");
    errore->SetMarkerStyle(20);
    errore->SetTitle("trend of error with N");
    errore->Draw("APL");

    c->cd(1);
    h1->GetXaxis() -> SetTitle ( "x [u.a.]" );
    h1->GetYaxis() -> SetTitle ( "y [u.a.]" );
    h1->Draw();
    
    c->cd(2);
    h2->GetXaxis() -> SetTitle ( "x [u.a.]" );
    h2->GetYaxis() -> SetTitle ( "y [u.a.]" );
    h2->Draw();
    
    
    
    app.Run();
    
    return 0;
}
