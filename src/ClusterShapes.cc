#include "ClusterShapes.h"





// #################################################
// #####                                       #####
// #####  Additional Structures and Functions  #####
// #####                                       #####
// #################################################
//=============================================================================

struct data {
  int n;
  float* x;
  float* y;
  float* z;
};

//=============================================================================

int funct(const gsl_vector* par, void* d, gsl_vector* f) {
  //     For helix fiting
  float x0   = gsl_vector_get(par,0);
  float y0   = gsl_vector_get(par,1);
  float R    = gsl_vector_get(par,2);
  float b    = gsl_vector_get(par,3);
  float phi0 = gsl_vector_get(par,4);
  int n    = ((struct data*)d)->n;
  float* x = ((struct data*)d)->x;
  float* y = ((struct data*)d)->y;
  float* z = ((struct data*)d)->z;
  // calculate fit function f[i] = 
  // ( (x0 + R*cos(b*z[i] + phi0)) - y[i] ) for i = 0 to n-1
  //                    and f[i] = 
  // ( (y0 + R*sin(b*z[i] + phi0)) - y[i] ) for i = n to dim*n - 1
  float fi = 0.0;

  // first dimension
  for (int i(0); i < n; i++) {
    fi = (x0 + R*cos(b*z[i] + phi0)) - x[i];
    gsl_vector_set(f,i,fi);
  }
  // second dimension
  for (int i(0); i < n; i++) {
    fi = (y0 + R*sin(b*z[i] + phi0)) - y[i];  
    gsl_vector_set(f,i+n,fi);
  }

  return GSL_SUCCESS;
}

//=============================================================================

int dfunct(const gsl_vector* par, void* d, gsl_matrix* J) {
  //     For helix fiting


  float R    = gsl_vector_get(par,2);
  float b    = gsl_vector_get(par,3);
  float phi0 = gsl_vector_get(par,4);

  int n    = ((struct data*)d)->n;
  float* z = ((struct data*)d)->z;


  // calculate Jacobi's matrix J[i][j] = dfi/dparj

  // part of Jacobi's matrix corresponding to first dimension
  for (int i(0); i < n; i++) {
    
    gsl_matrix_set(J,i,0,1);
    gsl_matrix_set(J,i,1,0);
    gsl_matrix_set(J,i,2,cos(b*z[i]+phi0));
    gsl_matrix_set(J,i,3,-z[i]*R*sin(b*z[i]+phi0));
    gsl_matrix_set(J,i,4,-R*sin(b*z[i]+phi0));

  }
  
  // part of Jacobi's matrix corresponding to second dimension
  for (int i(0); i < n; i++) {
    
    gsl_matrix_set(J,i+n,0,0);
    gsl_matrix_set(J,i+n,1,1);
    gsl_matrix_set(J,i+n,2,sin(b*z[i]+phi0));
    gsl_matrix_set(J,i+n,3,z[i]*R*cos(b*z[i]+phi0));
    gsl_matrix_set(J,i+n,4,R*cos(b*z[i]+phi0));
    
  }
  
  return GSL_SUCCESS;
}

//=============================================================================

int fdf(const gsl_vector* par, void* d, gsl_vector* f, gsl_matrix* J) {
  //     For helix fiting

  funct(par, d, f);
  dfunct(par, d, J);

  return GSL_SUCCESS;

}

//=============================================================================






// ##########################################
// #####                                #####
// #####   Constructor and Destructor   #####
// #####                                #####
// ##########################################

//=============================================================================

ClusterShapes::ClusterShapes(int nhits, float* a, float* x, float* y, float* z){

  _nHits = nhits;
  _aHit = new float[_nHits] ;
  _xHit = new float[_nHits] ;
  _yHit = new float[_nHits] ;
  _zHit = new float[_nHits] ;    
  for (int i(0); i < nhits; ++i) {
    _aHit[i] = a[i] ;
    _xHit[i] = x[i] ;
    _yHit[i] = y[i] ;
    _zHit[i] = z[i] ;
  }
  _ifNotGravity = 1 ;
  _ifNotWidth   = 1 ;
  _ifNotInertia = 1 ;
}


//=============================================================================

ClusterShapes::~ClusterShapes() {

  delete[] _aHit ;
  delete[] _xHit ;
  delete[] _yHit ;
  delete[] _zHit ;
}

//=============================================================================






// ##########################################
// #####                                #####
// #####        public methods          #####
// #####                                #####
// ##########################################

//=============================================================================

int ClusterShapes::getNumberOfHits() {
  return _nHits;
}

//=============================================================================

float ClusterShapes::getTotalAmplitude() {
  if (_ifNotGravity == 1) findGravity();
  return _totAmpl;
}

//=============================================================================

float* ClusterShapes::getCentreOfGravity() {
  if (_ifNotGravity == 1) findGravity() ;
  return &_analogGravity[0] ;    
}

//=============================================================================

float* ClusterShapes::getEigenValInertia() {
  if (_ifNotInertia == 1) findInertia();
  return &_ValAnalogInertia[0] ;
}

//=============================================================================

float* ClusterShapes::getEigenVecInertia() {
  if (_ifNotInertia == 1) findInertia();
  return &_VecAnalogInertia[0] ;
}

//=============================================================================

float ClusterShapes::getWidth() {
  if (_ifNotWidth == 1) findWidth();
  return _analogWidth;
}

//=============================================================================

int ClusterShapes::Fit3DProfile(float& chi2, float& a, float& b, float& c, float& d,
				float* xStart) {

  if (_ifNotInertia == 1) findInertia();
  
  float MainAxis[3];
  float MainCentre[3];

  MainAxis[0] = _VecAnalogInertia[0];
  MainAxis[1] = _VecAnalogInertia[1];
  MainAxis[2] = _VecAnalogInertia[2];      
  MainCentre[0] = _analogGravity[0];
  MainCentre[1] = _analogGravity[1];
  MainCentre[2] = _analogGravity[2];      
  
  int ifirst = 0;
  float xx[3];
  float prodmin = 0.0;
  
  for (int i(0); i < _nHits; ++i) {
    xx[0] = _xHit[i] - MainCentre[0];
    xx[1] = _yHit[i] - MainCentre[1];
    xx[2] = _zHit[i] - MainCentre[2];
    float prod = vecProject(xx,MainAxis);
    if (ifirst == 0 || prod < prodmin) {
      ifirst = 1;
      prodmin = prod;
    }
  }
  xStart[0] = MainCentre[0] + prodmin*MainAxis[0];
  xStart[1] = MainCentre[1] + prodmin*MainAxis[1];
  xStart[2] = MainCentre[2] + prodmin*MainAxis[2];	  
  float * xl = new float[_nHits];
  float * xt = new float[_nHits];
  const gsl_rng_type * T;
  gsl_rng * r;
  
  gsl_rng_env_setup();
  T = gsl_rng_default;
  r = gsl_rng_alloc(T);
  
  //std::cout << a << " " << b << " " << c << " " << d << std::endl;
  
  for (int i(0); i < _nHits; ++i) {
    xx[0] = _xHit[i] - xStart[0];
    xx[1] = _yHit[i] - xStart[1];
    xx[2] = _zHit[i] - xStart[2];
    float xx2(0.);
    for (int j(0); j < 3; ++j) xx2 += xx[j]*xx[j];      
    
    xl[i] = 0.001 + vecProject(xx,MainAxis);    
    xt[i] = sqrt(max(0.,xx2 + 0.1 - xl[i]*xl[i]));
    //std::cout << i << " " << xl[i] << " " << xt[i] << " " << _aHit[i] << " " 
    //          << Ampl << std::endl;
  }
  
  gsl_rng_free( r );

  float Slnxl(0.);
  float Sxl(0.);
  float Sxt(0.);
  float Sln2xl(0.);
  float Sxllnxl(0.);
  float Sxtlnxl(0.);
  float Sxlxl(0.);
  float Sxlxt(0.);
  float Sxtxt(0.);
  float SlnA(0.);
  float SlnAlnxl(0.);
  float SlnAxl(0.);
  float SlnAxt(0.);

  // for a quadratic matrix
  for (int i = 0; i < _nHits; i++) {
    Slnxl += log(xl[i]);
    Sxl += xl[i];
    Sxt += xt[i];
    Sln2xl += log(xl[i])*log(xl[i]);
    Sxllnxl += xl[i]*log(xl[i]);
    Sxtlnxl += xt[i]*log(xl[i]);
    Sxlxl += xl[i]*xl[i];
    Sxlxt += xl[i]*xt[i];
    Sxtxt += xt[i]*xt[i];
    SlnA += log(_aHit[i]);
    SlnAlnxl += log(_aHit[i])*log(xl[i]);
    SlnAxl += log(_aHit[i])*xl[i];
    SlnAxt += log(_aHit[i])*xt[i]; 
  }
  // create system of linear equations, written as Ae = z

  gsl_matrix* A = gsl_matrix_alloc(4,4);
  gsl_vector* z = gsl_vector_alloc(4);
  gsl_vector* e = gsl_vector_alloc(4);
  
  // initialise matrix and vectors
  
  gsl_matrix_set(A,0,0,_nHits);
  gsl_matrix_set(A,0,1,Slnxl);
  gsl_matrix_set(A,0,2,-Sxl);
  gsl_matrix_set(A,0,3,-Sxt);
  
  gsl_matrix_set(A,1,0,Slnxl);
  gsl_matrix_set(A,1,1,Sln2xl);
  gsl_matrix_set(A,1,2,-Sxllnxl);
  gsl_matrix_set(A,1,3,-Sxtlnxl);
  
  gsl_matrix_set(A,2,0,-Sxl);
  gsl_matrix_set(A,2,1,-Sxllnxl);
  gsl_matrix_set(A,2,2,Sxlxl);
  gsl_matrix_set(A,2,3,Sxlxt);

  gsl_matrix_set(A,3,0,-Sxt);
  gsl_matrix_set(A,3,1,-Sxtlnxl);
  gsl_matrix_set(A,3,2,Sxlxt);
  gsl_matrix_set(A,3,3,Sxtxt);

  gsl_vector_set(z,0,SlnA);
  gsl_vector_set(z,1,SlnAlnxl);
  gsl_vector_set(z,2,-SlnAxl);
  gsl_vector_set(z,3,-SlnAxt);
  
  gsl_linalg_HH_solve(A,z,e);

  a = exp(gsl_vector_get(e,0));
  b = gsl_vector_get(e,1);
  c = gsl_vector_get(e,2);
  d = gsl_vector_get(e,3);

  chi2 = 0.0;
  for (int i(0); i < _nHits; ++i) {
    float Ampl = a*(float)pow(xl[i],b)*exp(-c*xl[i]-d*xt[i]); 
      chi2 += ((Ampl - _aHit[i])*(Ampl - _aHit[i]))/(_aHit[i]*_aHit[i]);

  }
  chi2 = chi2/max((float)1.0,(float)(_nHits - 4));
  
  delete[] xl;
  delete[] xt;

  gsl_matrix_free(A);
  gsl_vector_free(z);
  gsl_vector_free(e);

  int result = 0;
  return result;
}

//=============================================================================

float ClusterShapes::getChi2Fit3DProfile(float a, float b, float c, float d) {

  float chi2 = 0.0;

  float MainAxis[3];
  float MainCentre[3];
  float xStart[3];

  int ifirst = 0;
  float xx[3];
  float prodmin = 0.0;
  
  if (_ifNotInertia == 1) findInertia();
  
  MainAxis[0] = _VecAnalogInertia[0];
  MainAxis[1] = _VecAnalogInertia[1];
  MainAxis[2] = _VecAnalogInertia[2];      
  MainCentre[0] = _analogGravity[0];
  MainCentre[1] = _analogGravity[1];
  MainCentre[2] = _analogGravity[2];      
  
  for (int i(0); i < _nHits; ++i) {
    xx[0] = _xHit[i] - MainCentre[0];
    xx[1] = _yHit[i] - MainCentre[1];
    xx[2] = _zHit[i] - MainCentre[2];
    float prod = vecProject(xx,MainAxis);
    if (ifirst == 0 || prod < prodmin) {
      ifirst = 1;
      prodmin = prod;
    }
  }
  xStart[0] = MainCentre[0] + prodmin*MainAxis[0];
  xStart[1] = MainCentre[1] + prodmin*MainAxis[1];
  xStart[2] = MainCentre[2] + prodmin*MainAxis[2];	  
  float * xl = new float[_nHits];
  float * xt = new float[_nHits];
  
  for (int i(0); i < _nHits; ++i) {
    xx[0] = _xHit[i] - xStart[0];
    xx[1] = _yHit[i] - xStart[1];
    xx[2] = _zHit[i] - xStart[2];
    float xx2(0.);
    for (int j(0); j < 3; ++j) xx2 += xx[j]*xx[j];      
    
    xl[i] = 0.001 + vecProject(xx,MainAxis);    
    xt[i] = sqrt(max(0.,xx2 + 0.1 - xl[i]*xl[i]));
    //std::cout << i << " " << xl[i] << " " << xt[i] << " " << _aHit[i] << " " 
    //          << Ampl << std::endl;
  }

  for (int i(0); i < _nHits; ++i) {
    float Ampl = a*(float)pow(xl[i],b)*exp(-c*xl[i]-d*xt[i]); 
    chi2 += ((Ampl - _aHit[i])*(Ampl - _aHit[i]))/(_aHit[i]*_aHit[i]);
    
  }

  chi2 = chi2/max((float)1.0,(float)(_nHits - 4));
  
  delete[] xl;
  delete[] xt;
  
  return chi2;
  
}

//=============================================================================

int ClusterShapes::FitHelix(int max_iter, int status_out, 
			    float* parameter, float* dparameter, float& chi2, 
			    float& distmax) {
  

  if (_nHits < 3) {
      std::cout << "ClusterShapes : helix fit impossible, two few points" ;
      std::cout << std::endl;
      for (int i(0); i < 5; ++i) {
	  parameter[i] = 0.;
	  dparameter[i] = 0.;
      }
      return 1;
  }

  // find initial parameters

  float Rmin = 1.0e+10;
  float Rmax = -1.0;
  int i1 = 0;

  // 1st loop  
  for (int i(0); i < _nHits; ++i) {
      float Rz = sqrt(_xHit[i]*_xHit[i] + _yHit[i]*_yHit[i]);
      if (Rz < Rmin) {
	  Rmin = Rz;
	  i1 = i;
      }
      if (Rz > Rmax) {
	  Rmax = Rz;
      }

  }

  // 2nd loop
  float Upper = Rmin + 1.1*(Rmax-Rmin);
  float Lower = Rmin + 0.9*(Rmax-Rmin);
  float dZmin  = 1.0e+20;

  int i3 = 0 ;

  for (int i(0); i < _nHits; ++i) {
      float Rz = sqrt(_xHit[i]*_xHit[i] + _yHit[i]*_yHit[i]);
      if ((Rz > Lower) && (Rz < Upper)) {
	  float dZ = fabs(_zHit[i]-_zHit[i1]);
	  if (dZ < dZmin) {
	      dZmin = dZ;
	      i3 = i;
	  }
      }
  }

  float z1 = min(_zHit[i1],_zHit[i3]);
  float z3 = max(_zHit[i1],_zHit[i3]);


  int i2 = 0;
  float dRmin = 1.0e+20;
  float Rref = 0.5 * ( Rmax + Rmin );

  // 3d loop

  for (int i(0); i < _nHits; ++i) {
      if (_zHit[i] >= z1 && _zHit[i] <= z3) {
	  float Rz = sqrt(_xHit[i]*_xHit[i] + _yHit[i]*_yHit[i]);
	  float dRz = fabs(Rz - Rref);
	  if (dRz < dRmin) {
	      i2 = i;
	      dRmin = dRz;
	  }
      }
  }


  if (i2 == 0 ) {
      for (int i(0); i < _nHits; ++i) {
	  if (i2 != i1 && i2 != i3) {
	      i2 = i;
	      if (_zHit[i2] < z1) {
		  int itemp = i1;
		  i1 = i2;
		  i2 = itemp;
	      }
	      else if (_zHit[i2] > z3) {
		  int itemp = i3;
		  i3 = i2;
		  i2 = itemp;
	      }
	      break;
	  }
      }      
  }


  float x0  = 0.5*(_xHit[i2]+_xHit[i1]);
  float y0  = 0.5*(_yHit[i2]+_yHit[i1]);
  float x0p = 0.5*(_xHit[i3]+_xHit[i2]);
  float y0p = 0.5*(_yHit[i3]+_yHit[i2]);
  float ax  = _yHit[i2] - _yHit[i1];
  float ay  = _xHit[i1] - _xHit[i2];
  float axp = _yHit[i3] - _yHit[i2];
  float ayp = _xHit[i2] - _xHit[i3];
  float det = ax * ayp - axp * ay;
  float time;

  if (det == 0.) {
      time = 500.;
  }
  else {
      gsl_matrix* A = gsl_matrix_alloc(2,2);
      gsl_vector* B = gsl_vector_alloc(2);
      gsl_vector* T = gsl_vector_alloc(2);     
      gsl_matrix_set(A,0,0,ax);
      gsl_matrix_set(A,0,1,-axp);
      gsl_matrix_set(A,1,0,ay);
      gsl_matrix_set(A,1,1,-ayp);
      gsl_vector_set(B,0,x0p-x0);
      gsl_vector_set(B,1,y0p-y0);
      gsl_linalg_HH_solve(A,B,T);
      time = gsl_vector_get(T,0); 
      gsl_matrix_free(A);
      gsl_vector_free(B);
      gsl_vector_free(T);
  }

  float X0 = x0 + ax*time;
  float Y0 = y0 + ay*time;

  float dX = _xHit[i1] - X0;
  float dY = _yHit[i1] - Y0;

  float R0 = sqrt(dX*dX + dY*dY);

  if (R0 == 0.) 
    std::cout << "Something is wrong; nHits = " << _nHits << std::endl;

  float _pi = acos(-1.);

  float phi1 = (float)atan2(_yHit[i1]-Y0,_xHit[i1]-X0);
  float phi2 = (float)atan2(_yHit[i2]-Y0,_xHit[i2]-X0);
  float phi3 = (float)atan2(_yHit[i3]-Y0,_xHit[i3]-X0);

// testing bz > 0 hypothesis

  if ( phi1 > phi2 ) 
      phi2 = phi2 + 2.0*_pi;
  if ( phi1 > phi3 )
      phi3 = phi3 + 2.0*_pi;
  if ( phi2 > phi3 )
      phi3 = phi3 + 2.0*_pi;

  float bz_plus = (phi3 - phi1) / (_zHit[i3]-_zHit[i1]);
  float phi0_plus = phi1 - bz_plus * _zHit[i1];
  float dphi_plus = fabs( bz_plus * _zHit[i2] + phi0_plus - phi2 );

// testing bz < 0 hypothesis

  phi1 = (float)atan2(_yHit[i1]-Y0,_xHit[i1]-X0);
  phi2 = (float)atan2(_yHit[i2]-Y0,_xHit[i2]-X0);
  phi3 = (float)atan2(_yHit[i3]-Y0,_xHit[i3]-X0);

  if ( phi1 < phi2 ) 
      phi2 = phi2 - 2.0*_pi;
  if ( phi1 < phi3 )
      phi3 = phi3 - 2.0*_pi;
  if ( phi2 < phi3 )
      phi3 = phi3 - 2.0*_pi;

  float bz_minus = (phi3 - phi1) / (_zHit[i3]-_zHit[i1]);
  float phi0_minus = phi1 - bz_minus * _zHit[i1];
  float dphi_minus = fabs( bz_minus * _zHit[i2] + phi0_minus - phi2 );

  float bz;
  float phi0;

  if (dphi_plus < dphi_minus) {
      bz = bz_plus;
      phi0 = phi0_plus;
  }
  else {
      bz = bz_minus;
      phi0 = phi0_minus;

  }

  double par_init[5];

  par_init[0] = (double)X0;
  par_init[1] = (double)Y0;
  par_init[2] = (double)R0;
  par_init[3] = (double)bz;
  par_init[4] = (double)phi0;

  // local variables
  int status = 0;
  int iter = 0;

  const int npar = 5; // five parameters to fit
  const int ndim = 2; // two dependent dimensions 


  float chi2_nofit = 0.0;
  int iFirst = 1;
  for (int ipoint(0); ipoint < _nHits; ipoint++) {
    float Dist = DistanceHelix(_xHit[ipoint],_yHit[ipoint],_zHit[ipoint],
			       X0,Y0,R0,bz,phi0);
    chi2_nofit = chi2_nofit + Dist;
    if (Dist > distmax || iFirst == 1) {
      distmax = Dist;
      iFirst = 0;
    }
  }      
  chi2_nofit = chi2_nofit/(float)_nHits;

  if ( status_out == 1 ) {
    for (int i(0); i < 5; ++i) {
      parameter[i] = (float)par_init[i];
      dparameter[i] = 0.0;      
    }
    chi2 = chi2_nofit;
    return 0;
  }


  // converging criteria
  const double abs_error = 1e-4;
  const double rel_error = 1e-4;

  gsl_multifit_function_fdf fitfunct;

  const gsl_multifit_fdfsolver_type* T = gsl_multifit_fdfsolver_lmsder;

  gsl_multifit_fdfsolver* s = gsl_multifit_fdfsolver_alloc(T,ndim*_nHits,npar);

  gsl_matrix* covar = gsl_matrix_alloc(npar,npar);   // covariance matrix

  data d;
  d.n = _nHits;
  d.x = &_xHit[0];
  d.y = &_yHit[0];
  d.z = &_zHit[0];


  fitfunct.f = &funct;
  fitfunct.df = &dfunct;
  fitfunct.fdf = &fdf;
  fitfunct.n = ndim*_nHits;
  fitfunct.p = npar;
  fitfunct.params = &d;

  gsl_vector_view pinit = gsl_vector_view_array(par_init,npar);
  gsl_multifit_fdfsolver_set(s,&fitfunct,&pinit.vector);

  // perform fit
  do {
    iter++;
    status = gsl_multifit_fdfsolver_iterate(s);

    if (status) break;
    status = gsl_multifit_test_delta (s->dx, s->x,abs_error,rel_error);

  } while ( status==GSL_CONTINUE && iter < max_iter);

  gsl_multifit_covar (s->J, rel_error, covar);

  chi2 = 0.0;
  X0   = (float)gsl_vector_get(s->x,0);
  Y0   = (float)gsl_vector_get(s->x,1);
  R0   = (float)gsl_vector_get(s->x,2);
  bz   = (float)gsl_vector_get(s->x,3);
  phi0 = (float)gsl_vector_get(s->x,4);

  iFirst = 1;
  float ddmax = 0.0;
  for (int ipoint(0); ipoint < _nHits; ipoint++) {
    float Dist = DistanceHelix(_xHit[ipoint],_yHit[ipoint],_zHit[ipoint],
			       X0,Y0,R0,bz,phi0);
    chi2 = chi2 + Dist;
    if (Dist > ddmax || iFirst == 1) {
      iFirst = 0;
      ddmax = Dist;
    }
  }
      
  chi2 = chi2/(float)_nHits;

  if (chi2 < chi2_nofit) {
    for (int i = 0; i < npar; i++) {
      parameter[i]  = gsl_vector_get(s->x,i);
      dparameter[i] = sqrt(gsl_matrix_get(covar,i,i));
    }    
    distmax = ddmax;
  }
  else {
    chi2 = chi2_nofit;
    for (int i = 0; i < npar; i++) {
      parameter[i] = (float)par_init[i];
      dparameter[i] = 0.0;
    }
  }

  gsl_multifit_fdfsolver_free(s);
  gsl_matrix_free(covar);
  return 0; 

}






// ##########################################
// #####                                #####
// #####        private methods         #####
// #####                                #####
// ##########################################

//=============================================================================

void ClusterShapes::findElipsoid() {

  /**   Elipsoid parameter calculations see cluster_proper.f  */
  float cx,cy,cz ;
  float dx,dy,dz ;
  float r_hit_max, d_begn, d_last, r_max, proj;
  if (_ifNotInertia == 1) findInertia() ;
  //   Normalize the eigen values of inertia tensor
  float wr1 = sqrt(_ValAnalogInertia[0]/_totAmpl);
  float wr2 = sqrt(_ValAnalogInertia[1]/_totAmpl);
  float wr3 = sqrt(_ValAnalogInertia[2]/_totAmpl);
  _r1 = sqrt(wr2*wr3);                // spatial axis length -- the largest
  _r2 = sqrt(wr1*wr3);                // spatial axis length -- less
  _r3 = sqrt(wr1*wr2);                // spatial axis length -- even more less
  _vol = 4.*M_PI*_r1*_r2*_r3/3.;      // ellipsoid volume
  _r_ave = cbrtf(_vol);               // average radius  (qubic root)
  _density = _totAmpl/_vol;           // density
  //    _eccentricity = _r_ave/_r1;   // Cluster Eccentricity
  _eccentricity =_analogWidth/_r1;   // Cluster Eccentricity

  // Find Minumal and Maximal Lenght for Principal axis
  r_hit_max = -100000.;
  d_begn    =  100000.;
  d_last    = -100000.;
  cx = _VecAnalogInertia[0] ;
  cy = _VecAnalogInertia[1] ;
  cz = _VecAnalogInertia[2] ;
  for (int i(0); i < _nHits; ++i) {
    dx = _xHit[i] - _xgr;
    dy = _yHit[i] - _ygr;
    dz = _zHit[i] - _zgr;
    r_max = sqrt(dx*dx + dy*dy + dz*dz);;
    if(r_max > r_hit_max) r_hit_max = r_max;
    proj = dx*cx + dy*cy + dz*cz;
    if(proj < d_begn)
      d_begn = proj;
    //            lad_begn = ladc(L)
    if(proj > d_last)
      d_last = proj;
    //            lad_last = ladc(L)
  }
  //        if (r_hit_max > 0.0)
  //	  _r1 = 1.05*r_hit_max; // + 5% of length
  _r1_forw = abs(d_last);
  _r1_back = abs(d_begn);
}

//=============================================================================

void ClusterShapes::findGravity() {

    _totAmpl = 0. ;
    for (int i(0); i < 3; ++i) {
	_analogGravity[i] = 0.0 ;
    }
    for (int i(0); i < _nHits; ++i) {
	_totAmpl+=_aHit[i] ;
	_analogGravity[0]+=_aHit[i]*_xHit[i] ;
	_analogGravity[1]+=_aHit[i]*_yHit[i] ;
	_analogGravity[2]+=_aHit[i]*_zHit[i] ;
    }
    for (int i(0); i < 3; ++i) {
	_analogGravity[i]/=_totAmpl ;
    }
    _xgr = _analogGravity[0];
    _ygr = _analogGravity[1];
    _zgr = _analogGravity[2];
    _ifNotGravity = 0;
}

//=============================================================================

void ClusterShapes::findInertia() {

  double aIne[3][3];
  //  float radius1;
  float radius2 = 0.0;

  findGravity();

  for (int i(0); i < 3; ++i) {
      for (int j(0); j < 3; ++j) {
	  aIne[i][j] = 0.0;
      }
  }

  for (int i(0); i < _nHits; ++i) {
      float dX = _xHit[i] - _analogGravity[0];
      float dY = _yHit[i] - _analogGravity[1];
      float dZ = _zHit[i] - _analogGravity[2];
      aIne[0][0] += _aHit[i]*(dY*dY+dZ*dZ);
      aIne[1][1] += _aHit[i]*(dX*dX+dZ*dZ);
      aIne[2][2] += _aHit[i]*(dX*dX+dY*dY);
      aIne[0][1] -= _aHit[i]*dX*dY;
      aIne[0][2] -= _aHit[i]*dX*dZ;
      aIne[1][2] -= _aHit[i]*dY*dZ;
  }

  for (int i(0); i < 2; ++i) {
      for (int j = i+1; j < 3; ++j) {
	  aIne[j][i] = aIne[i][j];
      }
  }
  //****************************************
  // analog Inertia
  //****************************************

  gsl_matrix_view aMatrix = gsl_matrix_view_array((double*)aIne,3,3);
  gsl_vector* aVector = gsl_vector_alloc(3);
  gsl_matrix* aEigenVec = gsl_matrix_alloc(3,3);
  gsl_eigen_symmv_workspace* wa = gsl_eigen_symmv_alloc(3);
  gsl_eigen_symmv(&aMatrix.matrix,aVector,aEigenVec,wa);
  gsl_eigen_symmv_free(wa);
  gsl_eigen_symmv_sort(aVector,aEigenVec,GSL_EIGEN_SORT_ABS_ASC);

  for (int i(0); i < 3; i++) {
    _ValAnalogInertia[i] = gsl_vector_get(aVector,i);
    for (int j(0); j < 3; j++) {
      _VecAnalogInertia[i+3*j] = gsl_matrix_get(aEigenVec,i,j);
    }
  }

  // Main principal points away from IP

  _radius = 0.;
  radius2 = 0.;

  for (int i(0); i < 3; ++i) {
      _radius += _analogGravity[i]*_analogGravity[i];
      radius2 += (_analogGravity[i]+_VecAnalogInertia[i])*(_analogGravity[i]+_VecAnalogInertia[i]);
  }

  if ( radius2 < _radius) {
      for (int i(0); i < 3; ++i)
	  _VecAnalogInertia[i] = - _VecAnalogInertia[i];
  }

  _radius = sqrt(_radius);
  _ifNotInertia = 0;

  // The final job
  findWidth();
  findElipsoid();

  gsl_vector_free(aVector);
  gsl_matrix_free(aEigenVec);

}

//=============================================================================

void ClusterShapes::findWidth() {

  float dist = 0.0;
  if (_ifNotInertia == 1)  findInertia() ;
  _analogWidth  = 0.0 ;
  for (int i(0); i < _nHits; ++i) {
    dist = findDistance(i) ;
    _analogWidth+=_aHit[i]*dist*dist ;
  }
  _analogWidth  = sqrt(_analogWidth / _totAmpl) ;
  _ifNotWidth = 0 ;
}

//=============================================================================

float ClusterShapes::findDistance(int i) {

    float cx = 0.0;
    float cy = 0.0;
    float cz = 0.0;
    float dx = 0.0;
    float dy = 0.0;
    float dz = 0.0;
    cx = _VecAnalogInertia[0] ;
    cy = _VecAnalogInertia[1] ;
    cz = _VecAnalogInertia[2] ;
    dx = _analogGravity[0] - _xHit[i] ;
    dy = _analogGravity[1] - _yHit[i] ;
    dz = _analogGravity[2] - _zHit[i] ;
    float tx = cy*dz - cz*dy ;
    float ty = cz*dx - cx*dz ;
    float tz = cx*dy - cy*dx ;
    float tt = sqrt(tx*tx+ty*ty+tz*tz) ;
    float ti = sqrt(cx*cx+cy*cy+cz*cz) ;
    float f = tt / ti ;
    return f ;
}
/**
      Function sdist(xp,yp,zp,cx,cy,cz,xv,yv,zv)
c----------------------------------------------------------------------
c        Distance from line to point
c       xp, yp, zp -- point is at the line
c       xv, yv, zv -- point is out of line
********************************************************************
*     Last update     V.L.Morgunov     08-Apr-2002                 *
********************************************************************
      real xp,yp,zp,cx,cy,cz,xv,yv,zv,t1,t2,t3,tt,sdist

      t1 = cy*(zp-zv)-cz*(yp-yv)
      t2 = cz*(xp-xv)-cx*(zp-zv)
      t3 = cx*(yp-yv)-cy*(xp-xv)
      tt = sqrt(cx**2+cy**2+cz**2)
      sdist = sqrt(t1**2+t2**2+t3**2)/tt

      return
      end
*/

//=============================================================================

float ClusterShapes::vecProduct(float * x1, float * x2) {

    float x1abs(0.);
    float x2abs(0.);
    float prod(0.);

    for (int i(0); i < 3; ++i) {
	x1abs += x1[i]*x1[i];
	x2abs += x2[i]*x2[i];
	prod  += x1[i]*x2[i];
    }


    x1abs = sqrt(x1abs);
    x2abs = sqrt(x2abs);

    if (x1abs > 0.0 && x2abs > 0.0) {
	prod = prod/(x1abs*x2abs);
    }
    else {
	prod = 0.;
    }

    return prod;

}

//=============================================================================

float ClusterShapes::vecProject(float * x, float * axis) {
    float axisabs(0.);
    float prod(0.);
    for (int i(0); i < 3; ++i) {
	axisabs += axis[i]*axis[i];
	prod  += x[i]*axis[i];
    }
    axisabs = sqrt(axisabs);
    if (axisabs > 0.0 ) {
	prod = prod/axisabs;
    }
    else {
	prod = 0.;
    }
    return prod;
}

//=============================================================================

float ClusterShapes::DistanceHelix(float x, float y, float z, float X0, float Y0,
				   float R0, float bz, float phi0) {

  float phi  = atan2(y-Y0,x-X0);
  float R    = sqrt( (y-Y0)*(y-Y0) + (x-X0)*(x-X0) );
  float dXY2 = (R-R0)*(R-R0);
  float _const_2pi = 2.0*acos(-1.0);
  float xN = (bz*z + phi0 - phi)/_const_2pi;

  int n1 = 0;
  int n2 = 0;
  int nSpirals = 0;

  if (xN > 0) {
    n1 = (int)xN;
    n2 = n1 + 1;
  }
  else {
    n1 = (int)xN - 1;
    n2 = n1 + 1;
  }

  if (fabs(n1-xN) < fabs(n2-xN)) {
    nSpirals = n1;
  }
  else {
    nSpirals = n2;
  }

  float dZ = (phi + _const_2pi*nSpirals - phi0)/bz - z;
  float dZ2 = dZ*dZ;

  return sqrt(dXY2 + dZ2);

}

//=============================================================================