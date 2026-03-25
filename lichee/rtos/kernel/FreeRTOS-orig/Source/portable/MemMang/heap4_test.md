# 内存追踪
heap_4.c集成了一个内存追踪测试代码，原理为每malloc一次，就会把这次malloc的backtrace生成一个节点，并加入到链表中，停止追踪时，就可以知道多大的内存没有释放，并且这个没有释放的内存的backtrace。
## 使用方法
1. 打开LI_MALLOC_TEST宏
2. 调用start_malloc_trace，开始追踪。
2. 调用finish_malloc_trace_and_show，停止追踪，并打印没有释放的内存的backtrace。