//////////////////////////////////////////////////////////////////////
//                             North Carolina State University
//
//
//
//                             Copyright 2016
//
////////////////////////////////////////////////////////////////////////
//
// This program is free software; you can redistribute it and/or modify it
// under the terms and conditions of the GNU General Public License,
// version 2, as published by the Free Software Foundation.
//
// This program is distributed in the hope it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
// more details.
//
// You should have received a copy of the GNU General Public License along with
// this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
//
////////////////////////////////////////////////////////////////////////
//
//   Author:  Hung-Wei Tseng
//
//   Description:
//     Skeleton of KeyValue Pseudo Device
//
////////////////////////////////////////////////////////////////////////

#include "keyvalue.h"
#include <asm/uaccess.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/poll.h>

/*keyvalue linked list struct node declaration */
typedef struct node {
  __u64 key;
  __u64 *size;
  void *data;
  struct node *next;
}keyval_node;

unsigned transaction_id;
static void free_callback(void *data)
{
}


/*handlers required for handling get, set and del
 *return types void for now.
 *TODO: Can be converted to thread functions. 
 *TODO: Kernel level threading?
*/
keyval_node *head = NULL;
uint8_t handleSet(struct keyvalue_set kv);
uint8_t handleGet(struct keyvalue_get kv);
uint8_t handleDel(struct keyvalue_delete kv);
keyval_node *searchList(__u64 key);

static long keyvalue_get(struct keyvalue_get __user *ukv)
{
    uint8_t result;
    struct keyvalue_get kv;
    kv.key = ukv->key;
    result = handleGet(kv);
    return transaction_id++;
}

static long keyvalue_set(struct keyvalue_set __user *ukv)
{
  uint8_t result;
  struct keyvalue_set kv;
  kv.key = ukv->key;
  kv.size = ukv->size;
  kv.data = ukv->data;
  result = handleSet(kv);
  return transaction_id++;
}

static long keyvalue_delete(struct keyvalue_delete __user *ukv)
{
  uint8_t result;
  struct keyvalue_delete kv;
  kv.key = ukv->key;
  result = handleDel(kv);
  return transaction_id++;
}

//Added by Hung-Wei
unsigned int keyvalue_poll(struct file *filp, struct poll_table_struct *wait)
{
    unsigned int mask = 0;
    printk("keyvalue_poll called. Process queued\n");
    return mask;
}

static long keyvalue_ioctl(struct file *filp, unsigned int cmd,
                                unsigned long arg)
{
    switch (cmd) {
    case KEYVALUE_IOCTL_GET:
        return keyvalue_get((void __user *) arg);
    case KEYVALUE_IOCTL_SET:
        return keyvalue_set((void __user *) arg);
    case KEYVALUE_IOCTL_DELETE:
        return keyvalue_delete((void __user *) arg);
    default:
        return -ENOTTY;
    }
}

static int keyvalue_mmap(struct file *filp, struct vm_area_struct *vma)
{
    return 0;
}

static const struct file_operations keyvalue_fops = {
    .owner                = THIS_MODULE,
    .unlocked_ioctl       = keyvalue_ioctl,
    .mmap                 = keyvalue_mmap,
//    .poll		  = keyvalue_poll,
};

static struct miscdevice keyvalue_dev = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "keyvalue",
    .fops = &keyvalue_fops,
};

static int __init keyvalue_init(void)
{
    int ret;

    if ((ret = misc_register(&keyvalue_dev)))
        printk(KERN_ERR "Unable to register \"keyvalue\" misc device\n");
    return ret;
}

static void __exit keyvalue_exit(void)
{
    misc_deregister(&keyvalue_dev);
}

//Added by Alok
/*handleSet: 
 *- Checks if key already exists in some node. 
 *- Overwrites existing key with new size and address.
 *- If key doesn't exist, adds a new node at the end.
 *- Returns 1 on success, 0 on failure
 */
uint8_t handleSet(struct keyvalue_set kv)
{
  keyval_node *keyloc = searchList(kv.key);
  return 0;
}

/*handleGet: 
 *- Searches list starting from head. 
 *- Returns 1 if found, 0 if not found.
 *- If found, assigns values to passed struct.
 */
uint8_t handleGet(struct keyvalue_get kv)
{
  keyval_node *keyloc = searchList(kv.key);
  return 0;
}

/*handleDel: 
 *- Checks if key exists. 
 *- Free the node, adjust list 
 *- Returns 1 on success, 0 on failure
 */
uint8_t handleDel(struct keyvalue_delete kv)
{
  keyval_node *keyloc = searchList(kv.key);
  return 0;
}

/*checkList: 
 *- Checks if key exists.  
 */
keyval_node *searchList(__u64 key)
{
  uint8_t found = 0;
  keyval_node *tmp;
  tmp = head;
  while(tmp != NULL)
    {
      if(tmp->key == key)
	{
	  //if key is found, break #TODO: return pointer
	  found = 1;
	  return tmp;
	  break;
	}
      tmp = tmp->next;
    }
  //if key not found in the list, return NULL
  return NULL;
}

MODULE_AUTHOR("Hung-Wei Tseng <htseng3@ncsu.edu>");
MODULE_LICENSE("GPL");
MODULE_VERSION("0.1");
module_init(keyvalue_init);
module_exit(keyvalue_exit);
