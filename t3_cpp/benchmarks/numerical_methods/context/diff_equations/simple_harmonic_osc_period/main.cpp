#include "equadiff.h"
#include "TApplication.h"
#include "TCanvas.h"
#include "TGraph.h"
#include "TAxis.h"



int main(int argc, char** argv){
    
    if (argc!=2){
        cerr << "Usage : " << argv[0] << " <step> " << endl;
        return -1;
    }
    double h = atof(argv[1]);
    
    
    TApplication app ("app", 0, 0);
    TGraph* traj = new TGraph;
    TCanvas* c = new TCanvas ("c","Results", 1200, 400);
    c->Divide(2,1);
    
    pendulum* mypend = new pendulum (1.);
    runge_kutta* myrunge = new runge_kutta;

    
    double tmax = 70.; 
    vector <double> x {0., 1.}; 
    double t = 0.;
    
    int i = 0;
    while (t<=tmax){
        traj->SetPoint(i, t, x[0]); 
        x = myrunge->step(x,t,h,mypend); 
        t = t + h; 
        ++i;
    }
    
    c->cd(1);
    traj->GetXaxis()->SetTitle("t[s]");
    traj->GetYaxis()->SetTitle("x[rad]");
    traj->Draw("APL");
    traj->SetTitle("trajectory");
    
    
    //==============================================
    //   plot for each amplitude
    //==============================================
    
    TGraph *periodo = new TGraph;
    
    int N = 35;
    double step = 0.0001;
    double amplitude = 0.5;
    double v_old = 0.;
    
    for (int i = 0; i<N; i++){
        x[0] = - amplitude  ; 
        x[1] = 0.;
        t = 0.;
        
        while (x[1]>=0.){
            v_old = x[1]; 
            x = myrunge->step (x,t,step,mypend); 
            t = t + step;
            }
        
        periodo -> SetPoint(i, x[0], t-h-v_old*step/(x[1]-v_old));
        amplitude = amplitude + 0.07; 
    }
    
    c->cd(2);

    periodo->SetMarkerStyle(20);
    periodo->GetXaxis()->SetTitle("A[rad]");
    periodo->GetYaxis()->SetTitle("T[s]");
    periodo->Draw("APL");
    periodo->SetTitle("Period");
    
  
    delete mypend;
    delete myrunge;
    
    app.Run();
    
    
    return 0;
}
