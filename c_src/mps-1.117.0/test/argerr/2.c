/* 
TEST_HEADER
 id = $Id: //info.ravenbrook.com/project/mps/version/1.117/test/argerr/2.c#1 $
 summary = destroy an arena with an null arena_t
 language = c
 link = testlib.o
OUTPUT_SPEC
 assert = true
 assertfile P= global.c
 assertcond = TESTT(Arena, arena)
END_HEADER
*/

#include "testlib.h"
#include "arg.h"

static void test(void)
{
 mps_arena_t arena;

 cdie(mps_arena_create(&arena, mps_arena_class_vm(), mmqaArenaSIZE),
      "Create arena");
 mps_arena_destroy(NULL);
}

int main(void)
{
 easy_tramp(test);
 return 0;
}
