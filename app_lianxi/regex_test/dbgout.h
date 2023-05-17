#ifndef DBG_OUT_H 
#define DBG_OUT_H 

#define DBG_ERR   0
#define DBG_INFO  7

#define DBG_OUT_INFO  7
#define DBG_OUT_LEVEL 9

///#define FACE_DETECT
#define DBG_LEVEL 9		//all message
//#define DBG_LEVEL 6		//error message

#include <stdio.h>

#ifndef dbg_out
#define dbg_out(level, format, args...) \
	{ \
	if(level<=DBG_LEVEL){ \
		if(level==DBG_ERR){ \
			fprintf(stderr,#level":%s %s[%04d] " format, __FILE__, __FUNCTION__, __LINE__, ##args); \
			fflush(stderr); \
		}\
  		else{ \
  			fprintf(stdout,#level":%s %s[%04d] " format, __FILE__, __FUNCTION__, __LINE__, ##args); \
			fflush(stdout); \
		}\
	} \
	}

#endif

int  traceout( char* fmt, ...);
char* format_time(void);

#ifdef POLI
#define strace(format, args...) { traceout("[%-4d %s] " format "\n",__LINE__,__FUNCTION__, ##args); } 

#define trace(format, args...) { traceout("[%-4d %s] " format "\n",__LINE__,__FUNCTION__, ##args); } 
#else
#define strace(format, args...) { traceout("[%s %s +%-4d %s] " format "\n", format_time(), __FILE__,__LINE__,__FUNCTION__, ##args); } 

#define trace(format, args...) { traceout("[%s %s +%-4d %s] " format "\n", format_time(), __FILE__,__LINE__,__FUNCTION__, ##args); } 

#endif

#endif
