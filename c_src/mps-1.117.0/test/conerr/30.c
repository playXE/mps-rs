/* 
TEST_HEADER
 id = $Id: //info.ravenbrook.com/project/mps/version/1.117/test/conerr/30.c#1 $
 summary = destroy an uncreated AP
 language = c
 link = testlib.o
OUTPUT_SPEC
 abort = true
END_HEADER
*/

#include "testlib.h"
#include "mpsclo.h"

static void test(void)
{
 mps_arena_t arena;
 mps_pool_t pool;
 mps_fmt_t format;
 mps_ap_t ap = (mps_ap_t)1;

 cdie(mps_arena_create(&arena, mps_arena_class_vm(), mmqaArenaSIZE), "create arena");

 cdie(mps_fmt_create_k(&format, arena, mps_args_none), "create format");

 cdie(mps_pool_create(&pool, arena, mps_class_lo(), format), "create pool");

 mps_ap_destroy(ap);
 comment("Destroyed ap.");

 mps_pool_destroy(pool);
 comment("Destroyed pool.");

 mps_fmt_destroy(format);
 comment("Destroyed format.");

 mps_arena_destroy(arena);
 comment("Destroyed arena.");
}

int main(void)
{
 easy_tramp(test);
 return 0;
}
