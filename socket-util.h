#ifndef __SOCKET_UTIL_H__
#define __SOCKET_UTIL_H__

#include <stddef.h>
#include <unistd.h>

static inline size_t read_full (int fd, void* buf, size_t byte_count)
{
    char* buf_addr = (char*)buf;

    for (size_t remaining = byte_count; remaining > 0; )
    {
        ssize_t n = read(fd, &buf_addr[byte_count - remaining], remaining);

        if (n > 0) {
            remaining -= n;
        }
        else if (n == 0) {
            return 0;
        }
        else {
            switch (n) {
                case EAGAIN:
                case EINTR:
                    /* Recoverable; just iterate again */
                    break;

                default:
                    /* Not recoverable; return error to caller */
                    return n;
                    break;
            }
        }
    }

    return byte_count;
}

static inline ssize_t write_full (int fd, void const* buf, size_t byte_count)
{
    char const* buf_addr = (char const*)buf;

    for (size_t remaining = byte_count; remaining > 0; )
    {
        ssize_t n = write(fd, &buf_addr[byte_count - remaining], remaining);

        if (n > 0) {
            remaining -= n;
        }
        else if (n == 0) {
            return 0;
        }
        else {
            switch (n) {
                case EAGAIN:
                case EINTR:
                    /* Recoverable; just iterate again */
                    break;

                default:
                    /* Not recoverable; return error to caller */
                    return n;
                    break;
            }
        }
    }

    return byte_count;
}

#endif
