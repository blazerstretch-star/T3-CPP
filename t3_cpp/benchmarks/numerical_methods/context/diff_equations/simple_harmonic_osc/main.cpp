#include "equadiff.h"
#include "TApplication.h"
#include "TCanvas.h"
#include "TGraph.h"
#include "TAxis.h"
#include "TF1.h"



int main(int argc, char** argv){
    
    if (argc!=2){
        cerr << "Usage: " << argv[0] << " <step> " << endl;
        return -1;
    }
    double h = atof(argv[1]);
    
    //==============================================
    //        solution (Runge-kutta)
    //==============================================
    TApplication app ("app", 0, 0);
    TGraph traj ;
    TCanvas c ("c","Results", 1200, 800);
    c.Divide(2,2);

    simple_osc * myoscillator = new simple_osc (1.);
    runge_kutta myrunge;
    
    double tmax = 70.; 
    vector <double> x {0., 1.}; //boundary conditions
    double t = 0.;

    int i = 0;
    while (t<=tmax){
        traj.SetPoint(i, t, x[0]); 
        x = myrunge.step(x,t,h,myoscillator); 
        t += h; 
        i++;
    }

    c.cd(1);
    traj.GetXaxis()->SetTitle("t[s]");
    traj.GetYaxis()->SetTitle("x[m]");
    traj.Draw("APL");
    traj.SetTitle("trajectory");

        
    //==============================================
    //       Errore accumulato (runge vs eulero)
    //==============================================
    
    TGraph g_errors_runge, g_errors_eulero;
    eulero myeulero;
    
    double error_step = 0.1;
    const int n_punti = 10;
    vector <double> y {0., 1.}; //vettore per eulero

    for (int i=0; i< n_punti; ++i){
        cout << "step size: " << error_step << endl;
        t = 0;
        x[0] = 0.;
        x[1] = 1.;
        y[0] = 0.;
        y[1] = 1.;
        
        while (t<=tmax){ 
            x = myrunge.step(x, t, error_step,myoscillator);
            y = myeulero.step(x, t, error_step,myoscillator);
            t = t + error_step;
        }
        g_errors_runge.SetPoint(i, error_step, fabs( x[0] - sin(t) ) );
        g_errors_eulero.SetPoint(i, error_step, fabs( y[0] - sin(t) ) );
        error_step = error_step / 2;
    }
    
    //==========================================================
    //                    trend of error kh^4
    //=========================================================
    
    TF1 myfitfunction ("myfit", "[0]*pow(x,[1])",0.003125,0.1);
    myfitfunction.SetParameter(0,0.5);
    myfitfunction.SetParameter(1,4);

    g_errors_runge.Fit("myfit","R");

    c.cd(4);
    c.cd(4)->SetLogx();
    c.cd(4)->SetLogy();
    
    g_errors_runge.GetXaxis()->SetTitle("step [s]");
    g_errors_runge.GetYaxis()->SetTitle("error 70 s");
    g_errors_runge.SetTitle("error vs step size: Runge-Kutta");

    g_errors_runge.SetMarkerStyle(20);
    g_errors_runge.Draw();

    c.cd(3);
    c.cd(3)->SetLogx();
    c.cd(3)->SetLogy();
    
    g_errors_eulero.GetXaxis()->SetTitle("step [s]");
    g_errors_eulero.GetYaxis()->SetTitle("error 70 s");
    g_errors_eulero.SetTitle("error vs step size: Eulero");

    g_errors_eulero.SetMarkerStyle(20);
    g_errors_eulero.Draw();
    
    app.Run();
    return 0;
}
