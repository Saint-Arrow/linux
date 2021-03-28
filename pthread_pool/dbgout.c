#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/uio.h>
#include <fcntl.h>
#include <pthread.h>
#include <errno.h>
#include <signal.h>
#include <stdarg.h>

#include "dbgout.h"

#define VHD_LOG_FILE_PATH "/dev/vhd.log"

#define VHD_LOG_FIZE_MAXSIZE (10 * 1024 * 1024)

static int trace_inited = 0;
static FILE * tracefp = NULL;

char* format_time(void)
{
	struct timeval tv;
	static char buf[23];

	memset(buf, 0, sizeof(buf));

	// clock time
	if (gettimeofday(&tv, NULL) == -1) {
		return buf;
	}

	// to calendar time
	struct tm* tm;
	if ((tm = localtime(&tv.tv_sec)) == NULL) {
		return buf;
	}
#ifdef POLI
	snprintf(buf, sizeof(buf),
			"%02d:%02d:%02d.%03d",
			tm->tm_mday,
			tm->tm_hour, tm->tm_min, tm->tm_sec,
			(int)(tv.tv_usec / 1000));
#else
	snprintf(buf, sizeof(buf),
			"%d-%02d-%02d %02d:%02d:%02d.%03d",
			1900 + tm->tm_year, 1 + tm->tm_mon, tm->tm_mday,
			tm->tm_hour, tm->tm_min, tm->tm_sec,
			(int)(tv.tv_usec / 1000));

#endif
#if 0
	snprintf(buf, sizeof(buf),"s:06%u us:%06u",(unsigned int)tv.tv_sec,(unsigned int)tv.tv_usec);
#endif
	return buf;
}

int  traceout( char* fmt, ...)
{
	va_list args;
	va_list args_cp;
	struct stat tracelogstat;
	long offset;
	int ret;

	if ( trace_inited == 0 ){
		if ( -1 == stat(VHD_LOG_FILE_PATH,&tracelogstat)){
			tracefp = fopen(VHD_LOG_FILE_PATH,"w+");
		}
		else {
			if ( tracelogstat.st_size > VHD_LOG_FIZE_MAXSIZE ){
				tracefp = fopen(VHD_LOG_FILE_PATH,"r+");
			}
			else{
				tracefp = fopen(VHD_LOG_FILE_PATH,"a");
			}
		}
		trace_inited = 1;
	}

	va_start(args, fmt);
	va_copy(args_cp,args);
	vprintf(fmt, args_cp);
	va_end(args_cp);
	vfprintf(tracefp,fmt, args);
	va_end(args);

	fflush( stdout); 
	fflush( tracefp ); 

	offset = ftell(tracefp);
//	dbg_out(DBG_ERR, "offset:%d\n", offset);

	if ( -1 == stat(VHD_LOG_FILE_PATH,&tracelogstat)){
		tracefp = fopen(VHD_LOG_FILE_PATH,"w+");
		dbg_out(DBG_ERR, "stat ERR\n");
	}
	else {
		//if ( tracelogstat.st_size > VHD_LOG_FIZE_MAXSIZE ){
		if ( offset > VHD_LOG_FIZE_MAXSIZE ){
		//	rewind ( tracefp );
#if 1
                        fclose(tracefp);
                        trace_inited = 0;
#else
			ret = fseek(tracefp, 0, SEEK_SET);
			if (ret != 0)
			{
				dbg_out(DBG_ERR, "fseek ERR\n");
			}
#endif
			//dbg_out(DBG_ERR, " MAXSIZE ERR\n");
		}
	}

	return 0;
}

