/*
 * inject.c
 *
 *  Created on: Jun 4, 2011
 *      Author: d
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
#include "utils.h"
#include <signal.h>
#include <sys/types.h>
#ifdef ANDROID
//#include <linker.h>
#endif
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <jni.h>
//#include <utils/Log.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>

#define BUFFER_SIZE 3

int find_pid_of(const char *process_name) {
	int id;
	pid_t pid = -1;
	DIR* dir;
	FILE *fp;
	char filename[32];
	char cmdline[256];

	struct dirent * entry;

	if (process_name == NULL)
		return -1;

	dir = opendir("/proc");
	if (dir == NULL)
		return -1;

	while ((entry = readdir(dir)) != NULL) {
		id = atoi(entry->d_name);
		if (id != 0) {
			sprintf(filename, "/proc/%d/cmdline", id);
			fp = fopen(filename, "r");
			if (fp) {
				fgets(cmdline, sizeof(cmdline), fp);
				fclose(fp);

				if (strcmp(process_name, cmdline) == 0) {
					/* process found */
					pid = id;
					break;
				}
			}
		}
	}
	closedir(dir);
	return pid;
}

char * str_contact(const char *str1, const char *str2) {
	char * result;
	result = (char*) malloc(strlen(str1) + strlen(str2) + 1); //str1的长度 + str2的长度 + \0;
	if (!result) { //如果内存动态分配失败
		LOGD("Error: malloc failed in concat! \n");
		exit(EXIT_FAILURE);
	}
	strcpy(result, str1);
	strcat(result, str2); //字符串拼接
	return result;
}

int main(int argc, char* argv[]) {
	int pid;
	struct link_map *map;
	struct elf_info einfo;

	extern dl_fl_t ldl;

	void *handle = NULL;
	long proc = 0;
	long hooker_fopen = 0;
	char pathfile[100];

	if (argc != 4) {
		LOGE("illegal arguments, injection reject");
		return -1;
	}
	LOGD("inject begin");

	pid = find_pid_of(argv[1]);
	ptrace_attach(pid);

	ptrace_find_dlinfo(pid);

	handle = ptrace_dlopen(pid, str_contact(argv[2], HOOK_LIB), 1);
	printf("ptrace_dlopen handle %p\n", handle);
	proc = (long) ptrace_dlsym(pid, handle, "hook");
	printf("main = %lx\n", proc);
	ptrace_arg arg;
	arg.s = argv[3];
	arg.type = PAT_STR;
	ptrace_call(pid, proc, 1, &arg);
	ptrace_detach(pid);
	LOGD("inject end");
	exit(0);
	return 0;
}
