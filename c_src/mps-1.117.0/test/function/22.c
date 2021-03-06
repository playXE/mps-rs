/* 
TEST_HEADER
 id = $Id: //info.ravenbrook.com/project/mps/version/1.117/test/function/22.c#1 $
 summary = allocate large object, free its middle, repeat interleaved
 language = c
 link = testlib.o
 parameters = OBJECTS=2000
END_HEADER
*/

#include "testlib.h"
#include "mpscmv.h"

static void test(void) {
 mps_arena_t arena;
 mps_pool_t pool;
 mps_addr_t q, r;
 int p;

 die(mps_arena_create_k(&arena, mps_arena_class_vm(), mps_args_none), "create");

 die(mps_pool_create(&pool, arena, mps_class_mv(),
                     (size_t)(32), (size_t)(16), (size_t)(256)),
     "create MV pool");

 die(mps_alloc(&q, pool, 1024), "alloc");

 for (p=0; p<OBJECTS; p++) {
  die(mps_alloc(&r, pool, 1024), "alloc");
  mps_free(pool, q, 256-MPS_PF_ALIGN);
  q = (mps_addr_t) ((char *) r + MPS_PF_ALIGN);
 }

 mps_pool_destroy(pool);
 mps_arena_destroy(arena);
}

int main(void) {
 easy_tramp(test);
 pass();
 return 0;
}
