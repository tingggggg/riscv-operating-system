#include "os.h"

#define DELAY 1000

extern void trap_test();

void user_task0(void)
{
    uart_puts("Task 0: Created!\n");
    char s[] = "12345";

    task_yield();
    uart_puts("Task 0: I'm back!\n");
    while (1) {
        for (int i = 0; i < 5; i++) {
            uart_putc(s[i]);
            task_delay(DELAY);
        }
    }
}

void user_task1(void)
{
    uart_puts("Task 1: Created!\n");
    char s[] = "abcde";
    while (1) {
        for (int i = 0; i < 5; i++) {
            uart_putc(s[i]);
            task_delay(DELAY);
        }
    }
}

void os_main(void)
{
    task_create(user_task0);
    task_create(user_task1);
}
