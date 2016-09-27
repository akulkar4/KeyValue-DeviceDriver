#ifdef __cplusplus
extern "C" {
#endif
#include <linux/types.h>
int kv_get(int devfd, __u64 key, __u64 *size, void *data);
int kv_set(int devfd, __u64 key, __u64 size, void *data);
int kv_delete(int devfd, __u64 key);
#ifdef __cplusplus
}
#endif