# 链表（list)

链表结构体定义位置： include/linux/types.h

```c
struct list_head {
    struct list_head *next, *prev;
}
```

相关函数：include/linux/list.h



## 1

### 1.1 INIT_LIST_HEAD

```c
/**
 * INIT_LIST_HEAD - Initialize a list_head structure
 * @list: list_head structure to be initialized
 *
 * Initializes the list_head to point to itself. If it is a list header,
 * the result is an empty list.
 */
static inline void INIT_LIST_HEAD(struct list_head *list)
{
    WRITE_ONCE(list->next, list);
    list->prev = list;
}
```

初始话就是把链表的前指针和后指针都指向自己。

[WRITE_ONCE讲解](https://zhuanlan.zhihu.com/p/463861185)

这里用了 WRITE_ONCE 这个宏。这个宏展开后几乎可以等同于 “list->next = list”。使用这个宏的原因是避免编译器优化。 比如缓存优化，顺序优化等。WRITE_ONCE 的原理就是 C 语言中的关键字：volatile。主要作用是告诉编译器，每次读写都不要使用寄存器或者缓存中的值，而是读写该变量所在的内存地址。

那为什么 list->prev 不需要 WRITE_ONCE 呢？

>   Prevent the compiler from merging or refetching reads or writes.

>   ensuring that the compiler does not fold, spindle, or otherwise mutilate accesses that either do not require ordering or that interact with an explicit memory barrier or atomic instruction that provides the required ordering.

[stackoverflow的讨论](https://stackoverflow.com/questions/34988277/write-once-in-linux-kernel-lists)

list 是无锁化设计，且 API 都是内联函数，对 next 使用 WRITE_ONCE 主要是为了保证原子化操作（避免超过 16bit 的变量赋值被拆分成多个指令）。next 的非原子操作（未及时会写内存）容易引发链表异常。其他变量不容易出错。

WRITE_ONCE位置：include/asm-generic/rwonce.h

```c
#define WRITE_ONCE(x, val)    \
do {	\
	compiletime_assert_rwonce_type(x); \
	__WRITE_ONCE(x, val); \
} while (0)
```

compiletime_assert_rwonce_type是为了在编译时检查参数的合法性，即x参数是否可以执行 WRITE_ONCE。












## 资料查询

