#include "equadiff.h"
#include "TApplication.h"
#include "TCanvas.h"
#include "TGraph.h"
#include "TAxis.h"

int main(int argc, char** argv){
    
    if (argc!=2){
        cerr << "Usage: " << argv[0] << " <step size> " << endl;
        return -1;
    }
    double h = atof(argv[1]);
    
    //==============================================
    //        solution of the equation
    //==============================================
    
    TApplication app ("app", 0, 0);
    TGraph *trajectory = new TGraph;
    TCanvas* c = new TCanvas ("c","Results", 1200, 400);


    oscillator * myoscillator = new oscillator (10., 10., 1./30.); //oscillator(omega, omega0, gamma)
    runge_kutta* myrunge = new runge_kutta;
    
    double tmax = 10 * (1/myoscillator->GetGamma()); // we integrate for at least 1/gamma to stabilize
    vector <double> x {0., 0.}; //boundary conditions
    double t = 0.;
    
    
    int i = 0;
    while (t<=tmax/10){ 
        trajectory -> SetPoint(i, t, x[0]); 
        x = myrunge->step(x,t,h,myoscillator); 
        t = t + h; 
        i++;
    }
    
    c->Divide(2,1);
    c->cd(1);
    trajectory->GetXaxis()->SetTitle("t[s]");
    trajectory->GetYaxis()->SetTitle("x[m]");
    trajectory->Draw();
    trajectory->SetTitle("Harmonic oscillator");
    

    //==============================================
    //                resonance
    //==============================================
    
    TGraph *lorentz = new TGraph;

    t = 0.;
    const int n_punti = 30;
    double omega = 9.15; 
    
    for (int i = 0; i<n_punti; ++i){
        omega = omega + 0.05;
        myoscillator->SetOmega(omega); 
        x[0] = 0.;
        x[1] = 0.; 
        t = 0.;
    
        while (t<=tmax ){ 
            x=myrunge->step (x,t,h,myoscillator);
            t+=h; 
        }
        if (x[1]>=0.){
            while ( x[1]>= 0.){
                x=myrunge->step (x,t,h,myoscillator);
                t+=h;
            }
        }
        else { 
            while ( x[1]<=0. ){
                x=myrunge->step (x,t,h,myoscillator);
                t+=h;
            }
        }
        lorentz -> SetPoint (i, myoscillator->GetOmega(), fabs(x[0]) );
        
    }
    
    c->cd(2);
    lorentz->GetXaxis()->SetTitle("omega [rad/s]");
    lorentz->GetYaxis()->SetTitle("amplitude [m]");
    lorentz->Draw("APL");
    lorentz->SetMarkerStyle(20);
    lorentz->SetTitle("resonance curve");
    
        
    delete myoscillator;
    delete myrunge;
    
    app.Run();
    
    
    return 0;
}
