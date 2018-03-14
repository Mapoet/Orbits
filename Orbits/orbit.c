
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include"orbit.h"

uint ndem = NDEM;
GravityType GType = Gravity_EIH;
real * temp7[25];


real sprod(Pos p1, Pos p2)
{
	real result = 0.;
	for (uint i = 0; i < ndem; i++)
		result += p1.pos[i] * p2.pos[i];
	return result;
}
Pos vprod(Pos p1, Pos p2)
{
	Pos result;
	for (uint i = 0; i < ndem; i++)
		result.pos[i] = p1.pos[(i + 1) % ndem] * p2.pos[(i - 1 + ndem) % ndem] - p2.pos[(i + 1) % ndem] * p1.pos[(i - 1 + ndem) % ndem];
	return result;
}
Pos norms(Pos p)
{
	real aa = sqrt(sprod(p, p));
	if (fabs(aa) < 1e-10)
		return p;
	struct pos result;
	for (uint i = 0; i < ndem; i++)result.pos[i] = p.pos[i] / aa;
	return result;
}
real pabs(Pos p)
{
	return sqrt(sprod(p, p));
}
Pos ppp(Pos p1, Pos p2)
{
	Pos result;
	for (uint i = 0; i < ndem; i++)result.pos[i] = p1.pos[i] + p2.pos[i];
	return result;
}
Pos psp(Pos p1, Pos p2)
{
	Pos result;
	for (uint i = 0; i < ndem; i++)result.pos[i] = p1.pos[i] - p2.pos[i];
	return result;
}
Pos pmp(Pos p1, Pos p2)
{
	Pos result;
	for (uint i = 0; i < ndem; i++)result.pos[i] = p1.pos[i] * p2.pos[i];
	return result;
}
Pos pdp(Pos p1, Pos p2)
{
	Pos result;
	for (uint i = 0; i < ndem; i++)result.pos[i] = p1.pos[i] / p2.pos[i];
	return result;
}
Pos ppv(Pos p1, real p2)
{
	Pos result;
	for (uint i = 0; i < ndem; i++)result.pos[i] = p1.pos[i] + p2;
	return result;
}
Pos psv(Pos p1, real p2)
{
	Pos result;
	for (uint i = 0; i < ndem; i++)result.pos[i] = p1.pos[i] - p2;
	return result;
}
Pos pmv(Pos p1, real p2)
{
	Pos result;
	for (uint i = 0; i < ndem; i++)result.pos[i] = p1.pos[i] * p2;
	return result;
}
Pos pdv(Pos p1, real p2)
{
	Pos result;
	for (uint i = 0; i < ndem; i++)result.pos[i] = p1.pos[i] / p2;
	return result;
}
Pos vpp(real p1, Pos p2)
{
	Pos result;
	for (uint i = 0; i < ndem; i++)result.pos[i] = p1 + p2.pos[i];
	return result;
}
Pos vsp(real p1, Pos p2)
{
	Pos result;
	for (uint i = 0; i < ndem; i++)result.pos[i] = p1 - p2.pos[i];
	return result;
}
Pos vmp(real p1, Pos p2)
{
	Pos result;
	for (uint i = 0; i < ndem; i++)result.pos[i] = p1 * p2.pos[i];
	return result;
}
Pos vdp(real p1, Pos p2)
{
	Pos result;
	for (uint i = 0; i < ndem; i++)result.pos[i] = p1 / p2.pos[i];
	return result;
}

int Cacu_Orbit(orbit p, real M0,real T0, real T, Pos*X, Pos*V){
	real Mk =M0+p.w*(T-T0);
	real Mkdot = p.w;
	real pi = atan2(0, -1);
	Mk = fmod(Mk, 2 * pi);
	real z = sin(Mk);
	real d = p.e*z;
	real ee = p.e*p.e; //pow(s.e,2.);
	real w = p.a*p.e*cos(Mk); //pow(s.a_,2.)*s.e*cos(Mk);
	real c = ee*d*(1.0 - (1.5*z*z));
	real Ek = Mk + d + w + c;
	real dEk = 0;
	int loop = 0;
	while (fabs(Ek-dEk)>1.0e-10)
	{
		dEk = Ek;
		Ek = dEk + (Mk - dEk + p.e * sin(dEk)) / (1.0 - p.e*cos(dEk));
		if (loop++>200)
		{
			return -1;
		}
		//Ek = dEk + p.e * sin(dEk);
	}
	real cvk = cos(Ek) - p.e;
	real svk = sqrt(1.0 - ee) * sin(Ek);

	real vk = atan2(svk, cvk);                   //真近点角
	/*if(vk < 0.0 ) vk += 2.0*PI; */

	real phik = vk + 0;//升交点角距
	real ik = 0;//改正后的轨道倾角
	real rk = p.a*(1.0 - p.e*cos(Ek));//改正后的向径
	real uk = phik;//改正后真近点角
	real omegak =0;//改正后升交点经度

	real csal = cos(phik*2.0);
	real ssal = sin(phik*2.0);

	real suk = sin(uk);
	real cuk = cos(uk);
	real sik = sin(ik);
	real cik = cos(ik);
	real slk = sin(omegak);
	real clk = cos(omegak);

	real Ekdot = Mkdot / (1.0 - p.e*cos(Ek));
	real sqe = sqrt((1.0 - ee));
	real fkdot = (sqe*Ekdot) / (1.0 - p.e*cos(Ek));


	real ukdot = fkdot;
	real rkdot = p.a*p.e*sin(Ek)*Ekdot - 2.0*0*ssal*fkdot + 2.0*0*csal*fkdot;
	real ikdot = 0;
	real elkdot =0;

	X->pos[0] = rk*(clk*cuk - slk*cik*suk);//卫星在地心坐标系中的位置
	X->pos[1] = rk*(slk*cuk + clk*cik*suk);
	X->pos[2] = rk*(sik*suk);

	real xdotor = -suk;
	real ydotor = cuk ;
	real zdotor = 0;

	V->pos[0] = ukdot * rk*(clk*xdotor - slk*cik*ydotor);// +rkdot*X->x;//卫星在地心坐标系中的位置
	V->pos[1] = ukdot * rk*(slk*xdotor + clk*cik*ydotor);// +rkdot*X->y;
	V->pos[2] = ukdot * rk*(sik*ydotor);// +rkdot*X->z;
	return 0;
}

void BinaryCollision(Body* MS, Pos* XS, Pos*VS, uint s, uint o)
{
	real RR = MS[s].R + MS[o].R;
	Pos X12 = psp(XS[o], XS[s]);
	real L = pabs(X12);
	if (L>RR)return;
	struct pos NN = norms(X12);
	Pos VN1 = vmp(sprod(VS[s], NN), NN), VN2 = vmp(sprod(VS[o], NN), NN);
	real L1 = pabs(VN1), L2 = pabs(VN2);
	real k1 = ((MS[s].M - MS[o].M)*L1 + 2 * MS[o].M*L2) / (MS[s].M + MS[o].M);
	real k2 = (2 * MS[s].M*L1 + (MS[o].M - MS[s].M)*L2) / (MS[s].M + MS[o].M);
	VS[s] = ppp(psp(VS[s], VN1), vmp(k1, NN));
	VS[o] = ppp(psp(VS[o], VN2), vmp(k2, NN));
	//*V1 = *V2 = cdiv_pv(cplu_pp(cmul_vp(Ms.M, *V1), cmul_vp(Mo.M, *V2)), Ms.M + Mo.M);
}

Pos Newtun(Body* MS, Pos* XS, Pos*VS, uint s, uint o){

	Pos R, R0, F;
	real r, r2;
	R = psp(XS[o], XS[s]); R0 = norms(R); r2 = sprod(R, R); r = sqrt(r2);
	F = vmp(G*MS[o].M / r2, R0);//GM/r^2*r0;
	return F;
}

Pos ParmentNewtun(Body* MS, Pos* XS, Pos*VS, uint s, uint o){

	Pos R, R0,V,HJ, F/*,F1,F11,F12,F13*/;
	real r, r2, v, v2, Rd, mu = MS[s].M*MS[o].M / (MS[s].M + MS[o].M), nu = MS[s].M*MS[o].M / (MS[s].M + MS[o].M) / (MS[s].M + MS[o].M);
	
	R = psp(XS[o], XS[s]); R0 = norms(R); r2 = sprod(R, R); r = sqrt(r2);
	V = psp(VS[o], XS[s]); Rd = sprod(R0, V); v2 = sprod(V, V); v = sqrt(v2);
	HJ = vprod(R, V);

	//F=GM1*(R/r^3*(1+2*(2+nu)/r-(1+3*nu)*v2+3*nu/2*Rd^2)+2*(2-nu)*Rd/r^2*V-R/r^3*(9/r^2-2*Rd^2/r)-2*Rd*V/r^3);

	if (fabs(r) < 1e-6){
		r = 1;
		r2 = 1;
	}
	F = vmp(G*MS[o].M*(1 + (2 * (2 + nu) / r - (1 + 3 * nu)*v2 + 3 * nu / 2 * Rd*Rd) / c2 ) / r2, R0);
	F = ppp(F, vmp(G*MS[o].M * 2 * (2 - nu)*Rd / r2 / c2, V));
	return F;
}
Pos EIHFunc(Body* MS, Pos* XS, Pos*VS, uint s, uint o, uint nbody){
	Pos R12=psp(XS[o],XS[s]), R23, R13;
	real r12=pabs(R12), r23, r13;
	Pos N12=pdv(R12,r12), N23, N13;
	Pos V21=psp(VS[s],VS[o]);
	Pos F;
	real k1=0, k2=0, k3=0;
	k1 = 1 + (sprod(VS[s], VS[s]) + 2*sprod(VS[o], VS[o]) - 4*sprod(VS[s], VS[o])-3.*pow(sprod(N12,VS[o]),2.)/2) / c2;
	F = pmv(V21, G*MS[o].M / c2 / r12 / r12*(4 * sprod(N12, VS[s]) - 3 * sprod(N12, VS[o])));
	for (uint i = 0; i < nbody; i++){
		if (s != i)
		{
			R13 = psp(XS[i], XS[s]);
			r13 = pabs(R13);
			N13 = pdv(R13, r13);
			k1 -= 4 * G*MS[i].M / c2 / r13;
		}
		if (o != i)
		{
			R23 = psp(XS[i], XS[o]);
			r23 = pabs(R23);
			N23 = pdv(R23, r23);
			k1 -= G*MS[i].M/ c2 / r23*(1-r12/r23*sprod(N12,N23)/2);
			F = ppp(F, pmv(N23,7./2.* G*G*MS[o].M*MS[i].M / c2 / r12 / r23 / r23));
		}
	}
	F = ppp(F, vmp(G*MS[o].M / r12 / r12*k1, N12));
	return F;
}
void Kernel(real t, real* x, real* y, uint n, Body*P){
	if(y==NULL)y=(real*)malloc(sizeof(real)*n);
	if (y == NULL){ printf("Out of Memory \n "); system("pause"); exit(1); }
	uint k = (uint)(n / (2*ndem));
	Pos xs[NPLANET], vs[NPLANET], fs[NPLANET];
	// get X,V;
	for (uint i = 0; i < k; i++)
	{
		for (uint j = 0; j < ndem; j++)
		{
			xs[i].pos[j] = x[i * ndem + j];
			vs[i].pos[j] = x[(i + k) * ndem + j];
		}
	}
	// binarycollision
	for (uint i = 0; i < k; i++)
	for (uint j = 0; j < k; j++)
	if (i != j)
	  BinaryCollision(P, xs, vs, i, j);
	// cacu force
	for (unsigned int i = 0; i < k; i++)
	{
		for (uint j = 0; j < ndem; j++)fs[i].pos[j] = 0.;
		for (unsigned int j = 0; j < k; j++)
		if (i != j)
		switch (GType)
		{
		case Gravity_Newtun:	fs[i] = ppp(fs[i], Newtun(P, xs, vs, i, j)); break;
		case Gravity_PN:		fs[i] = ppp(fs[i], ParmentNewtun(P, xs, vs, i, j)); break;
		case Gravity_EIH:		fs[i] = ppp(fs[i], EIHFunc(P, xs, vs, i, j, k)); break;
		default:break;
		}
	}
	//set V,F;
	for (unsigned int i = 0; i < k; i++)
	{
		for (uint j = 0; j < ndem; j++)
		{
			y[i * ndem + j] = vs[i].pos[j];
			y[(i + k) * ndem + j] = fs[i].pos[j];
		}
	}
}

void InitRungeKutta7(uint ny){
	for (unsigned int i = 0; i < 25; i++){
		temp7[i] = (real*)malloc(sizeof(real)*ny);
		if (temp7[i] == NULL){ printf("Out of Memory \n "); exit(1); }
	}
}

void RungeKutta7Interation(void(*f)(real, real*, real*, uint, Body*), Body*P, real*y, uint ny, real*t, real step, real*err){
		f(*t, y, temp7[0], ny, P);
		for (unsigned int i = 0; i < ny; i++)temp7[13][i] = y[i] + 2 * temp7[0][i] * step / 27.0;
		f(*t + 2 * step / 27.0, temp7[13], temp7[1], ny, P);
		for (unsigned int i = 0; i < ny; i++)temp7[14][i] = y[i] + (temp7[0][i] + 3 * temp7[1][i]) * step / 36.0;
		f(*t + step / 9.0, temp7[14], temp7[2], ny, P);
		for (unsigned int i = 0; i < ny; i++)temp7[15][i] = y[i] + (temp7[0][i] + 3 * temp7[2][i]) * step / 24.0;
		f(*t + step / 6.0, temp7[15], temp7[3], ny, P);
		for (unsigned int i = 0; i < ny; i++)temp7[16][i] = y[i] + (20 * temp7[0][i] + (-temp7[2][i] + temp7[3][i])*75.0) * step / 48.0;
		f(*t + 5 * step / 12.0, temp7[16], temp7[4], ny, P);
		for (unsigned int i = 0; i < ny; i++)temp7[17][i] = y[i] + (temp7[0][i] + 5 * temp7[3][i] + 4 * temp7[4][i]) * step / 20.0;
		f(*t + step / 2.0, temp7[17], temp7[5], ny, P);
		for (unsigned int i = 0; i < ny; i++)temp7[18][i] =y[i] + (-25 * temp7[0][i] + 125 * temp7[3][i] - 260 * temp7[4][i] + 250 * temp7[5][i]) * step / 108.0;
		f(*t + 5 * step / 6.0, temp7[18], temp7[6], ny, P);
		for (unsigned int i = 0; i < ny; i++)temp7[19][i] = y[i] + (93 * temp7[0][i] + 244 * temp7[4][i] - 200 * temp7[5][i] + 13 * temp7[6][i]) * step / 900.0;
		f(*t + step / 6.0, temp7[19], temp7[7], ny, P);
		for (unsigned int i = 0; i < ny; i++)temp7[20][i] = y[i] + (180 * temp7[0][i] - 795 * temp7[3][i] + 1408 * temp7[4][i] - 1070 * temp7[5][i] + 67 * temp7[6][i] + 270 * temp7[7][i]) * step / 90.0;
		f(*t + 2 * step / 3.0, temp7[20], temp7[8], ny, P);
		for (unsigned int i = 0; i < ny; i++)temp7[21][i] = y[i] + (-455 * temp7[0][i] + 115 * temp7[3][i] - 3904 * temp7[4][i] + 3110 * temp7[5][i] - 171 * temp7[6][i] + 1530 * temp7[7][i] - 45 * temp7[8][i]) * step / 540.0;
		f(*t + step / 3.0, temp7[21], temp7[9], ny, P);
		for (unsigned int i = 0; i < ny; i++)temp7[22][i] = y[i] + (2383 * temp7[0][i] - 8525 * temp7[3][i] + 17984 * temp7[4][i] - 15050 * temp7[5][i] + 2133 * temp7[6][i] + 2250 * temp7[7][i] + 1125 * temp7[8][i] + 1800 * temp7[9][i]) * step / 4100.0;
		f(*t + step, temp7[22], temp7[10], ny, P);
		for (unsigned int i = 0; i < ny; i++)temp7[23][i] =y[i] + (60 * temp7[0][i] - 600 * temp7[5][i] - 60 * temp7[6][i] + (-temp7[7][i] + temp7[8][i] + 2 * temp7[9][i])*300.0) * step / 4100.0;
		f(*t, temp7[23], temp7[11], ny, P);
		for (unsigned int i = 0; i < ny; i++)temp7[24][i] = y[i] + (-1777 * temp7[0][i] - 8525 * temp7[3][i] + 17984 * temp7[4][i] - 14450 * temp7[5][i] + 2193 * temp7[6][i] + 2550 * temp7[7][i] + 825 * temp7[8][i] + 1200 * temp7[9][i] + 4100 * temp7[11][i]) * step / 4100.0;
		f(*t + step, temp7[24], temp7[12], ny, P);
		for (unsigned int i = 0; i < ny; i++)y[i] =y[i] + (272 * temp7[5][i] + 216 * (temp7[6][i] + temp7[7][i]) + 27 * (temp7[8][i] + temp7[9][i]) + 41 * (temp7[11][i] + temp7[12][i]))*step / 840.0;
		*t = *t + step;
		if (err != NULL)for (unsigned int i = 0; i < ny; i++)err[i] = 41 / 810.0*(temp7[0][i] + temp7[10][i] + temp7[11][i] - temp7[12][i])*step;
}

void UninitRungeKutta7(){
	for (unsigned int i = 0; i < 25; i++)if (temp7[i] != NULL)free(temp7[i]);
}








//
//
//int main2(){
//	//real es =6.8567e-4 , ec =9.171e-5 ;
//	//real e = sqrt(es*es + ec*ec);
//	//real w = 2 * 3.141592654 / 1.629401788; atan2(es, ec);
//
//
//	//real e = 3.53561955e-2;
//	//real w = 2 * 3.141592654 / 327.257541;
//
//	//orbit p = { 118.043, e, w };
//	//Pos*X = (Pos*)malloc(sizeof(Pos)), *V = (Pos*)malloc(sizeof(Pos));
//	//FILE*fid = fopen("data.xyz", "w");
//	//FILE*fid2 = fopen("data2.xyz", "w");
//	//for (real t = 0; t < 500; t += 0.005)
//	//{
//	//	if (Cacu_Orbit(p, 0,0, t, X, V) != -1){
//	//		fprintf(fid, "time:%8.2lf,X:%19.8lf,Y:%19.8lf,Z:%19.8lf,VX:%19.8lf,VY:%19.8lf,VZ:%19.8lf\n", t, X->x, X->y, X->z, V->x, V->y, V->z);
//	//		fprintf(fid2, "%8.2lf,%19.8lf,%19.8lf,%19.8lf,%19.8lf,%19.8lf,%19.8lf\n", t, X->x, X->y, X->z, V->x, V->y, V->z);
//	//	}
//	//}
//	//fclose(fid);
//	//fclose(fid2);
//	//free(X);
//	//free(V);
//
//
//	/************************************************************************/
//	real tstart = 0, tend = DS *365*10, step = DS/12.;
//	/* SOLAR SYSTEM */
//	Pos XS[N] = {//  KM
//		{ 424813.36246171698,-109447.99609616444,-69820.229594572796 },//SUN
//		{-24322141.390958026, 132932582.20558938, 57606068.782710962 },//EARTH
//		{-24056684.890056696, 133191497.38471816, 57699588.623732656 },//MOON
//		{ 50423260.321339965,-26352384.011056598,-19271739.745880976 },//MERCURY
//		{ 81994975.847352982,-63880198.689242616,-33923711.283606485 },//VENUS
//		{ 203090292.28391004,-36203216.805950850,-22096437.036853414 },//MARS
//		{-557237566.54076004, 516845962.18155581, 235088268.18221751 },//JUPITER
//		{-808841416.06047797,-1167382274.8029778,-447366151.95612448 },//SATURN
//		{ 2888274214.4245334, 734209281.07829726, 280713132.53061473 },//URANUS
//		{ 4118322955.9381952,-1602490914.8978064,-758439864.16495633 },//NEPTUNE
//		{ 1106960135.8991766,-4456999238.7860842,-1724416620.9367239 },//PLUTO
//		//{-557207566.54076004, 516875962.18155581, 235038268.18221751 },//木卫一
//		//{-557267566.54076004, 516815962.18155581, 235138268.18221751 },//木卫二
//		{-24587672.374246156, 132673655.29061124, 57512543.856403052 },//ADDONE
//		{-59322178.632151421,-165932576.33766468, 68606066.240067832 }//ADDTWO
//	},
//		VS[N] = {//  KM/DAY
//			{ 594.93008957156087, 724.30051232037329, 298.13563569488031 },//SUN
//			{-2577588.9412655965,-406139.22073434829,-175985.06326830230 },//EARTH
//			{-2639023.6618419583,-344994.68674991967,-156919.89812121989 },//MOON
//			{ 1428354.6663402223, 3369331.3432997214, 1651741.3769841387 },//MERCURY
//			{ 1983165.3002298109, 2105096.1989612179, 821674.89286298235 },//VENUS
//			{ 504629.04402558395, 2032258.4346736313, 918500.10449742945 },//MARS
//			{-819078.34621351853,-685243.98184525024,-273772.45790289348 },//JUPITER
//			{ 654921.19579049514,-411144.82109271438,-198020.10638509810 },//SATURN
//			{-158668.36935971351, 494142.41357607668, 218665.31397146420 },//URANUS
//			{ 182479.84018068071, 403563.58303751971, 160637.70152958087 },//NEPTUNE
//			{ 466786.90393313725, 63633.395981461632,-120783.82925493439 },//PLUTO
//			//{-15078.34621351853,-80243.98184525024,-70772.45790289348 },//木卫一
//			//{-15078.34621351853,-80243.98184525024,-70772.45790289348 },//木卫二
//			{-2516154.2206892353,-467283.75471877721,-195050.22841538578 },//ADDONE
//			{-1839023.6618419583,-244994.68674991967, 96919.89812121989 }//ADDTWO
//	};
//	/*END SOLAR SYSTEM */
//	/*SUN EARTH MOON SYSTEM*/
//	//Pos XS[N] = {//  KM
//	//	{ 424809.99457713118, -109452.09637541689, -69821.917346614224 },//SUN
//	//	{ -24322178.632151421, 132932576.33766468, 57606066.240067832 },//EARTH
//	//	{ -24056684.890056696, 133191497.38471816, 57699588.623732656 },//MOON
//	//	{ -24587672.374246156, 132673655.29061124, 57512543.856403052 },//ADDONE
//	//	{ -59322178.632151421, -165932576.33766468, 68606066.240067832 }//ADDTWO
//	//},
//	//	VS[N] = {//  KM/DAY
//	//		{ 594.93008957156087, 724.30051232037329, 298.13563569488031 },//SUN
//	//		{ -2577588.9412655965, -406139.22073434829, -175985.06326830230 },//EARTH
//	//		{ -2639023.6618419583, -344994.68674991967, -156919.89812121989 },//MOON
//	//		{ -2516154.2206892353, -467283.75471877721, -195050.22841538578 },//ADDONE
//	//		{ -1839023.6618419583, -244994.68674991967, 96919.89812121989 }//ADDTWO
//	//};
//	/*END SUN EARTH MOON SYSTEM*/
//	real **y = NULL, y0[6 * N], err[6 * N];
//	const char* type = "Newtun";
//	char fif[50] = { 0 }, fnbf[50] = { 0 };
//	for (unsigned int n = 2; n <= 13; n++)
//	{
//		sprintf(fif, "%s%s%d.xyz", type, "info", n);
//		FILE*fnbinfo = fopen(fif, "w");
//		for (unsigned int i = 0; i < n; i++)
//		{
//			y0[i * 3] = XS[i].x*1e3;
//			y0[i * 3 + 1] = XS[i].y*1e3;
//			y0[i * 3 + 2] = XS[i].z*1e3;
//			y0[(i + n) * 3] = VS[i].x*1e3 / DS;
//			y0[(i + n) * 3 + 1] = VS[i].y*1e3 / DS;
//			y0[(i + n) * 3 + 2] = VS[i].z*1e3 / DS;
//			//y0[i * 3] = XS[i].x*1e1;
//			//y0[i * 3 + 1] = XS[i].y*1e1;
//			//y0[i * 3 + 2] = XS[i].z*1e1;
//			//y0[(i + n) * 3] = VS[i].x*1e4 / DS;
//			//y0[(i + n) * 3 + 1] = VS[i].y*1e4 / DS;
//			//y0[(i + n) * 3 + 2] = VS[i].z*1e4 / DS;
//			for (unsigned int j = 0; j < n; j++)
//			{
//				if (i != j){
//					fprintf(fnbinfo, "(%2.2d,%2.2d)->U:%40.15lf,UV:%40.15lf,V:%40.15lf\n",
//						i + 1, j + 1,
//						-G*M[i].M / abs_p(csub_pp(XS[j], XS[i])),
//						sqrt(2 * G*M[i].M / abs_p(csub_pp(XS[j], XS[i]))),
//						abs_p(csub_pp(VS[j], VS[i]))
//						);
//				}
//			}
//		}
//		fclose(fnbinfo);
//		RungeKutta7Interation(kernel, n * 6, y0, M, &y, tstart, tend, step, err);
//		unsigned int rkstep = (unsigned int)((tend - tstart) / step);
//		sprintf(fnbf, "%s%d.xyz", type, n);
//		FILE*fnb = fopen(fnbf, "w");
//		//fprintf(fnb, "Start:");
//		//for (unsigned int j = 0; j < 6 * N; j++)
//		//	fprintf(fnb, "%30.20lf", y0[j]);
//		//fprintf(fnb, "\n");
//		for (unsigned int i = 0; i < rkstep; i++)
//		{
//			fprintf(fnb, "%6d", i + 1);
//			for (unsigned int j = 0; j < 6 * n; j++)
//				fprintf(fnb, "%50.20lf", y[i][j]);
//			fprintf(fnb, "\n");
//		}
//		//fprintf(fnb, "Error:");
//		//for (unsigned int j = 0; j < 6 * N; j++)
//		//	fprintf(fnb, "%30.20lf", err[j]);
//		//fprintf(fnb, "\n");
//		fclose(fnb);
//		for (unsigned int i = 0; i < rkstep; i++)if (y[i] != NULL){ free(y[i]); y[i] = NULL; }
//		if (y != NULL){ free(y); y = NULL; }
//	}
//
//	/******************************************************************************/
//	//system("pause");
//	return 0;
//}