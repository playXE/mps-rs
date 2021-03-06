/* 
TEST_HEADER
 id = $Id: //info.ravenbrook.com/project/mps/version/1.117/test/conerr/45.c#1 $
 summary = register thread in uncreated arena
 language = c
 link = myfmt.o testlib.o
OUTPUT_SPEC
 abort = true
END_HEADER
*/

#include "testlib.h"
#include "mpscamc.h"
#include "myfmt.h"

void *stackpointer;

static void test(void)
{
 mps_arena_t arena = (mps_arena_t)1;
 mps_thr_t thread;

/*
 cdie(mps_arena_create(&arena, mps_arena_class_vm(), mmqaArenaSIZE), "create arena");
*/

 cdie(mps_thread_reg(&thread, arena), "register thread");

 mps_thread_dereg(thread);
 comment("Deregistered thread.");

 mps_arena_destroy(arena);
 comment("Destroyed arena.");
}

int main(void)
{
 void *m;
 stackpointer=&m; /* hack to get stack pointer */

 easy_tramp(test);
 return 0;
}

