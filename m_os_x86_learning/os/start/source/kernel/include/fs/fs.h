#ifndef FS_H
#define FS_H

struct stat;

void fs_init(void); // 文件系统初始化

int sys_open(const char* name, int flags, ...);
int sys_read(int file, char* ptr, int len);
int sys_write(int file, char* ptr, int len);
int sys_lseek(int file, int ptr, int dir);
int sys_close(int file);
int sys_isatty(int file);
int sys_fstat(int file, struct stat* st);

int sys_dup(int file);


#endif
