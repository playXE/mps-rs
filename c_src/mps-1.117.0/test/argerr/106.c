/* 
TEST_HEADER
 id = $Id: //info.ravenbrook.com/project/mps/version/1.117/test/argerr/106.c#1 $
 summary = highbit set rank for mps_root_create_table
 language = c
 link = testlib.o
OUTPUT_SPEC
 assert = true
 assertfile P= ref.c
 assertcond = rank < RankLIMIT
END_HEADER
*/

#include "testlib.h"
#include "arg.h"

void *stackpointer;

static void test(void)
{
 mps_arena_t arena;
 mps_thr_t thread;
 mps_root_t root;
 mps_addr_t a[30];

 cdie(mps_arena_create(&arena, mps_arena_class_vm(), mmqaArenaSIZE), "create arena");

 cdie(mps_thread_reg(&thread, arena), "register thread");

 cdie(mps_root_create_table(&root, arena,
        HIGHBIT_INT+mps_rank_ambig(), 0, a, sizeof a),
      "root create");

}

int main(void)
{
 void *m;
 stackpointer=&m; /* hack to get stack pointer */

 easy_tramp(test);
 return 0;
}
