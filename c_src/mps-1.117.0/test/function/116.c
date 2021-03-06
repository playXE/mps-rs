/* 
TEST_HEADER
 id = $Id: //info.ravenbrook.com/project/mps/version/1.117/test/function/116.c#1 $
 summary = allocation shouldn't fail if there's garbage to collect
 language = c
 link = testlib.o rankfmt.o
END_HEADER
*/

#include "testlib.h"
#include "mpscamc.h"
#include "mpsavm.h"
#include "rankfmt.h"

#define genCOUNT (3)

static mps_gen_param_s testChain[genCOUNT] = {
  { 6000, 0.90 }, { 8000, 0.65 }, { 16000, 0.50 } };

void *stackpointer;

mps_pool_t poolmv;
mps_arena_t arena;

static void test(void)
{
 mps_pool_t pool;
 mps_thr_t thread;

 mps_root_t root;

 mps_chain_t chain;
 mps_fmt_t format;
 mps_ap_t ap, ap2;

 mycell *a, *b;

 mps_res_t res;
 int i;

/* create an arena that can't grow beyond 30 M */

 cdie(mps_arena_create(&arena, mps_arena_class_vm(), (size_t) (1024*1024*30)),
  "create arena");
 die(mps_arena_commit_limit_set(arena, 1ul << 20), "commit_limit_set");

 cdie(mps_thread_reg(&thread, arena), "register thread");

 cdie(mps_root_create_reg(&root, arena, mps_rank_ambig(), 0, thread,
  mps_stack_scan_ambig, stackpointer, 0), "create root");

 cdie(
  mps_fmt_create_A(&format, arena, &fmtA),
  "create format");

 cdie(mps_chain_create(&chain, arena, genCOUNT, testChain), "chain_create");

 cdie(
  mps_pool_create(&pool, arena, mps_class_amc(), format, chain),
  "create pool");

 cdie(
  mps_ap_create(&ap, pool, mps_rank_exact()),
  "create ap");

/* allocate until full */


 i = 0;

 b = NULL;

 while (allocrone(&a, ap, 128, mps_rank_exact()) == MPS_RES_OK) {
  i++;
  setref(a, 0, b);
  b = a;
 }

 comment("%d objs allocated.", i);

/* try to allocate 10 times */

 cdie(mps_ap_create(&ap2, pool, mps_rank_exact()), "create second ap");
 mps_ap_destroy(ap);

 for (i = 0; i < 10; i++) {
  res = allocrone(&a, ap2, 128, mps_rank_exact());
  report("predie", "%s", err_text(res));
 }

/* now let everything die, and try to allocate 10 times */

 mps_root_destroy(root);

 for (i = 0; i < 10; i++) {
  res = allocrone(&a, ap2, 128, mps_rank_exact());
  report("postdie", "%s", err_text(res));
 }

 mps_arena_park(arena);
 mps_ap_destroy(ap2);
 comment("Destroyed ap.");

 mps_pool_destroy(pool);
 comment("Destroyed pool.");

 mps_fmt_destroy(format);
 comment("Destroyed format.");

 mps_chain_destroy(chain);
 comment("Destroyed chain.");

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
 pass();
 return 0;
}
