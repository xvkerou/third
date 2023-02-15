//Q1:realloc if oldly is needed?
//Q2:*[]down

#include "ramfs.h"
#include<string.h>
#include<stdlib.h>
/* modify this file freely */

typedef struct node {
	enum { FILE_NODE, DIR_NODE } type;
	struct node *dirents[65537];      // if it's a dir, there's subentries
	char *content;                   // if it's a file, there's data content
	int nrde;                        // number of subentries for dir
	int size;                        // size of file
	char *name;                      // it's short name
	int biggest;                     //�±�Ϊbiggest�Ĳ���
} node;

node root;

typedef struct FD_ {
	int offset;
	int flags;
	node *f;
} FD_;

FD_ FD[4096];
int MAXfd = 0;

int ropen(const char *pathname, int flags) {
	//along the path to find
	if (pathname[0] != 47) return -1;
	int lenth = strlen(pathname);
	int FirstLetter = 0;//name������Ϻ��������һ���ַ�
	node *find = &root;
	char name_of_find[32];
	char clear[32];
	for (int a = 0; a < lenth; a++)
	{
		if (find->type == 0) return -1;
		if (pathname[a] != 47)
		{
			if (isalnum(pathname[a]) == 0 && pathname[a] != 46) return -1;
			name_of_find[a - FirstLetter] = pathname[a];
			if (pathname[a + 1] == 47||a == lenth - 1)
			{
				FirstLetter = a + 1;
				for (int b = 0; b < find->size; b++)
				{
					if (find->dirents[b] == NULL) continue;
					if (strcmp(find->dirents[b]->name, name_of_find)==0)//why �Ͽ�
					{
						find = find->dirents[b];//why �Ͽ�
						strcpy(name_of_find,clear);//����ҵ������ļ�����name_of_findΪclear
						break;
					}
					else if (b == find->size -1)//�Ҳ���ƥ���
					{
						if (FirstLetter != lenth) return -1;
						else if (flags - 64 != 1024 && flags - 64 != 512 && flags - 64 > 2) return -1;
					}
				}
			}
		}
		else FirstLetter++;
	}

	int num = 0;//����ֵ
	if (MAXfd <= 4096)
	{
		num = MAXfd; MAXfd++;
	}
	else
	{
		while (FD[num].f != NULL) num++;
	}

	//if is a file , judge flags, ��ֵ���ṹ���ڶ�Ӧ�����������±꣬MAXfd++; ����Ѿ����ù��ˣ�����node�ҿ��е�
	if (find->type == 0)
	{
		if (name_of_find[0] != 0)//need to creat
		{
			int last = find->biggest;
			find->biggest++;
			find->size++;
			find->dirents[last]->name = name_of_find;
			find->dirents[last]->type = 0;
			find = find->dirents[last];
			flags = flags - 64;
		}
		
		else if (flags == 513 || flags == 514)
		{
			int len = strlen(find->content);
			memset(find->content, 0, len);
			flags -= 512;
		}

		FD[num].f = find;
		FD[num].flags = flags;
		if (flags > 1000) FD[num].offset = strlen(find->content);
		else FD[num].offset = 0;
	}

	//if is a dir ,if create, realloc dirents, return
	else FD[num].f = find;
	free(find);
	return num;
}

int rclose(int fd) {
  // ͨ���±��ң����f�Ѿ���NULL��return -1��
	if (FD[fd].f == NULL) return -1;
	else
	{
		FD[fd].f = NULL;
		FD[fd].flags = 0;
		FD[fd].offset = 0;
	}
}

ssize_t rwrite(int fd, const void *buf, size_t count) {
	int offset = FD[fd].offset;
  // �ж�fd[fd]�е�flag����������ֱ�ӷ���-1
	if (FD[fd].flags == 1024 || FD[fd].flags == 64 || FD[fd].flags == 512) return -1;
  //�ҵ�fd�ļ���ƫ������ѭ����buf��count���ַ���д��������������realloc file���ݣ�ƫ�������ƣ��Ƿ�Ҫ���ú�������
	int b = strlen(FD[fd].f->content);
	if (offset + count > b)
	{
		char *oldly = FD[fd].f->content;
		char *newly = realloc(oldly, FD[fd].offset + count);
		FD[fd].f->content = newly;
	}
	char *buf1;
	buf1 = realloc(buf, count);
	for (int a = 1; a <= count; a++)
	{
		FD[fd].f->content[offset] = buf1[a - 1];//////
		FD[fd].offset++;
		offset++:
	}
	free(buf1);
	return count;
}

ssize_t rread(int fd, void *buf, size_t count) {
  //  �ж�fd[fd]�е�flag����������ֱ�ӷ���-1
	char *buf1;
	int offset = FD[fd].offset;
	if (FD[fd].f->type == 1) return -1;
	if (FD[fd].flags - 1== 1024 || FD[fd].flags - 1== 64 || FD[fd].flags - 1== 512) return -1;
  //  �ҵ�fdƫ�������жϵ���ĩ�Ƿ�count���ֽڣ�ȷ������ֵ��ѭ����ֵ��buf(or memcpy)��ƫ�������ƣ�return
	int num;
	int b = strlen(FD[fd].f->content);
	if (offset + count > b) num = b - offset;
	else num = count;
	for (int a = 1; a <= num; a++)
	{
	   buf1 [a - 1] = FD[fd].f->content[offset];
		FD[fd].offset++;
		offset++;
	}
	buf = realloc(buf1, num);
	return num;
}

off_t rseek(int fd, off_t offset, int whence) {//off_t = long����ʾ��Ҫ�Ƶ�offsetλ��
  // �޸�ƫ����
	int last;
	if (whence == 0) last = offset;
	else if (whence == 1) last = FD[fd].offset + offset;
	else if (whence == 2) last = strlen(FD[fd].f->content) + offset;
	if (last >= 0) { FD[fd].offset = last; return last; }
	else return -1;
}

int rmkdir(const char *pathname) {
  // find,����Ŀ¼
	int FirstLetter = 0;//name������Ϻ��������һ���ַ�
	node *find = &root;
	char name_of_find[32];
	char clear[32];
	int last = strlen(pathname) - 1;
	while (pathname[last] == 47) last--;
	for (int a = 0; a <= last; a++)
	{
		if (find->type == 0) return -1;
		if (pathname[a] != 47)
		{
			name_of_find[a - FirstLetter] = pathname[a];
			if (pathname[a + 1] == 47 || a == last)
			{
				FirstLetter = a + 1;
				for (int b = 0; b < find->size; b++)
				{
					if (!strcmp(find->dirents[b]->name, name_of_find))
					{
						if (a == last) return -1;
						find = find->dirents[b];
						strcpy(name_of_find, clear);//����ҵ������ļ�����name_of_findΪclear
						break;
					}
				}
			}
		}
		else FirstLetter++;
	}
	int last = find->biggest;
	find->biggest++;
	find->size++;
	find->dirents[last]->type = 1;
	find->dirents[latest]->name = name_of_find;
	free(find);
	return 0;
}

int rrmdir(const char *pathname) {
  // find��ɾ����Ŀ¼
	int FirstLetter = 0;//name������Ϻ��������һ���ַ�
	node *header;
	node *find = &root;
	int num;
	char name_of_find[32];
	char clear[32];
	int last = strlen(pathname) - 1;
	while (pathname[last] == 47) last--;
	for (int a = 0; a <= last; a++)
	{
		if (find->type == 0) return -1;
		if (pathname[a] != 47)
		{
			name_of_find[a - FirstLetter] = pathname[a];
			if (pathname[a + 1] == 47 || a == last)
			{
				FirstLetter = a + 1;
				for (int b = 0; b < find->size; b++)
				{
					if (!strcmp(find->dirents[b]->name, name_of_find))
					{
						header = find;
						find =header->dirents[b];
						strcpy(name_of_find , clear);//����ҵ������ļ�����name_of_findΪclear
						num = b;
						break;
					}
					else return -1;
				}
			}
		}
		else FirstLetter++;
	}
	if (find->type == 0 || find->size != 0) return -1;
	else
	{
		header->dirents[num] = NULL;
		header->size--;
	}
	free(header);
	free(find);
	return 0;
}

int runlink(const char *pathname) {
  // ɾ���ļ�
	int FirstLetter = 0;//name������Ϻ��������һ���ַ�
	node *header;
	node *find = &root;
	int num;
	char name_of_find[32];
	char clear[32];
	int last = strlen(pathname) - 1;
	for (int a = 0; a <= last; a++)
	{
		if (find->type == 0) return -1;
		if (pathname[a] != 47)
		{
			name_of_find[a - FirstLetter] = pathname[a];
			if (pathname[a + 1] == 47 || a == last)
			{
				FirstLetter = a + 1;
				for (int b = 0; b < find->size; b++)
				{
					if (!strcmp(find->dirents[b]->name, name_of_find))
					{
						header = find;
						find = find->dirents[b];
						strcpy(name_of_find, clear);//����ҵ������ļ�����name_of_findΪclear
						num = b;
						break;
					}
					else return -1;
				}
			}
		}
		else FirstLetter++;
	}
	if (find->type == 1) return -1;
	else 
	{
		header->dirents[num] = NULL;
		header->size--;
	}
	free(header);
	free(find);
}

void init_ramfs() {
	root.type = 1;
	root.name = "/";
}
