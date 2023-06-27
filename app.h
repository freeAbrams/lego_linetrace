#include "target_test.h"

#ifndef STACK_SIZE
#define STACK_SIZE  4096    /*タスクのスタックサイズ*/
#endif /*STACK_SIZE*/

/*関数プロトタイプ宣言*/
#ifndef TOPPERS_MACRO_ONLY
extern void main_task(intptr_t exinf);
extern void run_task(intptr_t exinf);
extern void log_task(intptr_t exinf);
#endif /*TOPPERS MACRO ONLY*/
