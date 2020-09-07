/* $Id: //info.ravenbrook.com/project/mps/version/1.117/test/test/testlib/platform.h#1 $
   load appropriate header files to do platform-specific
   stuff.
*/

#ifdef MPS_OS_W3
#include "mpswin.h"
/* to trap access violations in the test harness */
LONG mySEHFilter(LPEXCEPTION_POINTERS);
#endif

