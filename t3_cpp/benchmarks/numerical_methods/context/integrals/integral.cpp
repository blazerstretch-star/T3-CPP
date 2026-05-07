#include "integral.h"

integral:: integral (double a, double b, const funcbase* f){
    m_a = a;
    m_b = b;
    m_f = f;
    if (a>b) m_sign = -1;
    else m_sign = 1;

}

integral::integral (double a, double b, const funcbase* f, double prec){
    m_a = a;
    m_b = b;
    m_f = f;
    if (a>b) m_sign = -1;
    else m_sign = 1;
    precision = prec;
}


//======================================================
//             midpoint fixed number of steps
//======================================================
double integral::midpoint (int N){
    // FUNCTION_ID: nummethod_func003 - START
    m_int = 0; 

    m_h = (fabs(m_b-m_a))/N;
    for (int i = 0; i< N; ++i){
    m_int +=  m_f->eval( m_a + i*m_h + (m_h/2.) );
    }
    return m_int*m_h*m_sign;
    // FUNCTION_ID: nummethod_func003 - END
}

//======================================================
//              midpoint fixed precision
//======================================================

double integral::midpoint(){
    // FUNCTION_ID: nummethod_func006 - START
    int N = 1;
    double x = midpoint(N); 
    double y = midpoint(2*N); 
    while ( ((4./3.)*fabs(x-y)) > precision ){ 
        N = 2*N; 
        x = midpoint(N);
        y = midpoint(2*N);
    }
    return y;
    // FUNCTION_ID: nummethod_func006 - END
}

//======================================================
//              simpson fixed number of steps
//======================================================

    double integral::simpson (int N){
    // FUNCTION_ID: nummethod_func004 - START
    if (N%2 != 0 ){
        cerr << "N steps must be even for Simpson method" << endl;
        exit (1);
    }
    m_int = 0; 
    m_h = (abs(m_b-m_a))/N;

    for (int i = 0; i<= N; ++i){
        if (i == 0 || i== N) m_int += m_f->eval(m_a + m_h*i);
        else if (i%2 == 0) m_int += 2.*m_f->eval(m_a + m_h*i);
        else m_int += 4.*m_f->eval(m_a + m_h*i);
    }
    return m_int * m_h * m_sign * (1./3.);
    // FUNCTION_ID: nummethod_func004 - END

}


//======================================================
//              simpson fixed precision
//======================================================

    double integral::simpson (){
    // FUNCTION_ID: nummethod_func007 - START
    int N = 2; 
    double x = simpson(N); 
    double y = simpson(2*N);
    while ( ((16./15.)*abs(x-y)) > precision ){ 
        N = 2*N;
        x = simpson(N);
        y = simpson(2*N);
    }
    return y;
    // FUNCTION_ID: nummethod_func007 - END
}


//======================================================
//              trapezoidal fixed precision
//======================================================

double integral::trapezoidal (){ 
    m_old = 0; 
    double h_old = 0;
    int N = 2;
    
    m_old = (m_f->eval(m_a)+m_f->eval(m_b))/2.; 
    
    m_h = fabs(m_b-m_a)/N; 
    m_int = m_old+m_f->eval((m_a+m_b)/2); 

    while ((4./3.)*fabs(m_int*m_h - m_old*m_h*2)>precision){ 
        m_old = m_int; 
        N=N*2; 
        m_h = fabs(m_b-m_a)/N; 
        for (int i = 0; (m_a+m_h+i*2*m_h)<m_b; ++i) m_int += m_f->eval(m_a+m_h+2*m_h*i);
    }
    return m_int * m_sign * m_h;
}

double integral::trapezoidal_not_optimized (){
    // FUNCTION_ID: nummethod_func008 - START
    int N = 1;
    double x = trapezoidal(N); 
    double y = trapezoidal(2*N); 
    while (((4./3.)*abs(x-y)) > precision ){ 
        N = 2*N;
        x = trapezoidal(N);
        y = trapezoidal(2*N);
    }
    return y;
    // FUNCTION_ID: nummethod_func008 - END
}

//======================================================
//              trapezoidal fixed number of steps
//======================================================

double integral::trapezoidal (int N) 
{
    // FUNCTION_ID: nummethod_func005 - START
    m_h = (fabs(m_b-m_a) )/N;
    m_int = 0; 
    
    for (int i=0; i<=N; ++i){ 
        if (i==0 || i== N) m_int += (m_f->eval( m_a + m_h * i))/2.;
        else m_int += m_f->eval(m_a + m_h * i);
    }
    
    return m_int * m_h * m_sign;
    // FUNCTION_ID: nummethod_func005 - END
}






