/****************************************************************************
 * libs/libc/stdio/lib_fseeko.c
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

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "libc.h"

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: fseeko
 *
 * Description:
 *   The fseeko() function sets the file position indicator for the stream
 *   pointed to by stream. The new position, measured in bytes, is obtained
 *   by adding offset bytes to the position specified by whence. If whence is
 *   set to SEEK_SET, SEEK_CUR, or SEEK_END, the offset is relative to the
 *   start of the file, the current position indicator, or end-of-file,
 *   respectively. A successful call to the fseeko() function clears the
 *   end-of-file indicator for the stream and undoes any effects of the
 *   ungetc(3) function on the same stream.
 *
 * Returned Value:
 *   Zero on success; -1 on failure with errno set appropriately.
 *
 ****************************************************************************/

int fseeko(FAR FILE *stream, off_t offset, int whence)
{
#ifdef CONFIG_DEBUG_FEATURES
  /* Verify that we were provided with a stream */

  if (!stream)
    {
      set_errno(EBADF);
      return ERROR;
    }
#endif

#ifndef CONFIG_STDIO_DISABLE_BUFFERING
  /* Flush any valid read/write data in the buffer (also verifies stream) */

  flockfile(stream);
  if (lib_rdflush_unlocked(stream) < 0 || lib_wrflush_unlocked(stream) < 0)
    {
      funlockfile(stream);
      return ERROR;
    }

  funlockfile(stream);
#endif

  /* On success or failure, discard any characters saved by ungetc() */

#if CONFIG_NUNGET_CHARS > 0
  stream->fs_nungotten = 0;
#endif

  /* Perform the fseeko on the underlying file descriptor */

  if (stream->fs_iofunc.seek != NULL)
    {
      return stream->fs_iofunc.seek(stream->fs_cookie, &offset,
                                    whence) == (off_t)-1 ? ERROR : OK;
    }
  else
    {
      return lseek((int)(intptr_t)stream->fs_cookie, offset,
                                    whence) == (off_t)-1 ? ERROR : OK;
    }
}
