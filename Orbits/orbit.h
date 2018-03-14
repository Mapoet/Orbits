
#ifndef __ORBIT_H__
#define __ORBIT_H__
#define  NSTR              (20)
#define  NDEM              (10)
#define  NPLANET           (13) 
#define  DS                (86400)
typedef long double real;
typedef unsigned int uint;
static real G = 6.67259e-11;
static real c = 299792458.;
static real c2 = 299792458.*299792458.;
static real AU = 149597870700.;
static real SM = 1.989e30;
static real SR = 6.955e8;
static real yita = 0;// 0.045 / 9.6212e47;// 0.045;
extern uint ndem;
typedef enum gravitytype{
	Gravity_Newtun,
	Gravity_PN,
	Gravity_EIH
}GravityType;
extern GravityType GType;
typedef struct {
	real a;
	real e;
	real w;
}orbit;
struct pos{
	real pos[NDEM];
};
typedef struct pos Pos;
struct body{
	real M;
	real R;
	real T;
	char name[20];
	char img[30];
	real A;
	real B;
	real C;
};
typedef struct body Body;

real sprod(Pos p1, Pos p2);
Pos  vprod(Pos p1, Pos p2);
Pos  norms(Pos p);
real pabs(Pos p);


Pos ppp(Pos p1, Pos p2);
Pos psp(Pos p1, Pos p2);
Pos pmp(Pos p1, Pos p2);
Pos pdp(Pos p1, Pos p2);
Pos vpp(real p1, Pos p2);
Pos vsp(real p1, Pos p2);
Pos vmp(real p1, Pos p2);
Pos vdp(real p1, Pos p2);
Pos ppv(Pos p1, real p2);
Pos psv(Pos p1, real p2);
Pos pmv(Pos p1, real p2);
Pos pdv(Pos p1, real p2);


int Cacu_Orbit(orbit p, real M0, real T0, real T, Pos*X, Pos*V);
void BinaryCollision(Body* MS, Pos* XS, Pos*VS, uint s, uint o);
Pos Newtun(Body* MS,Pos* XS, Pos*VS,uint s, uint o);
Pos ParmentNewtun(Body* MS, Pos* XS, Pos*VS, uint s, uint o);
Pos EIHFunc(Body* MS, Pos* XS, Pos*VS, uint s, uint o, uint nbody);
void Kernel(real t, real* x, real* y,uint n, Body*P);

void InitRungeKutta7(uint ny);
void RungeKutta7Interation(void(*f)(real, real*, real*, uint, Body*), Body*P, real*y, uint ny, real*t, real step, real*err);

void UninitRungeKutta7();




#endif