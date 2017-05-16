#ifndef __ADSB_DEBUG_INCLUDE__
#define __ADSB_DEBUG_INCLUDE__

#ifdef _DEBUG
#define log_track( format, ...)	\
	printf(format "FILE:%s Line:%d\r\n" , ##__VA_ARGS__, __FILE__, __LINE__)


#define log_print( format, ...)	\
	printf(format "\r\n" , ##__VA_ARGS__)

#else	

#define log_track( format, ...) ((void)0)
#define log_print( format, ...) ((void)0)
#endif


#endif



