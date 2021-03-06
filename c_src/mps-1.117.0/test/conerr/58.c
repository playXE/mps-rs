/* 
TEST_HEADER
 id = $Id: //info.ravenbrook.com/project/mps/version/1.117/test/conerr/58.c#1 $
 summary = isstale on ld in wrong arena
 language = c
 link = myfmt.o testlib.o
END_HEADER
*/

#include "testlib.h"
#include "mpscamc.h"
#include "myfmt.h"

static void test(void)
{
 mps_arena_t arena0;
 mps_arena_t arena1;
 mps_ld_s ld;

 cdie(mps_arena_create(&arena0, mps_arena_class_vm(), mmqaArenaSIZE), "create arena");
 cdie(mps_arena_create(&arena1, mps_arena_class_vm(), mmqaArenaSIZE), "create arena 1");

 mps_ld_reset(&ld, arena0);
 comment("Reset ld."); 

 report("isstale", "%d", mps_ld_isstale(&ld, arena1, &arena0));

 mps_arena_destroy(arena0);
 comment("Destroyed arena.");

 mps_arena_destroy(arena1);
 comment("Destroyed arena.");
}

int main(void)
{
 easy_tramp(test);
 return 0;
}

