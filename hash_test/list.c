/**  
 *auto create by python,according to doxygen 
*/ 
#include "../header_file.h"

#include "./list.h"
typedef struct
{
    char *key;
    int key_len;
    char *value;
    struct list_head list;
} work_queue_s;

work_queue_s fifo_queue;
void add(char *key,char *value)
{
    work_queue_s *work_entry=malloc(sizeof(work_queue_s));
    work_entry->key=strdup(key);
    work_entry->key_len=strlen(key);
    work_entry->value=strdup(value);
    list_add_tail(&work_entry->list,&fifo_queue.list);
    //printf("add %s\n",key);
}
char *lookup_key(char *key)
{
    int key_len=strlen(key);
    struct list_head *list_queue=&fifo_queue.list;
    struct list_head *list_entry;
    work_queue_s *entry;
    list_for_each(list_entry,list_queue)
    {
        entry = container_of(list_entry, work_queue_s, list);
        if((key_len == entry->key_len) &&  !strcmp(entry->key,key))
        {
            return entry->value;
        }
    }
    //printf("look up fail %s\n",key);
    return NULL;

}
void add_key(char *key,char *value)
{
    if(NULL == lookup_key(key))
    {
        add(key,value);
    }
}
void list_init(void)
{
    INIT_LIST_HEAD(&fifo_queue.list);
}

