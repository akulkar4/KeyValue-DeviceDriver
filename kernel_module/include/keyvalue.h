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


#ifndef _UAPI_KEYVALUE_H
#define _UAPI_KEYVALUE_H

#include <linux/types.h>


struct keyvalue {
    __u64 key;
    __u64 size;
    void *data;
};

#define KEYVALUE_IOCTL_GET     _IOWR('N', 0x43, struct keyvalue)
#define KEYVALUE_IOCTL_PUT  _IOWR('N', 0x44, struct keyvalue)
#define KEYVALUE_IOCTL_DELETE  _IOWR('N', 0x45, struct keyvalue)

#endif
