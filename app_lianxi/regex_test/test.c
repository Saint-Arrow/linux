#include "../header_file.h"
#include "dbgout.h"
#include <regex.h>

//error buf
char errbuf[1024];

//init..
regex_t reg;
const char *pattern="h{3,5}.*@.{5}..*";
int cflags= REG_EXTENDED;
/*
REG_EXTENDED 
REG_ICASE
REG_NOSUB
REG_NEWLINE
*/

//match input..
char *bematch = "hhhericchd@gmail.com";
//match condition..
int eflags=0;
/*
REG_NOTBOL
REG_NOTEOL
*/
//match output..
#define nm 10
regmatch_t pmatch[nm];
char match[100];
int main(int argc,char *argv[])
{
	int err=0,i;
	if(regcomp(&reg,pattern,cflags) < 0){
		regerror(err,&reg,errbuf,sizeof(errbuf));
		trace("err:%s\n",errbuf);
	}

	err = regexec(&reg,bematch,nm,pmatch,eflags);
	if(err == REG_NOMATCH){
		trace("no match\n");
		exit(-1);
	}else if(err){
		regerror(err,&reg,errbuf,sizeof(errbuf));
		trace("err:%s\n",errbuf);
		exit(-1);
	}

	for(i=0;i<10 && pmatch[i].rm_so!=-1;i++){
		int len = pmatch[i].rm_eo-pmatch[i].rm_so;
		if(len){
			memset(match,'\0',sizeof(match));
			memcpy(match,bematch+pmatch[i].rm_so,len);
			trace("%s\n",match);
		}
	}
	return 0;


}
