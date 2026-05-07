#include "TApplication.h"
#include "TH1F.h"
#include "TF1.h"

#include "TCanvas.h"
#include "TGraph.h"
#include <fstream>
#include <iostream>
using namespace std;

int main(){

    //    =========================================================================
    //                                 HIT OR MISS
    //    =========================================================================

    double temp = 0.;
    int n_integrals = 10000; 
    ifstream f_10E2HOM ("data/f_10E2HOM.dat");
    ifstream f_10E3HOM ("data/f_10E3HOM.dat");
    ifstream f_10E4HOM ("data/f_10E4HOM.dat");
    ifstream f_10E5HOM ("data/f_10E5HOM.dat");
    ifstream f_10E6HOM ("data/f_10E6HOM.dat");
    if( !f_10E2HOM.good() || !f_10E3HOM.good() || !f_10E4HOM.good() || !f_10E5HOM.good() || !f_10E6HOM.good() ){
        cerr << "error while opening the file " << endl;
        return 1;
    }
    TApplication app ("myapp", 0, 0);
    TCanvas* c = new TCanvas ("c", "Hit or Miss", 1800,1200);
    c->Divide(2,3);

    TH1F* h1 = new TH1F ("h1","distribution with  100 points",100,1.5,2.5);
    TH1F* h2 = new TH1F("h2","distribution with  1000 points",100,1.8,2.2);
    TH1F* h3 = new TH1F("h3","distribution with  10000 points",100,1.93,2.07);
    TH1F* h4 = new TH1F("h4","distribution with  100000 points",100,1.98,2.02);
    TH1F* h5 = new TH1F("h5","distribution with  1000000 points",100,1.99,2.01);
    
    
    h1->StatOverflows(kTRUE);
    h2->StatOverflows(kTRUE);
    h3->StatOverflows(kTRUE);
    h4->StatOverflows(kTRUE);
    h5->StatOverflows(kTRUE);

    for (int i = 0; i<n_integrals; ++i ){
        f_10E2HOM >> temp;
        h1->Fill(temp);
        f_10E3HOM >> temp;
        h2->Fill(temp);
        f_10E4HOM >> temp;
        h3->Fill(temp);
        f_10E5HOM >> temp;
        h4->Fill(temp);
        f_10E6HOM >> temp;
        h5->Fill(temp);
    }
        c->cd(1);
        h1->GetXaxis()->SetTitle("integral value");
        h1->GetYaxis()->SetTitle("# integrals");
        h1->Draw();
   
        c->cd(2);
        h2->GetXaxis()->SetTitle("integral value");
        h2->GetYaxis()->SetTitle("# integrals");
        h2->Draw();
        
        c->cd(3);
        h3->GetXaxis()->SetTitle("integral value");
        h3->GetYaxis()->SetTitle("# integrals");
        h3->Draw();
    
        c->cd(4);
        h4->GetXaxis()->SetTitle("integral value");
        h4->GetYaxis()->SetTitle("# integrals");
        h4->Draw();
    
        c->cd(5);
        h5->GetXaxis()->SetTitle("integral value");
        h5->GetYaxis()->SetTitle("# integrals");
        h5->Draw();
    
        f_10E2HOM.close();
        f_10E3HOM.close();
        f_10E4HOM.close();
        f_10E5HOM.close();
        f_10E6HOM.close();
        
        TCanvas* c1 = new TCanvas ("c1", "error trend", 1200,400);
        c1->Divide(2,1);
        c1->cd(1);
        c1->cd(1)->SetLogx();

        TGraph* errors_HOM = new TGraph;
        errors_HOM->SetTitle("Metodo Hit or Miss");
        errors_HOM->SetPoint(0, 100, h1->GetStdDev());
        errors_HOM->SetPoint(1, 1000, h2->GetStdDev());
        errors_HOM->SetPoint(2, 10000, h3->GetStdDev());
        errors_HOM->SetPoint(3, 100000, h4->GetStdDev());
        errors_HOM->SetPoint(4, 1000000, h5->GetStdDev());
    
        errors_HOM->GetXaxis()->SetTitle("Numero di points");
        errors_HOM->GetYaxis()->SetTitle("error");
        errors_HOM->SetMarkerStyle(20);
        errors_HOM->Draw("APL");

    
    //    FIT
    TF1* f1 = new TF1("f1","[0]/pow(x,0.5)",0.,1000000.);
    f1->SetParameter(0,1);
    errors_HOM->Fit("f1");
    f1->Draw("same");
    double k_HOM = f1->GetParameter(0);
    double sigma_HOM = 0.001;
    int n_points_originari = 10000;
    int N_points_necessari_HOM = static_cast<int>( pow(k_HOM,2)/pow(sigma_HOM,2));
    
    cout << "HoM requires " << N_points_necessari_HOM << " to get a precision of " << sigma_HOM << endl;


     
    //    =========================================================================
    //                                  MEAN
    //    =========================================================================

        temp = 0.;

        ifstream f_10E2Mean ("data/f_10E2Mean.dat");
        ifstream f_10E3Mean ("data/f_10E3Mean.dat");
        ifstream f_10E4Mean ("data/f_10E4Mean.dat");
        ifstream f_10E5Mean ("data/f_10E5Mean.dat");
        ifstream f_10E6Mean ("data/f_10E6Mean.dat");
        if( !f_10E2Mean.good() || !f_10E3Mean.good() || !f_10E4Mean.good() || !f_10E5Mean.good() || !f_10E6Mean.good() ){
            cerr << "error while opening the file " << endl;
            return 1;
        }
        TH1F* k1 = new TH1F("k1","distribution with  100 points",100,1.4,2.6);
        TH1F* k2 = new TH1F("k2","distribution with  1000 points",100,1.8,2.2);
        TH1F* k3 = new TH1F("k3","distribution with  10000 points",100,1.95,2.05);
        TH1F* k4 = new TH1F("k4","distribution with  100000 points",100,1.985,2.02);
        TH1F* k5 = new TH1F("k5","distribution with  1000000 points",100,1.996,2.004);
        
        k1->StatOverflows(kTRUE);
        k2->StatOverflows(kTRUE);
        k3->StatOverflows(kTRUE);
        k4->StatOverflows(kTRUE);
        k5->StatOverflows(kTRUE);
    
        for (int i = 0; i<n_integrals; ++i ){
            f_10E2Mean >> temp;
            k1->Fill(temp);
            f_10E3Mean >> temp;
            k2->Fill(temp);
            f_10E4Mean >> temp;
            k3->Fill(temp);
            f_10E5Mean >> temp;
            k4->Fill(temp);
            f_10E6Mean >> temp;
            k5->Fill(temp);
        }
            TCanvas* c2 = new TCanvas ("c2", "mean", 1800,1200);
            
            c2->Divide(3,2);
            c2->cd(1);

            k1->GetXaxis()->SetTitle("integral value");
            k1->GetYaxis()->SetTitle("# integrals");
            k1->Draw();
        
            c2->cd(2);
            k2->GetXaxis()->SetTitle("integral value");
            k2->GetYaxis()->SetTitle("# integrals");
            k2->Draw();
            
            c2->cd(3);
            k3->GetXaxis()->SetTitle("integral value");
            k3->GetYaxis()->SetTitle("# integrals");
            k3->Draw();
        
            c2->cd(4);
            k4->GetXaxis()->SetTitle("integral value");
            k4->GetYaxis()->SetTitle("# integrals");
            k4->Draw();
        
            c2->cd(5);
            k5->GetXaxis()->SetTitle("integral value");
            k5->GetYaxis()->SetTitle("# integrals");
            k5->Draw();
        
            f_10E2Mean.close();
            f_10E3Mean.close();
            f_10E4Mean.close();
            f_10E5Mean.close();
            f_10E6Mean.close();
            
        //    error DELL'error
            TGraph *errors_mean = new TGraph;
            errors_mean->SetTitle("mean method");
            c1->cd(2)->SetLogx();

            errors_mean->SetPoint(0, 100, k1->GetStdDev());
            errors_mean->SetPoint(1, 1000, k2->GetStdDev());
            errors_mean->SetPoint(2, 10000, k3->GetStdDev());
            errors_mean->SetPoint(3, 100000, k4->GetStdDev());
            errors_mean->SetPoint(4, 1000000, k5->GetStdDev());
        
            errors_mean->GetXaxis()->SetTitle("# points");
            errors_mean->GetYaxis()->SetTitle("error");
            errors_mean->SetMarkerStyle(20);
            c1->cd(2);
            errors_mean->Draw("APL");
        
        //    FIT
        TF1* f2 = new TF1("f2","[0]/pow(x,0.5)",0.,1000000.);
        f2->SetParameter(0,1);
        errors_mean->Fit("f2");
        f2->Draw("same");
        double k_mean = f2->GetParameter(0);
        
        double sigma_mean = 0.001; 
        int N_points_necessari_mean = static_cast<int>( pow(k_mean,2)/pow(sigma_mean,2));
    
        cout << "mean method requires " << N_points_necessari_mean << " for a precision of  " << sigma_mean << endl;
    
    app.Run();
    
    return 0;
}
