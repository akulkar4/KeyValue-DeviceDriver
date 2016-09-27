#include "keyvalue.h"
#include <keyvalue/keyvalue.h>
#include <stdio.h>
#include <sys/ioctl.h>

int kv_get(int devfd, __u64 key, __u64 *size, void *data)
{
     struct keyvalue_get cmd;
     cmd.key = key;     
     cmd.size = size;
     cmd.data = data;
     return ioctl(devfd, KEYVALUE_IOCTL_GET, &cmd);
}

int kv_set(int devfd, __u64 key, __u64 size, void *data)
{
     struct keyvalue_set cmd;
     cmd.key = key;     
     cmd.size = size;
     cmd.data = data;
     return ioctl(devfd, KEYVALUE_IOCTL_SET, &cmd);
}
int kv_delete(int devfd, __u64 key)
{
     struct keyvalue_delete cmd;
     cmd.key = key;     
     return ioctl(devfd, KEYVALUE_IOCTL_DELETE, &cmd);
}
