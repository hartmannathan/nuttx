/****************************************************************************
 * include/syscall.h
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 ****************************************************************************/

#ifndef __INCLUDE_SYSCALL_H
#define __INCLUDE_SYSCALL_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

/* This is just a wrapper around sys/syscall.h and arch/syscall.h */

#include <sys/syscall.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#ifdef CONFIG_LIB_SYSCALL
#  ifndef UP_WRAPSYM
#    define UP_WRAPSYM(s) __wrap_##s
#  endif
#  ifndef UP_REALSYM
#    define UP_REALSYM(s) __real_##s
#  endif
#else
#  define UP_WRAPSYM(s) s
#  define UP_REALSYM(s) s
#endif

/****************************************************************************
 * Public Type Definitions
 ****************************************************************************/

/****************************************************************************
 * Public Functions Definitions
 ****************************************************************************/

#ifdef __cplusplus
#define EXTERN extern "C"
extern "C"
{
#else
#define EXTERN extern
#endif

#undef EXTERN
#ifdef __cplusplus
}
#endif

#endif /* __INCLUDE_SYSCALL_H */
