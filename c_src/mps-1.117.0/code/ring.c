/* ring.c: RING IMPLEMENTATION
 *
 * $Id: //info.ravenbrook.com/project/mps/version/1.117/code/ring.c#1 $
 * Copyright (c) 2001-2014 Ravenbrook Limited.  See end of file for license.
 *
 * .intro: This is a portable implementation of Rings.
 *
 * .purpose: Rings are used to manage potentially unbounded collections
 * of things.
 *
 * .sources: <design/ring/>
 */

#include "ring.h"
#include "check.h"
#include "misc.h"

SRCID(ring, "$Id: //info.ravenbrook.com/project/mps/version/1.117/code/ring.c#1 $");


/* RingCheck, RingCheckSingle -- check the validity of a ring node
 *
 * RingCheck performs a consistency check on the ring node
 * (<design/ring#check>). RingCheckSingle performs the same check, but
 * also checks that the ring node is a singleton
 * (<design/ring/#check.single>).
 */

Bool RingCheck(Ring ring)
{
  CHECKL(ring != NULL);
  CHECKL(ring->next != NULL);
  CHECKL(ring->next->prev == ring);
  CHECKL(ring->prev != NULL);
  CHECKL(ring->prev->next == ring);
  UNUSED(ring); /* <code/mpm.c#check.unused> */
  return TRUE;
}

Bool RingCheckSingle(Ring ring)
{
  CHECKL(RingCheck(ring));
  CHECKL(ring->next == ring);
  CHECKL(ring->prev == ring);
  UNUSED(ring); /* <code/mpm.c#check.unused> */
  return TRUE;
}


/* RingIsSingle -- return true if ring is a singleton
 *
 * See <design/ring/#is.single>
 */

Bool RingIsSingle(Ring ring)
{
  AVERT(Ring, ring);
  return (ring->next == ring);
}


/* RingLength -- return the number of nodes in the ring, not including
 * this node
 *
 * See <design/ring/#length>
 */

Count RingLength(Ring ring)
{
  Count length = 0;
  Ring node, next;
  AVERT(Ring, ring);
  RING_FOR(node, ring, next)
    ++ length;
  return length;
}


/* RingInit -- initialize a ring node
 */

void (RingInit)(Ring ring)
{
  RingInit(ring);                       /* <code/mpm.h#ring.init> */
}


/* RingFinish -- finish a ring node
 */

void (RingFinish)(Ring ring)
{
  RingFinish(ring);                     /* <code/mpm.h#ring.finish> */
}


/* RingAppend -- add a ring node to the end of a ring
 */

void (RingAppend)(Ring ring, Ring new)
{
  RingAppend(ring, new);                /* <code/mpm.h#ring.append> */
}


/* RingInsert -- add a ring node to the start of a ring
 */

void (RingInsert)(Ring ring, Ring new)
{
  RingInsert(ring, new);                /* <code/mpm.h#ring.insert> */
}


/* RingRemove -- remove a node from a ring
 */

void (RingRemove)(Ring old)
{
  RingRemove(old);                      /* <code/mpm.h#ring.remove> */
}


/* RingNext -- get the next element of a ring
 */

Ring (RingNext)(Ring ring)
{
  return RingNext(ring);                /* <code/mpm.h#ring.next> */
}

/* RingPrev -- get the previous element of a ring
 */

Ring (RingPrev)(Ring ring)
{
  return RingPrev(ring);                /* <code/mpm.h#ring.prev> */
}


/* RING_ELT -- get the ring element structure
 *
 * RING_ELT has no function (as it does not have function-like
 * behaviour), and is defined in <code/mpm.h#ring.elt>.
 */


/* RING_FOR -- ring iterator construct
 *
 * RING_FOR has no function (as it does not have function-like
 * behaviour), and is defined in <code/mpm.h#ring.for>.
 */


/* C. COPYRIGHT AND LICENSE
 *
 * Copyright (C) 2001-2014 Ravenbrook Limited <http://www.ravenbrook.com/>.
 * All rights reserved.  This is an open source license.  Contact
 * Ravenbrook for commercial licensing options.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * 
 * 3. Redistributions in any form must be accompanied by information on how
 * to obtain complete source code for this software and any accompanying
 * software that uses this software.  The source code must either be
 * included in the distribution or be available for no more than the cost
 * of distribution plus a nominal fee, and must be freely redistributable
 * under reasonable conditions.  For an executable file, complete source
 * code means the source code for all modules it contains. It does not
 * include source code for modules or files that typically accompany the
 * major components of the operating system on which the executable file
 * runs.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE, OR NON-INFRINGEMENT, ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDERS AND CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
