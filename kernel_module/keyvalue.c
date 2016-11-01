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
#include <linux/mutex.h>

#define DEBUG 0
#define H_BITS 10
#define H_SIZE 1024

/*keyvalue linked list struct node declaration */
typedef struct node 
{
  __u64 key;
  __u64 size;
  void *data;
  struct node *next;
}keyval_node;

unsigned transaction_id;
static void free_callback(void *data)
{
}

/*
 * Mutex init
 * hashfunction declaration
 * hashtable init
 */
DEFINE_MUTEX(devlock);
uint16_t gethashkey(uint64_t key);
keyval_node *hashtable[H_SIZE];

/*  keyvalue_get: 
 * Searches list starting from head. 
 * Returns 1 if found, 0 if not found.
 * If found, assigns values to passed struct.
 */
static long keyvalue_get(struct keyvalue_get __user *ukv)
{
  uint32_t result;
  unsigned tid;
  struct keyvalue_get getStruct;
  uint16_t key;
  keyval_node *tmp;
  mutex_lock(&devlock);
  copy_from_user(&getStruct, ukv, sizeof(getStruct));
  key = gethashkey(getStruct.key);
  tmp = hashtable[key];
  
  while (tmp != NULL)
    {  
      if(tmp->key == getStruct.key)
	{
	  result = copy_to_user(getStruct.size, &(tmp->size), sizeof(tmp->size));
	  if(result != 0)
	    {
	      mutex_unlock(&devlock);
	      return -1;
	    }
	  result = copy_to_user(getStruct.data, tmp->data, tmp->size);
	  if(result != 0)
	    {
	      mutex_unlock(&devlock);
	      return -1;
	    }
          #if DEBUG
	  printk(KERN_DEBUG "HIT! Size: %d, Result is :%d\n", sizeof(tmp->size),result);
          #endif
	  tid = transaction_id++;
	  mutex_unlock(&devlock);
	  return tid;
	}
      else
	tmp = tmp->next;
    }
  mutex_unlock(&devlock);
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
  uint32_t result;
  uint16_t key;
  unsigned tid;
  struct keyvalue_set setStruct;
  keyval_node *tmp;
  keyval_node *prev;

  mutex_lock(&devlock);
  result = copy_from_user(&setStruct, ukv, sizeof(setStruct));
  if(result != 0)
    {
      mutex_unlock(&devlock);
      return -1;
    }
  key = gethashkey(setStruct.key);
  tmp = hashtable[key];
  prev = NULL;
  
  //first element in the key-list?
  if(tmp == NULL)
    {
      tmp = (keyval_node *)kmalloc(sizeof(keyval_node), GFP_KERNEL);
      if(tmp == NULL)
	{
	  mutex_unlock(&devlock);
	  return -1;
	}
      hashtable[key] = tmp;
      tmp->key = setStruct.key;
      tmp->size = setStruct.size;
      tmp->next = NULL;
      tmp->data = kmalloc(setStruct.size, GFP_KERNEL);
      if(tmp->data == NULL)
	{
	  mutex_unlock(&devlock);
	  return -1;
	}
      result = copy_from_user(tmp->data, setStruct.data, setStruct.size);
      if(result != 0)
	{
	  mutex_unlock(&devlock);
	  return -1;
	}
      else
	{
          #if DEBUG
	  printk(KERN_DEBUG "Added to head. Hash key:%d, Key:%lu, Size:%d, String: %s\n", key, tmp->key, tmp->size, tmp->data);
          #endif
	  tid = transaction_id++;
	  mutex_unlock(&devlock);	
	  return tid;
	}
    }
  
  while(tmp  != NULL)
    {
      if(tmp->key == setStruct.key)

	{
	  kfree(tmp->data);
	  tmp->data = kmalloc(setStruct.size, GFP_KERNEL);
	  if(tmp->data == NULL)
	    {
	      mutex_unlock(&devlock);
	      return -1;
	    }
	  tmp->size = setStruct.size;
	  result = copy_from_user(tmp->data, setStruct.data, setStruct.size);
	  if(result != 0)
	    {
	      mutex_unlock(&devlock);
	      return -1;
	    }
	  else
	    {
              #if DEBUG
	      printk(KERN_DEBUG "Matched a key, re-wrote. Result is :%d, String: %s\n", result, tmp->data);
              #endif
	      tid = transaction_id++;
	      mutex_unlock(&devlock);	
	      return tid;
	    }
	}
      else
	{
	  prev = tmp;
	  tmp = tmp->next;
	}
    }
  
  //adding to the tail in case of hash collision
  if(tmp == NULL)
    {
	  keyval_node *newnode = (keyval_node *)kmalloc(sizeof(keyval_node), GFP_KERNEL);
	  if(newnode == NULL)
	    {
	      mutex_unlock(&devlock);
	      return -1;
	    }
	  newnode->key = setStruct.key;
	  newnode->size = setStruct.size;
	  newnode->data = kmalloc(setStruct.size, GFP_KERNEL);
	  newnode->next = NULL;
	  prev->next = newnode;
	  result = copy_from_user(newnode->data, setStruct.data, setStruct.size);
	  if(result != 0)
	    {
	      mutex_unlock(&devlock);
	      return -1;
	    }
	  else
	    {
              #if DEBUG
	      printk(KERN_DEBUG "Added to tail. Result is :%d, Hash Key:%d, Key:%lu, Size:%d, String: %s\n", result, 
		 key, newnode->key, newnode->size, newnode->data);
              #endif
	      tid = transaction_id++;
	      mutex_unlock(&devlock);
	      return tid;
	    }
    }
}

/* keyvalue_delete: 
 * Checks if key exists. 
 * Free the node, adjust list 
 * Returns 1 on success, -1 on failure
 */
static long keyvalue_delete(struct keyvalue_delete __user *ukv)
{
  uint32_t result;
  uint16_t key;
  unsigned tid;
  struct keyvalue_delete delStruct;
  keyval_node *tmp;
  keyval_node *nxtPtr;
  keyval_node *prev;

  mutex_lock(&devlock);
  result = copy_from_user(&delStruct, ukv, sizeof(delStruct));
  if(result != 0)
    {
      mutex_unlock(&devlock);
      return -1;
    }
  key = gethashkey(delStruct.key);
  tmp = hashtable[key];
  nxtPtr = NULL;
  prev = NULL;

  //empty hash entry
  if(tmp == NULL)
    {
      mutex_unlock(&devlock);
      return -1;
    } 

  //search for key
  while(tmp->next != NULL)
    {
      if(tmp->key == delStruct.key)
	{
	      kfree(tmp->data);
	      tmp->key = tmp->next->key;
	      tmp->size = tmp->next->size;
	      tmp->data = tmp->next->data;
	      nxtPtr = tmp->next->next;
	      kfree(tmp->next);
	      tmp->next = nxtPtr;
              #if DEBUG     
	      printk(KERN_DEBUG "Deleted node in the middle!\n");
              #endif
	      tid = transaction_id++;
	      mutex_unlock(&devlock);
	      return tid;
	}
      else
	{
	  prev = tmp;
	  tmp = tmp->next;
	}
    }

  if(tmp->next == NULL && tmp->key == delStruct.key)
    {
      if(tmp != hashtable[key])
	{
	  kfree(prev->next->data);
	  kfree(prev->next);
	  prev->next = NULL;
          #if DEBUG
	  printk(KERN_DEBUG "Deleted node in the tail!\n");
          #endif
	  tid = transaction_id++;
	  mutex_unlock(&devlock);
	  return tid;
	}
      
      else if(tmp == hashtable[key])
	{
	  kfree(tmp->data);
	  kfree(tmp);
	  hashtable[key] = NULL;
          #if DEBUG
	  printk(KERN_DEBUG "Deleted lone head!\n");
          #endif
	  tid = transaction_id++;
	  mutex_unlock(&devlock);
	  return tid;
	}
    }
  mutex_unlock(&devlock);
  return -1;
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
  //return ((key * 11400714819323198549ul) >> (64 - H_BITS));
  return((key%H_SIZE));
}

MODULE_AUTHOR("Hung-Wei Tseng <htseng3@ncsu.edu>");
MODULE_LICENSE("GPL");
MODULE_VERSION("0.1");
module_init(keyvalue_init);
module_exit(keyvalue_exit);
