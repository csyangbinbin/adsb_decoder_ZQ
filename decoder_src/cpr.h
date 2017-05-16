/********************************************************************
created:	2013/07/31
created:	31:7:2013   10:50
filename:	cpr.h
file path:	
file base:	cpr
file ext:	h
author:		ybb

purpose:	CPR解码算法
*********************************************************************/
#ifndef __FST_CPR_DE_ENCODE_INCLUDE__
#define __FST_CPR_DE_ENCODE_INCLUDE__
#include <map>


#define latz	(15.0)


typedef unsigned int timestamp ; 


typedef struct cpr_loc_t
{
	double lon ;
	double lat ;
}cpr_loc_t;

typedef struct encoded_info
{
	unsigned int  encode_lat ;
	unsigned int  encode_lon;
	timestamp recv_time ; //毫秒时间
}encoded_info;

timestamp cpr_now() ; 

cpr_loc_t cpr_resolve_local(cpr_loc_t my_location, encoded_info encoded_location, int ctype, int surface) ; 
cpr_loc_t cpr_resolve_global(encoded_info even_pos, encoded_info odd_pos, cpr_loc_t mypos, int  mostrecent, int surface) ;
encoded_info cpr_encode(double lat, double lon, int ctype, int surface) ; 
bool valid_location(cpr_loc_t loc) ; 



class cpr_decoder
{
public:
	cpr_decoder(cpr_loc_t my_location) ;
	cpr_loc_t get_location() const { return m_my_location ; }
	void set_location(cpr_loc_t my_location) ;
	void weed_poslists() ;
	cpr_loc_t  decode(int icao24, unsigned int encoded_lat, unsigned int encoded_lon, unsigned int cpr_format, int surface) ;
	void reset(int icao24);

private:
	typedef std::map<int,encoded_info>			encoded_list_t;
	typedef encoded_list_t::iterator			list_iter ;
	typedef encoded_list_t::const_iterator		const_list_iter ;
private:
	void delete_time_out(encoded_list_t& the_list , unsigned int time_diff) ; 

	cpr_loc_t m_my_location ;
	encoded_list_t evenlist ;
	encoded_list_t oddlist ;
	encoded_list_t evenlist_sfc ;
	encoded_list_t oddlist_sfc ;

	
} ;



#endif