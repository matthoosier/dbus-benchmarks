#ifndef __DBUS_UTIL_H__
#define __DBUS_UTIL_H__

#include <stdlib.h>
#include <stdio.h>

static inline void enforce_error (DBusError const* error)
{
    if (dbus_error_is_set(error)) {
        fprintf(stderr, "%s: %s\n", error->name, error->message);
        fflush(stderr);
        abort();
    }
}

#endif
