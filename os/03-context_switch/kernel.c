#include "os.h"

extern void uart_init(void);
extern void uart_puts(char *s);
extern void sched_init(void);
extern void schedule(void);

void start_kernel(void)
{
    uart_init();
    uart_puts("Hello, RVOS!\n");

    page_init();
    // page_test();

    sched_init();
    schedule();
    
    while (1) {}; // loop here
}