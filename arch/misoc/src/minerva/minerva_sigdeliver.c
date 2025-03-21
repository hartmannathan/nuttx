/****************************************************************************
 * arch/misoc/src/minerva/minerva_sigdeliver.c
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

#include <stdint.h>
#include <sched.h>
#include <syscall.h>
#include <assert.h>
#include <debug.h>

#include <nuttx/irq.h>
#include <nuttx/arch.h>
#include <nuttx/board.h>
#include <arch/board/board.h>
#include <arch/minerva/csrdefs.h>

#include "sched/sched.h"
#include "minerva.h"

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: minerva_sigdeliver
 *
 * Description:
 *   This is the a signal handling trampoline.  When a signal action was
 *   posted.  The task context was mucked with and forced to branch to this
 *   location with interrupts disabled.
 *
 ****************************************************************************/

void minerva_sigdeliver(void)
{
  struct tcb_s *rtcb = this_task();
  uint32_t regs[XCPTCONTEXT_REGS];

  board_autoled_on(LED_SIGNAL);

  sinfo("rtcb=%p sigpendactionq.head=%p\n",
        rtcb, rtcb->sigpendactionq.head);
  DEBUGASSERT((rtcb->flags & TCB_FLAG_SIGDELIVER) != 0);

  /* Save the real return state on the stack. */

  up_copystate(regs, rtcb->xcp.regs);
  regs[REG_CSR_MEPC] = rtcb->xcp.saved_epc;
  regs[REG_CSR_MSTATUS] = rtcb->xcp.saved_int_ctx;

  /* We do this so that we can nullify the TCB_FLAG_SIGDELIVER in the TCB
   * and accept more signal deliveries while processing the current pending
   * signals.
   */

  rtcb->flags &= ~TCB_FLAG_SIGDELIVER;

#  ifndef CONFIG_SUPPRESS_INTERRUPTS
  /* Then make sure that interrupts are enabled.  Signal handlers must always
   * run with interrupts enabled.
   */

  up_irq_enable();
#  endif

  /* Deliver the signals */

  nxsig_deliver(rtcb);

  /* Output any debug messages BEFORE restoring errno (because they may
   * alter errno), then disable interrupts again and restore the original
   * errno that is needed by the user logic (it is probably EINTR).
   */

  sinfo("Resuming EPC: %08x INT_CTX: %08x\n", regs[REG_CSR_MEPC],
        regs[REG_CSR_MSTATUS]);

  up_irq_save();

  /* Then restore the correct state for this thread of execution. */

  board_autoled_off(LED_SIGNAL);
  up_fullcontextrestore(regs);

  /* up_fullcontextrestore() should not return but could if the software
   * interrupts are disabled.
   */

  DEBUGPANIC();
}
