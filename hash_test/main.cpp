/**  
 *auto create by python,according to doxygen 
*/ 


#include "../header_file.h"
#include<iostream>
using namespace std;;







#include "BasicHashTable.hh" 
class MediaLookupTable {
public:
    MediaLookupTable();
    ~MediaLookupTable();

    HashTable* fTable;
};

MediaLookupTable::MediaLookupTable(void)
  : fTable(HashTable::create(STRING_HASH_KEYS)) {
}
MediaLookupTable::~MediaLookupTable() {
  delete fTable;
}





extern "C" {
void list_init(void);
char *lookup_key(char *key);
void add_key(char *key,char *value);
}


MediaLookupTable test_table;
#define LOOP_TEST 100
#define HASH
int main(int argc,char *argv[])
{
    int i=0;
    char key_buf[32]="";
    char value_buf[32]="";
    double CurTimeMs = 0;
	double CurTimeMs2 = 0;
	list_init();
	struct timeval tvCurTime;
	gettimeofday(&tvCurTime, NULL);
	CurTimeMs = tvCurTime.tv_sec*1000 + (1.0 * tvCurTime.tv_usec)/1000;

    test_table.fTable->Add("test",(char *)"55");
    printf("hash:%s \n",test_table.fTable->Lookup("test"));
	add_key("test","55");
    printf("hash:%s \n",lookup_key("test"));
    
    for(i=0;i<LOOP_TEST;i++)
    {
        unsigned key=random()%10000;
        unsigned value=random()%10000;
        snprintf(key_buf,sizeof(key_buf),"%d",key);
        snprintf(value_buf,sizeof(value_buf),"%d",value);
        #ifdef HASH
        test_table.fTable->Add(key_buf,value_buf);
        #else
        add_key(key_buf,value_buf);
        #endif
    }

    for(i=0;i<LOOP_TEST;i++)
     {
         unsigned key=random()%10000;
         snprintf(key_buf,sizeof(key_buf),"%d",key);
         #ifdef HASH
         test_table.fTable->Lookup(key_buf);
         #else
         lookup_key(key_buf);
         #endif
     }
     
    
    gettimeofday(&tvCurTime, NULL);
    CurTimeMs2 = tvCurTime.tv_sec*1000 + (1.0 * tvCurTime.tv_usec)/1000;
    printf("diff_time:%lf ms \n",(CurTimeMs2 - CurTimeMs));


    printf("numEntries:%d\n",test_table.fTable->numEntries());
    
    sync();
    sleep(1);
}
