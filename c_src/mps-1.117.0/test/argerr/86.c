/* 
TEST_HEADER
 id = $Id: //info.ravenbrook.com/project/mps/version/1.117/test/argerr/86.c#1 $
 summary = NULL arena for is_stale
 language = c
 link = testlib.o
OUTPUT_SPEC
 assert = true
 assertfile P= ld.c
 assertcond = TESTT(Arena, arena)
END_HEADER
*/

#include "testlib.h"
#include "arg.h"

void *stackpointer;

static void test(void)
{
 mps_arena_t arena;
 mps_ld_s ld;
 mps_thr_t thread;
 mps_addr_t p;

 cdie(mps_arena_create(&arena, mps_arena_class_vm(), mmqaArenaSIZE), "create arena");

 cdie(mps_thread_reg(&thread, arena), "register thread");

 mps_ld_reset(&ld, arena);

 mps_ld_add(&ld, arena, &p);

 mps_ld_isstale(&ld, NULL, &p);
}

int main(void)
{
 void *m;
 stackpointer=&m; /* hack to get stack pointer */

 easy_tramp(test);
 return 0;
}
