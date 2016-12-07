#define LOG_TAG "Commands"

#include <iostream>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <utils/Log.h>
#include <string.h>
#include <errno.h>

#include "Commands.h"

#define GO_MINSPEED_LOAD "minspeedload"
#define GOVERNOR "governor"

#define LIMITMEMORY "limit"
#define FORCERECLAIM "reclaim"
using namespace std;

//CPufreqCmd used for tune cpufreq
CpufreqCmd::CpufreqCmd(const char* cmd) 
   :ICommand(cmd) {
    
}

CpufreqCmd::~CpufreqCmd() {
    
}

void CpufreqCmd::onCommand(int argc, char *args[]) {
    int ret = 0;
    if (!strcmp(args[1], GOVERNOR)) {
        changeGovernor(argc, args);
		return;
    } else if (!strcmp(args[1], GO_MINSPEED_LOAD)) {
        tuneMinspeedLoad(argc, args);
        return;
    }
}

int CpufreqCmd::changeGovernor(int argc, char **args) {
    int ret = 0;
    char buf[12] = {0};
    const char *scaling_governor = "/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor";
    FILE* fp = NULL;
    char *governor;
    if (argc != 3) {
         cout << "change governor need only 3 arguments" << endl;
         return -1;
    }

	governor = args[2];
    fp = fopen(scaling_governor, "r+");
    if (fp == NULL) {
       perror("fopen");
       return -1;
    }
    
    ret = fread(buf, 1,  sizeof(buf), fp);
    if (ret < 0) {
        ALOGE("%s\n", strerror(errno));
        fclose(fp);
        return -1;
    }

    if (!strcmp(governor, buf)) {
         cout << "same governor, no need to change" << endl;
         fclose(fp);
         return 0; 
    }

    ret = fwrite(governor, 1, strlen(governor), fp);
    if (ret < 0) {
        ALOGE("%s\n", strerror(errno));
		fclose(fp);
        return -1;
    }

	fclose(fp);
    return 0;
}

int CpufreqCmd::tuneMinspeedLoad(int argc, char **args) {
    int ret = 0;
    if (argc != 3) {
         cout << "tune minspeed load need only 4 arguments" << endl;
         return -1;
    }

    const char *minload = "/sys/devices/system/cpu/cpufreq/interactive/go_minspeed_load";
    char *load = args[2];

    int fd = open(minload, O_WRONLY);
    if (fd < 0) {
        ALOGE("%s\n", strerror(errno));
         return -1;
    }

    ret = write(fd, load, strlen(load));    
    if (ret < 0) {
        ALOGE("%s\n", strerror(errno));
    }

    close(fd);

    return ret;
}

CgroupCmd::CgroupCmd(const char* cmd)
	:ICommand(cmd) {

}

CgroupCmd::~CgroupCmd() {

}

void CgroupCmd::onCommand(int argc, char *args[]) {
    if (!strcmp(args[1], LIMITMEMORY)) {
        tuneLimitMemory(argc, args);
	}
}

int CgroupCmd::tuneLimitMemory(int argc, char **args) {
    int ret = 0;
    if (argc != 3) {
         cout << "tune minspeed load need only 4 arguments" << endl;
         return -1;
    }

    const char* limit = "/sys/fs/cgroup/memory/limit/memory.limit_in_bytes";
	char* memory = args[2];

	FILE* fp = fopen(limit, "r+");

	ret = fwrite(memory, 1, strlen(memory), fp);
	
    if (ret < 0) {
        ALOGE("%s\n", strerror(errno));

		fclose(fp);
        return -1;
    }

	fclose(fp);
	return 0;
}

ReclaimCmd::ReclaimCmd(const char* cmd) 
	:ICommand(cmd) {

}

ReclaimCmd::~ReclaimCmd() {
}

void ReclaimCmd::onCommand(int argc, char *args[]) {
    if (!strcmp(args[1], FORCERECLAIM))
	    forceReclaim(argc, args);
}

void ReclaimCmd::forceReclaim(int argc, char **args) {
	char reclaim[] = {"1"};
    char filename[32];

	if (argc != 2)
		return;

    snprintf(filename, sizeof(filename), "/proc/%s/reclaim", args[1]);

    FILE * file = fopen(filename, "w+");
    if (!file) {
		ALOGD("open %s failed\n", filename);
        return;
    }

    int ret = fwrite(reclaim, sizeof(char), strlen(reclaim), file);
    ALOGD("force reclaim return:%d\n", ret);
	fclose(file);
}