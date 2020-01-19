#include <common.h>
#include <chicken/thread.h>
#include <chicken/thread/futex.h>
#include <errno.h>

int sys_futex(int *uaddr, int futex_op, int val, const struct timespec *timeout, int *uaddr2, int val3)
{
    printf("FUTEX: %p %i %i %p %p %i\n", uaddr, futex_op, val, timeout, uaddr2, val3);

    return -ENOSYS;
}