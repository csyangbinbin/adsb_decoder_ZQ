
#include <cmath>
#include <stdlib.h> 
#include <time.h>
#include "cpr.h"
#ifdef WIN32
#include <Windows.h>
#endif

#ifndef M_PI
#define M_PI ( 3.14159265358979323846)
#endif
 
#ifndef min
#define min(X,Y) ((X) < (Y) ? (X) : (Y))
#endif

#ifndef max
#define max(X,Y) ((X) > (Y) ? (X) : (Y))
#endif

bool float_value_equ(double a ,double b)
{
	const double min_flt= 1e-5 ;
	return fabs(a-b)<min_flt;
}

bool valid_location(cpr_loc_t loc) 
{
	return (!(float_value_equ(loc.lon ,-360.0) && float_value_equ(loc.lat,-360.0))) ; 
}


timestamp cpr_now()
{
#ifdef WIN32
return ::GetTickCount() ;
#else

    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);

    return (ts.tv_sec * 1000 + ts.tv_nsec / 1000000);

#endif
}


double   nz(int ctype)
{
	return 4 * latz - ctype ; 
}


double cpr_mod(double val,double modval)
{
	double 	_modulo = fmod(val,modval);
	if(val<0)  	{ 	
		_modulo =_modulo +modval ; 
	}
	return _modulo;
}

double  dlat(int ctype , int surface)
{
	double tmp = 0 ;

	if (surface == 1)
		tmp = 90.0 ;
	else
		tmp = 360.0 ;

	double  nzcalc = nz(ctype) ; 
	if (nzcalc == 0)
		return tmp ; 
	else
		return tmp / nzcalc ;
}

int    nl(double declat_in)
{
	if ( fabs(declat_in) >= 87.0 )
		return 1 ;

	double U = 1- cos(M_PI / (2* latz)) ; 
	double T = cos(M_PI/180.0 * fabs(declat_in)) ;
	return static_cast<int>(floor(2.0*M_PI* 1.0 / (  acos(1- U/(T*T))))) ;  ;  

}


double  dlon(double declat_in, int ctype, int surface)
{
	double tmp = 0 ; 

	if (surface==1)
		tmp = 90.0 ;
	else
		tmp = 360.0 ;

	int  nlcalc = max( (nl(declat_in)-ctype), 1) ; 
	return tmp / nlcalc ; 
}


double decode_lat(int enclat, int ctype, double my_lat, int surface)
{
	double tmp1 = dlat(ctype, surface) ; 
	double tmp2 = double(enclat) / pow(2.0,17) ;
	double j = floor(my_lat/tmp1) + floor(0.5 + ( cpr_mod(my_lat,tmp1) / tmp1) - tmp2) ; 
	return tmp1 * (j + tmp2) ; 
}

double  decode_lon(double declat, double enclon, int ctype,double  my_lon, int  surface)
{
	double tmp1 = dlon(declat, ctype, surface) ; 
	double tmp2 = double(enclon) / pow(2.0 , 17) ;
	double m = floor(my_lon / tmp1) + floor(0.5 + (cpr_mod(my_lon , tmp1) / tmp1) - tmp2) ; 

	return tmp1 * (m + tmp2) ; 
}


cpr_loc_t cpr_resolve_local(cpr_loc_t my_location, encoded_info encoded_location, int ctype, int surface)
{
	cpr_loc_t loc ; 

	loc.lat = decode_lat(encoded_location.encode_lat, ctype, my_location.lat, surface) ; 
	loc.lon = decode_lon(loc.lat, encoded_location.encode_lon, ctype, my_location.lon, surface) ; 

	return loc ; 
}


double clon(double lon)
{
	int tmp = static_cast<int>(lon) / 90 ; 

	return 90.0 * tmp ; 
}


#define K  ( pow(2.0,17) )

cpr_loc_t cpr_resolve_global(encoded_info even_pos, encoded_info odd_pos, cpr_loc_t mypos, int  mostrecent, int surface)
{
	//cannot resolve surface positions unambiguously without knowing receiver position

	double dlateven = dlat(0, surface);
	double dlatodd  = dlat(1, surface);

	double evenpos[2]  = { (double)even_pos.encode_lat , (double)even_pos.encode_lon} ;
	double oddpos[2]  = { (double)odd_pos.encode_lat , (double)odd_pos.encode_lon} ;;

	int  j = static_cast<int>(floor(((nz(1)*evenpos[0] - nz(0)*oddpos[0])/K) + 0.5)) ;  //latitude index

	double rlateven = dlateven * ( cpr_mod(j , nz(0)) + evenpos[0]/K) ; 
	double rlatodd  = dlatodd  * ( cpr_mod(j , nz(1)) + oddpos[0] /K);

	//limit to -90, 90
	if (rlateven > 270.0)
		rlateven -= 360.0 ; //rlat[0]
	if (rlatodd > 270.0)
		rlatodd -= 360.0 ;  //rlat[1]

	//This checks to see if the latitudes of the reports straddle a transition boundary
	//If so, you can't get a globally-resolvable location.

	if (nl(rlateven) != nl(rlatodd))
	{
		cpr_loc_t err = {-360,-360};
		return err ; 
	}


	double rlat = 0 ; 
	if (mostrecent == 0)
		rlat = rlateven ;
	else
		rlat = rlatodd ; 

	//disambiguate latitude
	if (surface==1)
		if (mypos.lat < 0)
			rlat -= 90 ; 

	double dl = dlon(rlat, mostrecent, surface) ; 
	int   nl_rlat = nl(rlat) ; 

	int   m = static_cast<int>(floor(((evenpos[1]*(nl_rlat-1.0)-oddpos[1]*nl_rlat)/K)+0.5)); //longitude index

	//when surface positions straddle a disambiguation boundary (90 degrees),
	//surface decoding will fail. this might never be a problem in real life, but it'll fail in the
	//test case. the documentation doesn't mention it.
	double enclon =0;
	if (mostrecent == 0)
		enclon = evenpos[1];
	else
		enclon = oddpos[1];

	double rlon = dl * (cpr_mod(m , (max( (nl_rlat-mostrecent) ,1))  ) + enclon/(double)K ) ; 

	//print "DL: %f nl: %f m: %f rlon: %f" % (dl, nl_rlat, m, rlon)
	//print "evenpos: %x, oddpos: %x, mostrecent: %i" % (evenpos[1], oddpos[1], mostrecent)

	if (surface==1)
	{
		//longitudes need to be resolved to the nearest 90 degree segment to the receiver.
		double wat = mypos.lon ; 
		if (wat < 0)
			wat += 360 ; 
		rlon += (clon(wat) - clon(rlon)) ; 
	}

	//limit to (-180, 180)
	if (rlon > 180)
		rlon -= 360.0 ; 

	cpr_loc_t ret = {rlon , rlat} ;

	return ret ; 
}


encoded_info cpr_encode(double lat, double lon, int ctype, int surface)
{
	double scalar = 0 ;
	if (surface==1)
		scalar = pow(2.0,19) ;
	else
		scalar = pow(2.0,17);

	//encode using 360 constant for segment size.
	double dlati = dlat(ctype, 0) ; 
	double yz = floor(scalar * (cpr_mod(lat , dlati)/dlati) + 0.5) ; 
	double rlat = dlati * ((yz / scalar) + floor(lat / dlati)) ; 

	//encode using 360 constant for segment size.
	double dloni = dlon(lat, ctype, 0) ;
	double xz = floor(scalar * ( cpr_mod(lon ,dloni)/dloni) + 0.5) ; 

	encoded_info info = { (int(yz)&0x0001FFFF) , (int(xz)&0x0001FFFF) , 0} ; 

	return info ;
}


cpr_decoder::cpr_decoder(cpr_loc_t my_location)
:m_my_location(my_location)
{

}
void cpr_decoder::set_location(cpr_loc_t my_location)
{
	m_my_location = my_location;
}

void cpr_decoder::delete_time_out(encoded_list_t& the_list , unsigned int time_diff) 
{
	timestamp  now_t = cpr_now() ; 
	list_iter it = the_list.begin();

	for(;it!=the_list.end();)
	{
		if(now_t - it->second.recv_time  > time_diff)
		{
			the_list.erase(it++);
		}
		else
			++it ; 

	}

}


void cpr_decoder::weed_poslists()
{
	//删除超时的信息，空中10S超时，地面25秒超时
	delete_time_out(evenlist ,	10*1000) ;
	delete_time_out(oddlist ,	10*1000) ;
	delete_time_out(evenlist_sfc ,25*1000) ;
	delete_time_out(oddlist_sfc ,25*1000) ;

}

void cpr_decoder::reset(int icao24)
{
evenlist.erase(icao24) ; 
oddlist.erase(icao24) ;
evenlist_sfc.erase(icao24) ;
oddlist_sfc.erase(icao24) ; 

}
cpr_loc_t  cpr_decoder::decode(int icao24, unsigned  int encoded_lat,unsigned  int encoded_lon,unsigned  int cpr_format, int surface)
{

	encoded_list_t* oddlist_ptr = NULL ; 
	encoded_list_t* evenlist_ptr = NULL ; 

	encoded_info info ; 
	info.encode_lat = encoded_lat ; 
	info.encode_lon = encoded_lon ; 
	info.recv_time = cpr_now();


	if(surface ==1)
	{
		oddlist_ptr = &oddlist_sfc ; 
		evenlist_ptr = &evenlist_sfc ;
	}
	else
	{
		oddlist_ptr = &oddlist;
		evenlist_ptr = &evenlist ; 
	}

	if(cpr_format==1){
		if (surface ==1 ){
			oddlist_sfc[icao24] = info ; 
		}
		else{
			oddlist[icao24] = info ;
		}
	}
	else{
		if(surface ==1){
			evenlist_sfc[icao24]=info;
		}
		else{
			evenlist[icao24] = info ;
		}
	}

	//okay, let's traverse the lists and weed out those entries that are older than 10 seconds
	weed_poslists() ; 

	bool in_even_list  = evenlist_ptr->find(icao24)!=evenlist_ptr->end();
	bool in_odd_list   = oddlist_ptr->find(icao24)!=oddlist_ptr->end();

	cpr_loc_t real_loc ; 

	if (in_even_list && in_odd_list) //全局解码
	{
 
// 		timestamp odd_time = oddlist_ptr->find(icao24)->second.recv_time ; 
// 		timestamp even_time = evenlist_ptr->find(icao24)->second.recv_time ; 
// 		int newer = (odd_time - even_time )>0?1:0  ; //figure out which report is newer

		encoded_info even_pos =  evenlist_ptr->find(icao24)->second ; 
		encoded_info odd_pos  =  oddlist_ptr->find(icao24)->second;

		real_loc = cpr_resolve_global(even_pos, odd_pos, m_my_location, /*newer*/cpr_format, surface) ;  //do a global decode

		if(!valid_location(real_loc))
		{
		reset(icao24) ; 
		}
	}
// 	else if(in_even_list || in_odd_list) //局部解码
// 	{
// 			#ifdef _DEBUG
// 				LOG_DBG("局部解码");
// 			#endif
// 
// 		encoded_info local_info = {encoded_lat ,encoded_lon , 0 } ;
// 		cpr_loc_t local_loc = cpr_resolve_local(m_my_location ,local_info ,  cpr_format , surface) ;
// 		return local_loc ;
// 
// 	}
	else
	{
		cpr_loc_t err_loc = {-360.0 , -360.0} ;
		return err_loc; 
	}

	return real_loc  ;
}
