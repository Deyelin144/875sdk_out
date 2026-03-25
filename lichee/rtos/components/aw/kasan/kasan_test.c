#include <console.h>
#include "cmd_util.h"

#ifdef CONFIG_KASAN
#if 0
volatile char kasan_test_var1[4];

int test_kasan_global(void)
{

        printf("kasan global var test start.\n");
	kasan_test_var1[6] = 0x12;

        printf("kasan global var test end.\n");
	return 0;
}
int test_kasan_stack(void)
{
        printf("kasan global stack test start.\n");
	char vv[20];
	vv[20] = 100;
        printf("kasan global stack test end.\n");
	return 0;

}

int test_kasan_heap(void)
{
        printf("kasan heap test start.\n");
	char *heap_test_src = pvPortMalloc(64);
	memset(heap_test_src, 0x11, 68);
//	char *heap_test_dst = pvPortMalloc(64);
//	memcpy(heap_test_dst, heap_test_src, 64);
	vPortFree(heap_test_src);
	memset(heap_test_src+8, 0x11, 30);
        printf("kasan heap test end.\n");
	return 0;

}
#else
int test_kasan_global(void)
{

	return 0;
}
int test_kasan_stack(void)
{
	return 0;

}

int test_kasan_heap(void)
{
	return 0;

}
#endif

int cmd_kasan_stack(int argc, char ** argv)
{
    test_kasan_stack();
}
int cmd_kasan_global(int argc, char ** argv)
{
    test_kasan_global();
}
int cmd_kasan_heap(int argc, char ** argv)
{
    test_kasan_heap();
}


FINSH_FUNCTION_EXPORT_CMD(cmd_kasan_stack, kasan_stack, stack detect);
FINSH_FUNCTION_EXPORT_CMD(cmd_kasan_global, kasan_global, global var detect);
FINSH_FUNCTION_EXPORT_CMD(cmd_kasan_heap, kasan_heap, heap detect);
#endif
