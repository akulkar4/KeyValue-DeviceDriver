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
//#include <stdlib.h>

#define H_BITS 10
#define H_SIZE 1024

/*keyvalue linked list struct node declaration */
typedef struct node {
  __u64 key;
  __u64 size;
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
uint16_t gethashkey(uint64_t key);
keyval_node *hashtable[H_SIZE];

/*  keyvalue_get: 
 *- Searches list starting from head. 
 *- Returns 1 if found, 0 if not found.
 *- If found, assigns values to passed struct.
 */
static long keyvalue_get(struct keyvalue_get __user *ukv)
{
    uint8_t result;
    struct keyvalue_get getStruct;
    copy_from_user((void *)&getStruct, (void *)ukv, sizeof(keyvalue_get));
    uint16_t key = gethashkey(getStruct.key);
    keyval_node *tmp = hashtable[key];
    
    while (tmp != NULL)
      {
	if(tmp->key == getStruct.key)
	  {
	    copy_to_user((void *)getStruct.data, (void *)tmp->data, tmp->size);
	    copy_to_user((void *)getStruct.size, (void *)&tmp->size, sizeof(tmp->size));
	    return transaction_id++;
	  }
	else
	  tmp = tmp->next;
      }

    return -1;
}

/*  keyvalue_set: 
 *- Checks if key already exists in some node. 
 *- Overwrites existing key with new size and address.
 *- If key doesn't exist, adds a new node at the end.
 *- Returns 1 on success, 0 on failure
 */
static long keyvalue_set(struct keyvalue_set __user *ukv)
{
  uint8_t result;
  struct keyvalue_set setStruct;
  copy_from_user((void *)&setStruct, (void *)ukv, sizeof(keyvalue_set));
  uint16_t key = gethashkey(setStruct.key);
  keyval_node *tmp = hashtable[key];

  while(tmp != NULL)
    {
      if(tmp->key == setStruct.key)
	{
	  kfree(tmp->data);
	  tmp->data = kmalloc(setStruct.size, GFP_KERNEL);
	  result = copy_from_user(&tmp->size, &setStruct.size, sizeof(setStruct.size));
	  result = copy_from_user(tmp->data, setStruct.data, setStruct.size);
	  if(result != 0)
	    return -1;
	  else
	    return transaction_id++;
	}
      else
	tmp = tmp->next;
    }
  
  tmp = (keyval_node *)kmalloc(sizeof(keyval_node), GFP_KERNEL);
  tmp->key = setStruct.key;
  tmp->data = kmalloc(setStruct.size, GFP_KERNEL);
  result = copy_from_user(tmp->data, setStruct.data, setStruct.size);
  if(result != 0)
    return -1;
  else
    return transaction_id++;
}

/*  keyvalue_delete: 
 *- Checks if key exists. 
 *- Free the node, adjust list 
 *- Returns 1 on success, 0 on failure
 */
static long keyvalue_delete(struct keyvalue_delete __user *ukv)
{
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
  int ret, i;
    
    if ((ret = misc_register(&keyvalue_dev)))
        printk(KERN_ERR "Unable to register \"keyvalue\" misc device\n");
    
    //Initializing pointers to NULL
    for(i=0; i<H_SIZE; i++)
      {
	hashtable[i] = NULL;
      }

    return ret;
}

static void __exit keyvalue_exit(void)
{
    misc_deregister(&keyvalue_dev);
}

/*  gethashkey : 
 *  Simple hashing function key exists. 
 */
uint16_t gethashkey(uint64_t key)
{
  return ((key * 11400714819323198549ul) >> (64 - H_BITS));
}

MODULE_AUTHOR("Hung-Wei Tseng <htseng3@ncsu.edu>");
MODULE_LICENSE("GPL");
MODULE_VERSION("0.1");
module_init(keyvalue_init);
module_exit(keyvalue_exit);
