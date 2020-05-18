#include "../header_file.h"


unsigned long get_file_size(const char *path)
{
	unsigned long filesize = -1;	
	struct stat statbuff;
	if(stat(path, &statbuff) < 0){
		return filesize;
	}else{
		filesize = statbuff.st_size;
	}
	return filesize;
}

int main(int argc,char *argv[])
{
	int cut_begin=atoi(argv[1]);
	int cut_end=atoi(argv[2]);
	int read_len=cut_end-cut_begin;
	FILE *fp=fopen(argv[3],"rb");
	if(fp==NULL)
	{
		return -1;
	}
	fseek(fp,cut_begin,SEEK_SET);
	if(cut_end<0)
	{
		read_len=get_file_size(argv[3])-cut_begin;
	}
	char *buf=malloc(read_len*sizeof(char));
	fread(buf,read_len,1,fp);
	fclose(fp);
	
	FILE *fp_out=fopen("./out","wb+");
	fwrite(buf,read_len,1,fp_out);
	fclose(fp_out);
}
