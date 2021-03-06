/* 
TEST_HEADER
 id = $Id: //info.ravenbrook.com/project/mps/version/1.117/test/conerr/0.c#1 $
 summary = create an arena and then destroy it, twice!
 language = c
 link = testlib.o
OUTPUT_SPEC
 abort = true
END_HEADER
*/

#include "testlib.h"

static void test(void)
{
 mps_arena_t arena;

 cdie(mps_arena_create(&arena, mps_arena_class_vm(), mmqaArenaSIZE), "Create arena");
 mps_arena_destroy(arena);
 comment("Destroy arena once.");
 mps_arena_destroy(arena);
 comment("Destroy arena twice!");
}

int main(void)
{
 easy_tramp(test);
 return 0;
}
