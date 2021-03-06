/* locus.c: LOCUS MANAGER
 *
 * $Id: //info.ravenbrook.com/project/mps/version/1.117/code/locus.c#1 $
 * Copyright (c) 2001-2018 Ravenbrook Limited.  See end of file for license.
 *
 * DESIGN
 *
 * See <design/arenavm/> and <design/locus/> for basic locus stuff.
 * See <design/trace/> for chains. See <design/strategy/> for the
 * collection strategy.
 */

#include "locus.h"
#include "ring.h"
#include "mpm.h"
#include "mpstd.h"
#include <float.h> /* for DBL_MAX */

SRCID(locus, "$Id: //info.ravenbrook.com/project/mps/version/1.117/code/locus.c#1 $");


/* LocusPrefCheck -- check the consistency of a locus preference */

Bool LocusPrefCheck(LocusPref pref)
{
  CHECKS(LocusPref, pref);
  CHECKL(BoolCheck(pref->high));
  /* zones can't be checked because it's arbitrary. */
  /* avoid can't be checked because it's arbitrary. */
  return TRUE;
}


/* LocusPrefDefault -- return a locus preference representing the defaults */

static LocusPrefStruct locusPrefDefault = LocusPrefDEFAULT;

LocusPref LocusPrefDefault(void)
{
  return &locusPrefDefault;
}

/* LocusPrefInit -- initialise a locus preference to the defaults */

void LocusPrefInit(LocusPref pref)
{
  (void)mps_lib_memcpy(pref, &locusPrefDefault, sizeof(LocusPrefStruct));
}


/* LocusPrefExpress -- express a locus preference */

void LocusPrefExpress(LocusPref pref, LocusPrefKind kind, void *p)
{
  AVERT(LocusPref, pref);
  AVER(pref != &locusPrefDefault);

  switch(kind) {
  case LocusPrefHIGH:
    AVER(p == NULL);
    pref->high = TRUE;
    break;

  case LocusPrefLOW:
    AVER(p == NULL);
    pref->high = FALSE;
    break;

  case LocusPrefZONESET:
    AVER(p != NULL);
    pref->zones = *(ZoneSet *)p;
    break;

  default:
    /* Unknown kinds are ignored for binary compatibility. */
    break;
  }
}


/* LocusPrefDescribe -- describe a locus preference */

Res LocusPrefDescribe(LocusPref pref, mps_lib_FILE *stream, Count depth)
{
  Res res;

  if (!TESTT(LocusPref, pref))
    return ResFAIL;
  if (stream == NULL)
    return ResFAIL;

  res = WriteF(stream, depth,
               "LocusPref $P {\n", (WriteFP)pref,
               "  high $S\n", WriteFYesNo(pref->high),
               "  zones $B\n", (WriteFB)pref->zones,
               "  avoid $B\n", (WriteFB)pref->avoid,
               "} LocusPref $P\n", (WriteFP)pref,
               NULL);
  return res;
}


/* GenDescCheck -- check a GenDesc */

ATTRIBUTE_UNUSED
Bool GenDescCheck(GenDesc gen)
{
  CHECKS(GenDesc, gen);
  /* nothing to check for zones */
  CHECKL(gen->capacity > 0);
  CHECKL(gen->mortality >= 0.0);
  CHECKL(gen->mortality <= 1.0);
  CHECKD_NOSIG(Ring, &gen->locusRing);
  CHECKD_NOSIG(Ring, &gen->segRing);
  return TRUE;
}


/* GenParamCheck -- check consistency of generation parameters */

ATTRIBUTE_UNUSED
static Bool GenParamCheck(GenParamStruct *params)
{
  CHECKL(params != NULL);
  CHECKL(params->capacity > 0);
  CHECKL(params->capacity <= SizeMAX / 1024);
  CHECKL(params->mortality >= 0.0);
  CHECKL(params->mortality <= 1.0);
  return TRUE;
}


/* GenDescInit -- initialize a generation in a chain */

static void GenDescInit(GenDesc gen, GenParamStruct *params)
{
  TraceId ti;

  AVER(gen != NULL);
  AVER(GenParamCheck(params));

  gen->zones = ZoneSetEMPTY;
  gen->capacity = params->capacity * 1024;
  gen->mortality = params->mortality;
  RingInit(&gen->locusRing);
  RingInit(&gen->segRing);
  gen->activeTraces = TraceSetEMPTY;
  for (ti = 0; ti < TraceLIMIT; ++ti)
    RingInit(&gen->trace[ti].traceRing);
  gen->sig = GenDescSig;
  AVERT(GenDesc, gen);
}


/* GenDescFinish -- finish a generation in a chain */

static void GenDescFinish(GenDesc gen)
{
  TraceId ti;

  AVERT(GenDesc, gen);

  gen->sig = SigInvalid;
  RingFinish(&gen->locusRing);
  RingFinish(&gen->segRing);
  AVER(gen->activeTraces == TraceSetEMPTY); /* <design/check/#.common> */
  for (ti = 0; ti < TraceLIMIT; ++ti)
    RingFinish(&gen->trace[ti].traceRing);
}


/* GenDescNewSize -- return effective size of generation */

Size GenDescNewSize(GenDesc gen)
{
  Size size = 0;
  Ring node, nextNode;

  AVERT(GenDesc, gen);

  RING_FOR(node, &gen->locusRing, nextNode) {
    PoolGen pgen = RING_ELT(PoolGen, genRing, node);
    AVERT(PoolGen, pgen);
    size += pgen->newSize;
  }
  return size;
}


/* genDescTraceStart -- notify generation of start of a trace */

void GenDescStartTrace(GenDesc gen, Trace trace)
{
  GenTrace genTrace;

  AVERT(GenDesc, gen);
  AVERT(Trace, trace);

  AVER(!TraceSetIsMember(gen->activeTraces, trace));
  gen->activeTraces = TraceSetAdd(gen->activeTraces, trace);
  genTrace = &gen->trace[trace->ti];
  AVER(RingIsSingle(&genTrace->traceRing));
  RingAppend(&trace->genRing, &genTrace->traceRing);
  genTrace->condemned = 0;
  genTrace->forwarded = 0;
  genTrace->preservedInPlace = 0;
}


/* genDescEndTrace -- notify generation of end of a trace */

void GenDescEndTrace(GenDesc gen, Trace trace)
{
  GenTrace genTrace;
  Size survived;

  AVERT(GenDesc, gen);
  AVERT(Trace, trace);

  AVER(TraceSetIsMember(gen->activeTraces, trace));
  gen->activeTraces = TraceSetDel(gen->activeTraces, trace);
  genTrace = &gen->trace[trace->ti];
  RingRemove(&genTrace->traceRing);
  survived = genTrace->forwarded + genTrace->preservedInPlace;
  AVER(survived <= genTrace->condemned);

  if (genTrace->condemned > 0) {
    double mortality = 1.0 - survived / (double)genTrace->condemned;
    double alpha = LocusMortalityALPHA;
    gen->mortality = gen->mortality * (1 - alpha) + mortality * alpha;
    EVENT6(TraceEndGen, trace, gen, genTrace->condemned, genTrace->forwarded,
           genTrace->preservedInPlace, gen->mortality);
  }
}


/* GenDescCondemned -- memory in a generation was condemned for a trace */

void GenDescCondemned(GenDesc gen, Trace trace, Size size)
{
  GenTrace genTrace;

  AVERT(GenDesc, gen);
  AVERT(Trace, trace);

  genTrace = &gen->trace[trace->ti];
  genTrace->condemned += size;
  trace->condemned += size;
}


/* GenDescSurvived -- memory in a generation survived a trace */

void GenDescSurvived(GenDesc gen, Trace trace, Size forwarded,
                     Size preservedInPlace)
{
  GenTrace genTrace;

  AVERT(GenDesc, gen);
  AVERT(Trace, trace);

  genTrace = &gen->trace[trace->ti];
  genTrace->forwarded += forwarded;
  genTrace->preservedInPlace += preservedInPlace;
  trace->forwardedSize += forwarded;
  trace->preservedInPlaceSize += preservedInPlace;
}


/* GenDescTotalSize -- return total size of generation */

Size GenDescTotalSize(GenDesc gen)
{
  Size size = 0;
  Ring node, nextNode;

  AVERT(GenDesc, gen);

  RING_FOR(node, &gen->locusRing, nextNode) {
    PoolGen pgen = RING_ELT(PoolGen, genRing, node);
    AVERT(PoolGen, pgen);
    size += pgen->totalSize;
  }
  return size;
}


/* GenDescDescribe -- describe a generation in a chain */

Res GenDescDescribe(GenDesc gen, mps_lib_FILE *stream, Count depth)
{
  Index i;
  Res res;
  Ring node, nextNode;

  if (!TESTT(GenDesc, gen))
    return ResFAIL;
  if (stream == NULL)
    return ResFAIL;

  res = WriteF(stream, depth,
               "GenDesc $P {\n", (WriteFP)gen,
               "  zones $B\n", (WriteFB)gen->zones,
               "  capacity $U\n", (WriteFW)gen->capacity,
               "  mortality $D\n", (WriteFD)gen->mortality,
               "  activeTraces $B\n", (WriteFB)gen->activeTraces,
               NULL);
  if (res != ResOK)
    return res;

  for (i = 0; i < NELEMS(gen->trace); ++i) {
    GenTrace genTrace = &gen->trace[i];
    res = WriteF(stream, depth + 2,
                 "trace $U {\n", (WriteFW)i,
                 "  condemned $U\n", (WriteFW)genTrace->condemned,
                 "  forwarded $U\n", (WriteFW)genTrace->forwarded,
                 "  preservedInPlace $U\n", (WriteFW)genTrace->preservedInPlace,
                 "}\n", NULL);
    if (res != ResOK)
      return res;
  }

  RING_FOR(node, &gen->locusRing, nextNode) {
    PoolGen pgen = RING_ELT(PoolGen, genRing, node);
    res = PoolGenDescribe(pgen, stream, depth + 2);
    if (res != ResOK)
      return res;
  }

  res = WriteF(stream, depth, "} GenDesc $P\n", (WriteFP)gen, NULL);
  return res;
}


/* ChainInit -- initialize a generation chain */

static void ChainInit(ChainStruct *chain, Arena arena, GenDescStruct *gens,
                      Count genCount)
{
  AVER(chain != NULL);
  AVERT(Arena, arena);
  AVER(gens != NULL);
  AVER(genCount > 0);

  chain->arena = arena;
  RingInit(&chain->chainRing);
  chain->genCount = genCount;
  chain->gens = gens;
  chain->sig = ChainSig;

  AVERT(Chain, chain);

  RingAppend(&arena->chainRing, &chain->chainRing);
}


/* ChainCreate -- create a generation chain */

Res ChainCreate(Chain *chainReturn, Arena arena, size_t genCount,
                GenParamStruct *params)
{
  size_t i;
  Size size;
  Chain chain;
  GenDescStruct *gens;
  Res res;
  void *p;

  AVER(chainReturn != NULL);
  AVERT(Arena, arena);
  AVER(genCount > 0);
  AVER(params != NULL);

  size = sizeof(ChainStruct) + genCount * sizeof(GenDescStruct);
  res = ControlAlloc(&p, arena, size);
  if (res != ResOK)
    return res;
  chain = p;
  gens = PointerAdd(p, sizeof(ChainStruct));

  for (i = 0; i < genCount; ++i)
    GenDescInit(&gens[i], &params[i]);
  ChainInit(chain, arena, gens, genCount);

  *chainReturn = chain;
  return ResOK;
}


/* ChainCheck -- check a chain */

Bool ChainCheck(Chain chain)
{
  size_t i;

  CHECKS(Chain, chain);
  CHECKU(Arena, chain->arena);
  CHECKD_NOSIG(Ring, &chain->chainRing);
  CHECKL(chain->genCount > 0);
  for (i = 0; i < chain->genCount; ++i) {
    CHECKD(GenDesc, &chain->gens[i]);
  }
  return TRUE;
}


/* ChainDestroy -- destroy a chain */

void ChainDestroy(Chain chain)
{
  Arena arena;
  Size size;
  size_t genCount;
  size_t i;

  AVERT(Chain, chain);

  arena = chain->arena;
  genCount = chain->genCount;
  RingRemove(&chain->chainRing);
  chain->sig = SigInvalid;
  for (i = 0; i < genCount; ++i)
    GenDescFinish(&chain->gens[i]);

  RingFinish(&chain->chainRing);

  size = sizeof(ChainStruct) + genCount * sizeof(GenDescStruct);
  ControlFree(arena, chain, size);
}


/* ChainGens -- return the number of generation in chain */

size_t ChainGens(Chain chain)
{
  AVERT(Chain, chain);
  return chain->genCount;
}


/* ChainGen -- return a generation in a chain, or the arena top generation */

GenDesc ChainGen(Chain chain, Index gen)
{
  AVERT(Chain, chain);
  AVER(gen <= chain->genCount);

  if (gen < chain->genCount)
    return &chain->gens[gen];
  else
    return &chain->arena->topGen;
}


/* ChainDeferral -- time until next ephemeral GC for this chain */

double ChainDeferral(Chain chain)
{
  double time = DBL_MAX;
  size_t i;

  AVERT(Chain, chain);

  for (i = 0; i < chain->genCount; ++i) {
    double genTime;
    GenDesc gen = &chain->gens[i];
    if (gen->activeTraces != TraceSetEMPTY)
      return DBL_MAX;
    genTime = (double)gen->capacity - (double)GenDescNewSize(&chain->gens[i]);
    if (genTime < time)
      time = genTime;
  }

  return time;
}


/* ChainDescribe -- describe a chain */

Res ChainDescribe(Chain chain, mps_lib_FILE *stream, Count depth)
{
  Res res;
  size_t i;

  if (!TESTT(Chain, chain))
    return ResFAIL;
  if (stream == NULL)
    return ResFAIL;

  res = WriteF(stream, depth,
               "Chain $P {\n", (WriteFP)chain,
               "  arena $P\n", (WriteFP)chain->arena,
               NULL);
  if (res != ResOK)
    return res;

  for (i = 0; i < chain->genCount; ++i) {
    res = GenDescDescribe(&chain->gens[i], stream, depth + 2);
    if (res != ResOK)
      return res;
  }

  res = WriteF(stream, depth,
               "} Chain $P\n", (WriteFP)chain,
               NULL);
  return res;
}


/* PoolGenInit -- initialize a PoolGen */

Res PoolGenInit(PoolGen pgen, GenDesc gen, Pool pool)
{
  /* Can't check pgen, because it's not been initialized. */
  AVER(pgen != NULL);
  AVERT(GenDesc, gen);
  AVERT(Pool, pool);
  AVER(PoolHasAttr(pool, AttrGC));

  pgen->pool = pool;
  pgen->gen = gen;
  RingInit(&pgen->genRing);
  pgen->segs = 0;
  pgen->totalSize = 0;
  pgen->freeSize = 0;
  pgen->bufferedSize = 0;
  pgen->newSize = 0;
  pgen->oldSize = 0;
  pgen->newDeferredSize = 0;
  pgen->oldDeferredSize = 0;
  pgen->sig = PoolGenSig;
  AVERT(PoolGen, pgen);

  RingAppend(&gen->locusRing, &pgen->genRing);
  return ResOK;
}


/* PoolGenFinish -- finish a PoolGen */

void PoolGenFinish(PoolGen pgen)
{
  AVERT(PoolGen, pgen);
  AVER(pgen->segs == 0);
  AVER(pgen->totalSize == 0);
  AVER(pgen->freeSize == 0);
  AVER(pgen->bufferedSize == 0);
  AVER(pgen->newSize == 0);
  AVER(pgen->newDeferredSize == 0);
  AVER(pgen->oldSize == 0);
  AVER(pgen->oldDeferredSize == 0);

  pgen->sig = SigInvalid;
  RingRemove(&pgen->genRing);
}


/* PoolGenCheck -- check a PoolGen */

Bool PoolGenCheck(PoolGen pgen)
{
  CHECKS(PoolGen, pgen);
  /* nothing to check about serial */
  CHECKU(Pool, pgen->pool);
  CHECKU(GenDesc, pgen->gen);
  CHECKD_NOSIG(Ring, &pgen->genRing);
  CHECKL((pgen->totalSize == 0) == (pgen->segs == 0));
  CHECKL(pgen->totalSize >= pgen->segs * ArenaGrainSize(PoolArena(pgen->pool)));
  CHECKL(pgen->totalSize == pgen->freeSize + pgen->bufferedSize
         + pgen->newSize + pgen->oldSize
         + pgen->newDeferredSize + pgen->oldDeferredSize);
  return TRUE;
}


/* PoolGenAccountForAlloc -- accounting for allocation of a segment */

static void PoolGenAccountForAlloc(PoolGen pgen, Size size)
{
  pgen->totalSize += size;
  ++ pgen->segs;
  pgen->freeSize += size;
}  


/* PoolGenAlloc -- allocate a segment in a pool generation
 *
 * Allocate a segment belong to klass (which must be GCSegClass or a
 * subclass), attach it to the generation, and update the accounting.
 */

Res PoolGenAlloc(Seg *segReturn, PoolGen pgen, SegClass klass, Size size,
                 ArgList args)
{
  LocusPrefStruct pref;
  Res res;
  Seg seg;
  ZoneSet zones, moreZones;
  Arena arena;
  GenDesc gen;

  AVER(segReturn != NULL);
  AVERT(PoolGen, pgen);
  AVERT(SegClass, klass);
  AVER(IsSubclass(klass, GCSeg));
  AVER(size > 0);
  AVERT(ArgList, args);

  arena = PoolArena(pgen->pool);
  gen = pgen->gen;
  zones = gen->zones;

  LocusPrefInit(&pref);
  pref.high = FALSE;
  pref.zones = zones;
  pref.avoid = ZoneSetBlacklist(arena);
  res = SegAlloc(&seg, klass, &pref, size, pgen->pool, args);
  if (res != ResOK)
    return res;

  RingAppend(&gen->segRing, &SegGCSeg(seg)->genRing);

  moreZones = ZoneSetUnion(zones, ZoneSetOfSeg(arena, seg));
  gen->zones = moreZones;
  
  if (!ZoneSetSuper(zones, moreZones)) {
    /* Tracking the whole zoneset for each generation gives more
     * understandable telemetry than just reporting the added
     * zones. */
    EVENT3(ArenaGenZoneAdd, arena, gen, moreZones);
  }

  PoolGenAccountForAlloc(pgen, SegSize(seg));

  *segReturn = seg;
  return ResOK;
}


/* PoolGenAccountForFill -- accounting for allocation within a segment
 *
 * Call this when the pool allocates memory to the client program via
 * BufferFill.
 *
 * See <design/strategy/#accounting.op.fill>
 */

void PoolGenAccountForFill(PoolGen pgen, Size size)
{
  AVERT(PoolGen, pgen);

  AVER(pgen->freeSize >= size);
  pgen->freeSize -= size;
  pgen->bufferedSize += size;
}


/* PoolGenAccountForEmpty -- accounting for emptying a buffer
 *
 * Call this when the client program returns memory to the pool via
 * BufferEmpty. The deferred flag indicates whether the accounting of
 * the used memory (for the purpose of scheduling collections) should
 * be deferred until later.
 *
 * See <design/strategy/#accounting.op.empty>
 */

void PoolGenAccountForEmpty(PoolGen pgen, Size used, Size unused, Bool deferred)
{
  AVERT(PoolGen, pgen);
  AVERT(Bool, deferred);

  AVER(pgen->bufferedSize >= used + unused);
  pgen->bufferedSize -= used + unused;
  if (deferred) {
    pgen->newDeferredSize += used;
  } else {
    pgen->newSize += used;
  }
  pgen->freeSize += unused;
}


/* PoolGenAccountForAge -- accounting for condemning
 *
 * Call this when memory is condemned via PoolWhiten, or when
 * artificially ageing memory in PoolGenFree. The size parameter
 * should be the amount of memory that is being condemned for the
 * first time. The deferred flag is as for PoolGenAccountForEmpty.
 *
 * See <design/strategy/#accounting.op.age>
 */

void PoolGenAccountForAge(PoolGen pgen, Size wasBuffered, Size wasNew,
                          Bool deferred)
{
  AVERT(PoolGen, pgen);
  AVERT(Bool, deferred);

  AVER(pgen->bufferedSize >= wasBuffered);
  pgen->bufferedSize -= wasBuffered;
  if (deferred) {
    AVER(pgen->newDeferredSize >= wasNew);
    pgen->newDeferredSize -= wasNew;
    pgen->oldDeferredSize += wasBuffered + wasNew;
  } else {
    AVER(pgen->newSize >= wasNew);
    pgen->newSize -= wasNew;
    pgen->oldSize += wasBuffered + wasNew;
  }
}


/* PoolGenAccountForReclaim -- accounting for reclaiming
 *
 * Call this when reclaiming memory, passing the amount of memory that
 * was reclaimed. The deferred flag is as for PoolGenAccountForEmpty.
 *
 * See <design/strategy/#accounting.op.reclaim>
 */

void PoolGenAccountForReclaim(PoolGen pgen, Size reclaimed, Bool deferred)
{
  AVERT(PoolGen, pgen);
  AVERT(Bool, deferred);

  if (deferred) {
    AVER(pgen->oldDeferredSize >= reclaimed);
    pgen->oldDeferredSize -= reclaimed;
  } else {
    AVER(pgen->oldSize >= reclaimed);
    pgen->oldSize -= reclaimed;
  }
  pgen->freeSize += reclaimed;
}


/* PoolGenUndefer -- finish deferring accounting
 *
 * Call this when exiting ramp mode, passing the amount of old
 * (condemned at least once) and new (never condemned) memory whose
 * accounting was deferred (for example, during a ramp).
 *
 * See <design/strategy/#accounting.op.undefer>
 */

void PoolGenUndefer(PoolGen pgen, Size oldSize, Size newSize)
{
  AVERT(PoolGen, pgen);
  AVER(pgen->oldDeferredSize >= oldSize);
  pgen->oldDeferredSize -= oldSize;
  pgen->oldSize += oldSize;
  AVER(pgen->newDeferredSize >= newSize);
  pgen->newDeferredSize -= newSize;
  pgen->newSize += newSize;
}


/* PoolGenAccountForSegSplit -- accounting for splitting a segment */

void PoolGenAccountForSegSplit(PoolGen pgen)
{
  AVERT(PoolGen, pgen);
  AVER(pgen->segs >= 1);   /* must be at least one segment to split */
  ++ pgen->segs;
}


/* PoolGenAccountForSegMerge -- accounting for merging a segment */

void PoolGenAccountForSegMerge(PoolGen pgen)
{
  AVERT(PoolGen, pgen);
  AVER(pgen->segs >= 2);  /* must be at least two segments to merge */
  -- pgen->segs;
}


/* PoolGenAccountForFree -- accounting for the freeing of a segment */

static void PoolGenAccountForFree(PoolGen pgen, Size size,
                                  Size oldSize, Size newSize,
                                  Bool deferred)
{
  /* Pretend to age and reclaim the contents of the segment to ensure
   * that the entire segment is accounted as free. */
  PoolGenAccountForAge(pgen, 0, newSize, deferred);
  PoolGenAccountForReclaim(pgen, oldSize + newSize, deferred);

  AVER(pgen->totalSize >= size);
  pgen->totalSize -= size;
  AVER(pgen->segs > 0);
  -- pgen->segs;
  AVER(pgen->freeSize >= size);
  pgen->freeSize -= size;
}


/* PoolGenFree -- free a segment and update accounting
 *
 * Pass the amount of memory in the segment that is accounted as free,
 * old, or new, respectively. The deferred flag is as for
 * PoolGenAccountForEmpty.
 *
 * See <design/strategy/#accounting.op.free>
 */

void PoolGenFree(PoolGen pgen, Seg seg, Size freeSize, Size oldSize,
                 Size newSize, Bool deferred)
{
  Size size;

  AVERT(PoolGen, pgen);
  AVERT(Seg, seg);

  size = SegSize(seg);
  AVER(freeSize + oldSize + newSize == size);

  PoolGenAccountForFree(pgen, size, oldSize, newSize, deferred);

  RingRemove(&SegGCSeg(seg)->genRing);

  SegFree(seg);
}


/* PoolGenDescribe -- describe a PoolGen */

Res PoolGenDescribe(PoolGen pgen, mps_lib_FILE *stream, Count depth)
{
  Res res;
  PoolClass poolClass;

  if (!TESTT(PoolGen, pgen))
    return ResPARAM;
  if (stream == NULL)
    return ResPARAM;

  poolClass = ClassOfPoly(Pool, pgen->pool);
  
  res = WriteF(stream, depth,
               "PoolGen $P {\n", (WriteFP)pgen,
               "  pool $P ($U) \"$S\"\n",
               (WriteFP)pgen->pool, (WriteFU)pgen->pool->serial,
               (WriteFS)ClassName(poolClass),
               "  segs $U\n", (WriteFU)pgen->segs,
               "  totalSize $U\n", (WriteFU)pgen->totalSize,
               "  freeSize $U\n", (WriteFU)pgen->freeSize,
               "  bufferedSize $U\n", (WriteFU)pgen->bufferedSize,
               "  oldSize $U\n", (WriteFU)pgen->oldSize,
               "  oldDeferredSize $U\n", (WriteFU)pgen->oldDeferredSize,
               "  newSize $U\n", (WriteFU)pgen->newSize,
               "  newDeferredSize $U\n", (WriteFU)pgen->newDeferredSize,
               "} PoolGen $P\n", (WriteFP)pgen,
               NULL);
  return res;
}


/* LocusInit -- initialize the locus module */

void LocusInit(Arena arena)
{
  GenParamStruct params;

  AVER(arena != NULL); /* not initialized yet. */
 
  params.capacity = 1; /* unused since top generation is not on any chain */
  params.mortality = 0.5;
 
  GenDescInit(&arena->topGen, &params);
}


/* LocusFinish -- finish the locus module */

void LocusFinish(Arena arena)
{
  /* Can't check arena, because it's being finished. */
  AVER(arena != NULL);
  GenDescFinish(&arena->topGen);
}


/* LocusCheck -- check the locus module */

Bool LocusCheck(Arena arena)
{
  /* Can't check arena, because this is part of ArenaCheck. */
  CHECKD(GenDesc, &arena->topGen);
  return TRUE;
}


/* C. COPYRIGHT AND LICENSE
 *
 * Copyright (C) 2001-2018 Ravenbrook Limited <http://www.ravenbrook.com/>.
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
