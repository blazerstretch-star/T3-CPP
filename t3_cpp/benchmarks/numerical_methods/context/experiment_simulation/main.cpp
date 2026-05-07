#include "prism.h"
#include "TApplication.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TAxis.h"
#include "TCanvas.h"
using namespace std;
int main(){
    
    
    vector <double> values_A(10000);
    vector <double> values_B(10000);
    vector <double> values_theta0(10000);
    vector <double> values_theta1(10000);
    vector <double> values_theta2(10000);
    vector <double> values_n1(10000);
    vector <double> values_n2(10000);
    vector <double> values_delta1(10000);
    vector <double> values_delta2(10000);
    
    prism experiment;
    
    for (int i = 0; i < 10000; ++i){
        experiment.execute();
        experiment.analyze();
        values_A[i] = experiment.get_A();
        values_B[i] = experiment.get_B();
        values_theta0[i] = experiment.get_th0();
        values_theta1[i] = experiment.get_th1();
        values_theta2[i] = experiment.get_th2();
        values_delta1[i] = experiment.get_delta1();
        values_delta2[i] = experiment.get_delta2();
        values_n1[i] = experiment.get_n1();
        values_n2[i] = experiment.get_n2();
    }
    
    TApplication app ("myapp", 0, 0);
    TCanvas c("c", "values distribution", 1800,1800);
    c.Divide(3,3);
    
    //GRAFICI DEI values MISURATI
    TH1F* ha = new TH1F ("h","distribution A", 100, 1.648, 1.643 );
    TH1F* hb = new TH1F ("hb","distribution  B", 100, 5.9E-14, 6.1E-14 );
    TH1F* ht0 =  new TH1F ("ht0","distribution theta 0", 100, 1.573, 1.569 );
    TH1F* ht1 = new TH1F ("ht1","distribution theta 1 ", 100, 2.550, 2.547 );
    TH1F* ht2 = new TH1F ("ht2","distribution theta 2", 100, 2.659, 2.655 );
    TH1F* hn1 = new TH1F ("hn1","distribution n1", 100, 1.700, 1.695 );
    TH1F* hn2 = new TH1F ("hn2","distribution n2", 100, 1.750, 1.752 );
    TH1F* hdelta1 = new TH1F ("hdelta1","distribution delta 1", 100, 0.9789, 0.9783 );
    TH1F* hdelta2 = new TH1F ("hdelta2","distribution delta 2", 100, 0.9789, 0.9783 );
    
    ha->StatOverflows(kTRUE);
    hb->StatOverflows(kTRUE);
    ht0->StatOverflows(kTRUE);
    ht1->StatOverflows(kTRUE);
    ht2->StatOverflows(kTRUE);
    hn1->StatOverflows(kTRUE);
    hn2->StatOverflows(kTRUE);
    hdelta1->StatOverflows(kTRUE);
    hdelta2->StatOverflows(kTRUE);


    
    for (int i = 0; i< 10000; ++i){
        ha->Fill(values_A[i]);
        hb->Fill(values_B[i]);
        ht0->Fill(values_theta0 [i]);
        ht1->Fill(values_theta1[i]);
        ht2->Fill(values_theta2[i]);
        hn1->Fill(values_n1[i]);
        hn2->Fill(values_n2[i]);
        hdelta1->Fill(values_delta1[i]);
        hdelta2->Fill(values_delta2[i]);
    }
    
    ha->GetXaxis()->SetRangeUser(ha->GetMean()-4*ha->GetStdDev(),ha->GetMean()+4*ha->GetStdDev() );
    hb->GetXaxis()->SetRangeUser(hb->GetMean()-4*hb->GetStdDev(),hb->GetMean()+4*hb->GetStdDev() );
    ht0->GetXaxis()->SetRangeUser(ht0->GetMean()-4*ht0->GetStdDev(),ht0->GetMean()+4*ht0->GetStdDev() );
    ht1->GetXaxis()->SetRangeUser(ht1->GetMean()-4*ht1->GetStdDev(),ht1->GetMean()+4*ht1->GetStdDev() );
    ht2->GetXaxis()->SetRangeUser(ht2->GetMean()-4*ht2->GetStdDev(),ht2->GetMean()+4*ht2->GetStdDev() );
    hn1->GetXaxis()->SetRangeUser(hn1->GetMean()-4*hn1->GetStdDev(),hn1->GetMean()+4*hn1->GetStdDev() );
    hn2->GetXaxis()->SetRangeUser(hn2->GetMean()-4*hn2->GetStdDev(),hn2->GetMean()+4*hn2->GetStdDev() );
    hdelta1->GetXaxis()->SetRangeUser(hdelta1->GetMean()-4*hdelta1->GetStdDev(),hdelta1->GetMean()+4*hdelta1->GetStdDev() );
    hdelta2->GetXaxis()->SetRangeUser(hdelta2->GetMean()-4*hdelta2->GetStdDev(),hdelta2->GetMean()+4*hdelta2->GetStdDev() );
    
    
    c.cd(1);
    ha->Draw();
    ha->GetYaxis()->SetTitle("value of A");
    
    c.cd(2);
    hb->Draw();
    hb->GetYaxis()->SetTitle("value of B");
    hb->GetXaxis()->SetTitle("[m^2]");
    
    c.cd(3);
    ht0->Draw();
    ht0->GetYaxis()->SetTitle("value oftheta 0");
    ht0->GetXaxis()->SetTitle("[rad]");
    
    c.cd(4);
    ht1->Draw();
    ht1->GetYaxis()->SetTitle("value of theta 1");
    ht1->GetXaxis()->SetTitle("[rad]");
    
    c.cd(5);
    ht2->Draw();
    ht2->GetYaxis()->SetTitle("value of theta 2");
    ht2->GetXaxis()->SetTitle("[rad]");
    
    c.cd(6);
    hn1->Draw();
    hn1->GetXaxis()->SetTitle("value of n1");

    c.cd(7);
    hn2->Draw();
    hn2->GetXaxis()->SetTitle("value ofn2");

    c.cd(8);
    hdelta1->Draw();
    hdelta1->GetYaxis()->SetTitle("value of delta 1");
    hdelta1->GetXaxis()->SetTitle("[rad]");

    c.cd(9);
    hdelta1->Draw();
    hdelta2->GetYaxis()->SetTitle("value of delta 2");
    hdelta2->GetXaxis()->SetTitle("[rad]");
    
    cout << "results: " << endl << endl
        << "A = " << ha->GetMean() << " +/- " << ha->GetStdDev() << endl
        << "B = " << hb->GetMean() << " +/- " << hb->GetStdDev() << endl
        << "n1 = " << hn1->GetMean() << " +/- " << hn1->GetStdDev() << endl
        << "n2 = " << hn2->GetMean() << " +/- " << hn2->GetStdDev() << endl
        << "theta 1 = " << ht1->GetMean() << " +/- " << ht1->GetStdDev() << endl
        << "theta 2 = " << ht2->GetMean() << " +/- " << ht2->GetStdDev() << endl
        << "theta 0 = " << ht0->GetMean() << " +/- " << ht0->GetStdDev() << endl
        << "delta 1 = " << hdelta1->GetMean() << " +/- " << hdelta1->GetStdDev() << endl
        << "delta 2 = " << hdelta2->GetMean() << " +/- " << hdelta2->GetStdDev() << endl;
    
    
    
//    ================================================================================
    TH2F* delta_correlation = new TH2F ("dbi","residues delta", 100, -0.002, 0.002, 100, -0.002, 0.002 );
    TH2F* n_correlation = new TH2F ("nbi","residues n", 100, -0.001, 0.001, 100, -0.001, 0.001 );
    TH2F* ab_correlation =  new TH2F("ab","residues A, B", 100, -0.004, 0.004, 100, -0.8E-15, 0.8E-15 );
    TH1F* diffn1 = new TH1F ("diffn1","distribution differences between the expected and the measured value: n1", 100, 0.001, 0.001 );
    TH1F* diffn2 = new TH1F ("diffn2","distribution differences between the expected and the measured value: n2", 100, -0.001, 0.001 );
    TH1F* diffa =  new TH1F ("sa","distribution differences between the expected and the measured value: A ", 100, 0.001, 0.001 );
    TH1F* diffb = new TH1F ("sb","distribution differences between the expected and the measured value: B", 100, 0.001, 0.001 );
    TH1F* diffd1 =  new TH1F ("sd1","distribution differences between the expected and the measured value: delta 1", 100, 0.001, 0.001 );
    TH1F* diffd2 = new TH1F ("sd2","distribution differences between the expected and the measured value: delta 2", 100, 0.001, 0.001 );
    

    delta_correlation->StatOverflows(kTRUE);
    n_correlation->StatOverflows(kTRUE);
    ab_correlation->StatOverflows(kTRUE);
    diffn1->StatOverflows(kTRUE);
    diffn2->StatOverflows(kTRUE);
    diffa->StatOverflows(kTRUE);
    diffb->StatOverflows(kTRUE);
    diffd1->StatOverflows(kTRUE);
    diffd2->StatOverflows(kTRUE);
    
    for(int i = 0; i< 10000;i++){
        ab_correlation->Fill(values_A[i]-experiment.get_A_real(), values_B[i]-experiment.get_B_real() );
        n_correlation->Fill(values_n1[i]-experiment.get_n1_real(), values_n2[i]-experiment.get_n2_real() );
        delta_correlation->Fill(values_delta1[i] - experiment.get_delta1_real(), values_delta2[i]-experiment.get_delta2_real() );
        diffn1->Fill(values_n1[i] - experiment.get_n1_real());
        diffn2->Fill(values_n2[i] - experiment.get_n2_real());
        diffa->Fill(values_A[i] - experiment.get_A_real());
        diffb->Fill(values_B[i] - experiment.get_B_real());
        diffd1->Fill(values_delta1[i] - experiment.get_delta1_real());
        diffd2->Fill(values_delta1[i] - experiment.get_delta2_real());

    }
    
       diffn1->GetXaxis()->SetRangeUser(diffn1->GetMean()-4*diffn1->GetStdDev(),diffn1->GetMean()+4*diffn1->GetStdDev() );
       diffn2->GetXaxis()->SetRangeUser(diffn2->GetMean()-4*diffn2->GetStdDev(),diffn2->GetMean()+4*diffn2->GetStdDev() );
       diffa->GetXaxis()->SetRangeUser(diffa->GetMean()-4*diffa->GetStdDev(),diffa->GetMean()+4*diffa->GetStdDev() );
       diffb->GetXaxis()->SetRangeUser(diffb->GetMean()-4*diffb->GetStdDev(),diffb->GetMean()+4*diffb->GetStdDev() );
       diffd1->GetXaxis()->SetRangeUser(diffd1->GetMean()-4*diffd1->GetStdDev(),diffd1->GetMean()+4*diffd1->GetStdDev() );
       diffd2->GetXaxis()->SetRangeUser(diffd2->GetMean()-4*diffd2->GetStdDev(),diffd2->GetMean()+4*diffd2->GetStdDev() );
      
    TCanvas c1("c1", "residues", 1800,1800);

    c1.Divide(3,3);
    c1.cd(1);
    ab_correlation->Draw("CONT0");
    ab_correlation->GetYaxis()->SetTitle("errors B [m^2]");
    ab_correlation->GetXaxis()->SetTitle("errors A");
    
    c1.cd(2);
    n_correlation->Draw("CONT0");
    n_correlation->GetYaxis()->SetTitle("errors n2");
    n_correlation->GetXaxis()->SetTitle("errors n1");
    
    c1.cd(3);
    delta_correlation->Draw("CONT0");
    delta_correlation->GetYaxis()->SetTitle("errors delta 2 [rad]");
    delta_correlation->GetXaxis()->SetTitle("errors delta 1 [rad]");
    
    c1.cd(4);
    diffn1->Draw();
    diffn1->GetXaxis()->SetTitle("errors n1");
    
    c1.cd(5);
    diffn2->Draw();
    diffn2->GetXaxis()->SetTitle("errors n2");

    c1.cd(6);
    diffa->Draw();
    diffa->GetXaxis()->SetTitle("errors a");

    c1.cd(7);
    diffb->Draw();
    diffb->GetXaxis()->SetTitle("errors b [m^2]");

    c1.cd(8);
    diffd1->Draw();
    diffd1->GetXaxis()->SetTitle("errors delta 1 [rad]");

    c1.cd(9);
    diffd2->Draw();
    diffd2->GetXaxis()->SetTitle("errors delta 2 [rad]");

    cout << endl << endl << endl << endl << "correlation: " << endl << endl
         << "A - B: " << ab_correlation->GetCorrelationFactor() << endl
         << "n1 - n2: " << n_correlation->GetCorrelationFactor() << endl
         << "delta1 - delta2: " << delta_correlation->GetCorrelationFactor() << endl;

    app.Run();
    return 0;
}
