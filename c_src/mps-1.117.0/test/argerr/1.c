/* 
TEST_HEADER
 id = $Id: //info.ravenbrook.com/project/mps/version/1.117/test/argerr/1.c#1 $
 summary = create an arena with an unaligned arena_t
 language = c
 link = testlib.o
OUTPUT_SPEC
 abort = true
END_HEADER
*/

#include "testlib.h"
#include "arg.h"

static void test(void)
{
 adie(mps_arena_create(UNALIGNED, mps_arena_class_vm(), mmqaArenaSIZE),
      "Create arena");
}

int main(void)
{
 easy_tramp(test);
 return 0;
}
