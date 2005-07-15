#include "HelixClass.h"
#include <math.h>
#include <stdlib.h>
#include <iostream.h>
#include "ced_cli.h"

HelixClass::HelixClass() {
    _const_pi = acos(-1.0);
    _const_2pi = 2.0*_const_pi;
    _const_pi2 = 0.5*_const_pi;
    _FCT = 2.99792458E-4;
}

HelixClass::~HelixClass() {}

void HelixClass::Initialize_VP(float * pos, float * mom, float q, float B) {
    _referencePoint[0] = pos[0];
    _referencePoint[1] = pos[1];
    _referencePoint[2] = pos[2];
    _momentum[0] = mom[0];
    _momentum[1] = mom[1];
    _momentum[2] = mom[2];
    _charge = q;
    _bField = B;
//  Some calculations : define all parameters of the helix
    _pxy = sqrt(mom[0]*mom[0]+mom[1]*mom[1]);
    _radius = _pxy / (_FCT*B);
    _omega = q/_radius;
    _tanLambda = mom[2]/_pxy;
    _phiMomRefPoint = atan2(mom[1],mom[0]);
    _xCentre = pos[0] + _radius*cos(_phiMomRefPoint+_const_pi2*q);
    _yCentre = pos[1] + _radius*sin(_phiMomRefPoint+_const_pi2*q);
    _phiRefPoint = atan2(pos[1]-_yCentre,pos[0]-_xCentre);
    _phiAtPCA = atan2(-_yCentre,-_xCentre);
    _phi0 = _const_pi2*q + _phiAtPCA;
    if (_phi0 < -_const_pi)
	_phi0 += _const_2pi;
    if (_phi0 > _const_pi)
	_phi0 -= _const_2pi;
    _xAtPCA = _xCentre + _radius*cos(_phiAtPCA);
    _yAtPCA = _yCentre + _radius*sin(_phiAtPCA);
    _d0 = -_xAtPCA*sin(_phi0) + _yAtPCA*cos(_phi0);
    _pxAtPCA = _pxy*cos(_phi0);
    _pyAtPCA = _pxy*sin(_phi0);
    float deltaPhi = _phiRefPoint - _phiAtPCA;    
    float xCircles = pos[2]*q/(_radius*_tanLambda) - deltaPhi;
    xCircles = xCircles/_const_2pi;
    int nCircles;
    int n1,n2;

    if (xCircles >= 0.) {
	n1 = int(xCircles);
	n2 = n1 + 1;
    }
    else {
	n1 = int(xCircles) - 1;
	n2 = n1 + 1;
    }
    
    if (fabs(n1-xCircles) < fabs(n2-xCircles)) {
	nCircles = n1;
    }
    else {
	nCircles = n2;
    }
    _z0 = pos[2] - _radius*_tanLambda*q*(deltaPhi + _const_2pi*nCircles);

}

void HelixClass::Initialize_Canonical(float phi0, float d0, float z0, 
				   float omega, float tanLambda, float B) {
    _omega = omega;
    _d0 = d0;
    _phi0 = phi0;
    _z0 = z0;
    _tanLambda = tanLambda;
    _charge = -omega/fabs(omega);
    _radius = 1./fabs(omega);
    _xAtPCA = -_d0*sin(_phi0);
    _yAtPCA = _d0*cos(_phi0);    
    _referencePoint[0] = _xAtPCA;
    _referencePoint[1] = _yAtPCA;
    _referencePoint[2] = _z0;
    _pxy = _FCT*B*_radius;
    _momentum[0] = _pxy*cos(_phi0);
    _momentum[1] = _pxy*sin(_phi0);
    _momentum[2] = _tanLambda * _pxy;    
    _phiMomRefPoint = atan2(_momentum[1],_momentum[0]);
    _xCentre = _referencePoint[0] + 
      _radius*cos(_phiMomRefPoint+_const_pi2*_charge);
    _yCentre = _referencePoint[1] + 
      _radius*sin(_phiMomRefPoint+_const_pi2*_charge);
    _phiAtPCA = atan2(-_yCentre,-_xCentre);
    _phiRefPoint =  _phiAtPCA ;


}

const float * HelixClass::getMomentum() {
    return _momentum;
}
const float * HelixClass::getReferencePoint() {
    return _referencePoint;
}
float HelixClass::getPhi0() {
    return _phi0;
}
float HelixClass::getD0() {
    return _d0;
}
float HelixClass::getZ0() {
    return _z0;
}
float HelixClass::getOmega() {
    return _omega;
}
float HelixClass::getTanLambda() {
    return _tanLambda;
}
float HelixClass::getPXY() {
    return _pxy;
}

float HelixClass::getPointInXY(float x0, float y0, float ax, float ay, 
			      float * ref , float * point) {

  float time;

  float AA = sqrt(ax*ax+ay*ay);


  if (AA <= 0) {
    time = 1.0e+10; 
    return time;
  }


  float BB = ax*(x0-_xCentre) + ay*(y0-_yCentre);
  BB = BB / AA;

  float CC = (x0-_xCentre)*(x0-_xCentre) 
    + (y0-_yCentre)*(y0-_yCentre) - _radius*_radius;

  CC = CC / AA;

  float DET = BB*BB - CC;
  float tt1 = 0.;
  float tt2 = 0.;
  float xx1,xx2,yy1,yy2; 


  if (DET < 0 ) {
    time = 1.0e+10;
    point[0]=0.0;
    point[1]=0.0;
    point[2]=0.0;
    return time;
  }
  
  
  tt1 = - BB + sqrt(DET);
  tt2 = - BB - sqrt(DET);

  xx1 = x0 + tt1*ax;
  yy1 = y0 + tt1*ay;
  xx2 = x0 + tt2*ax;
  yy2 = y0 + tt2*ay;
  
  float phi1 = atan2(yy1-_yCentre,xx1-_xCentre);
  float phi2 = atan2(yy2-_yCentre,xx2-_xCentre);
  float phi0 = atan2(ref[1]-_yCentre,ref[0]-_xCentre);

  float dphi1 = phi1 - phi0;
  float dphi2 = phi2 - phi0;

  if (dphi1 < 0 && _charge > 0) {
    dphi1 = dphi1 + _const_2pi;
  }
  else if (dphi1 > 0 && _charge < 0) { 
    dphi1 = dphi1 - _const_2pi;
  }

  if (dphi2 < 0 && _charge > 0) {
    dphi2 = dphi2 + _const_2pi;
  }
  else if (dphi2 > 0 && _charge < 0) { 
    dphi2 = dphi2 - _const_2pi;
  }

  // Times
  tt1 = _charge*dphi1*_radius/_pxy;
  tt2 = _charge*dphi2*_radius/_pxy;

  if (tt1 < 0. )
    std::cout << "WARNING " << tt1 << std::endl;
  if (tt2 < 0. )
    std::cout << "WARNING " << tt2 << std::endl;
  

  if (tt1 < tt2) {
    point[0] = xx1;
    point[1] = yy1;
    time = tt1;
  }
  else {
    point[0] = xx2;
    point[1] = yy2;
    time = tt2;
  }

  point[2] = ref[2]+time*_momentum[2];

  

  return time;

}

float HelixClass::getPointInZ(float zLine, float * ref, float * point) {

  float time = zLine - ref[2];

  if (_momentum[2] == 0.) {
    time = 1.0e+10;
    point[0] = 0.;
    point[1] = 0.;
    point[2] = 0.;
    return time;
  }

  time = time/_momentum[2];

  float phi0 = atan2(ref[1] - _yCentre , ref[0] - _xCentre);
  float phi = phi0 + _charge*_pxy*time/_radius;
  float xx = _xCentre + _radius*cos(phi);
  float yy = _yCentre + _radius*sin(phi);

  point[0] = xx;
  point[1] = yy;
  point[2] = zLine;

  return time;


}