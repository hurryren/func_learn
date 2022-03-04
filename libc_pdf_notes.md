https://www.gnu.org/software/libc/manual/pdf/libc.pdf









### chapter 3 Vitual memory allocation and paging

---

The C language supports two kinds of memory allocation through the variables in C programs:

-   *static allocation* is what happens when you declare a static or global variable. Each static or global variable defines one block of space, of a fixed size. The space is allocated once, when your program is started (part of the exec operation), and is never freed.
-   Automatic allocation happens when you declare an automatic varible, such as a function argument or a local varibale. The space for an automatic variable is allocated when the compond statement containing the declaration is entered, and is freed when that compound statement is exited.