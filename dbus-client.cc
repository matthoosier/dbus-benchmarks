#include <dbus/dbus.h>
#include <stdio.h>
#include <sys/time.h>
#include <string.h>

#include "dbus-strings.h"
#include "dbus-util.h"

static void send_message (DBusConnection* conn, DBusError* error)
{
    DBusMessage* message = dbus_message_new_method_call(
                ECHO_SERVICE_NAME,
                ECHO_SERVICE_PATH,
                ECHO_SERVICE_INTERFACE,
                ECHO_SERVICE_METHODNAME_ECHO);

    if (!message) {
        printf("Didn't allocate message\n");
        abort();
    }

    static bool inited = false;
    static char sent_string[1024];
    char* ptr_sent_string = &sent_string[0];

    if (!inited) {
        memset(sent_string, 'a', sizeof(sent_string) - 1);
        sent_string[sizeof(sent_string) - 1] = '\0';
        inited = true;
    }

    dbus_message_append_args(message, DBUS_TYPE_STRING, &ptr_sent_string, DBUS_TYPE_INVALID);

    DBusMessage* reply = dbus_connection_send_with_reply_and_block(conn, message, DBUS_TIMEOUT_INFINITE, error);
    enforce_error(error);

    dbus_message_unref(message);
    message = NULL;

    char const* replied_string;

    dbus_message_get_args(reply, error, DBUS_TYPE_STRING, &replied_string, DBUS_TYPE_INVALID);
    enforce_error(error);

    dbus_message_unref(reply);
    reply = NULL;
}

int main (int argc, char* argv[])
{
    DBusError error;
    dbus_error_init(&error);

    #if defined(__QNX__)
        DBusConnection* conn = dbus_bus_get(DBUS_BUS_SYSTEM, &error);
    #else
        DBusConnection* conn = dbus_bus_get(DBUS_BUS_SESSION, &error);
    #endif

    enforce_error(&error);

    enum { CALLS = 5000 };

    struct timeval before, after;
    gettimeofday(&before, NULL);

    for (unsigned int i = 0; i < CALLS; ++i) {
        send_message(conn, &error);
    }

    gettimeofday(&after, NULL);

    double elapsed = after.tv_sec + (after.tv_usec / (double)1000000);
    elapsed -= before.tv_sec + (before.tv_usec / (double)1000000);

    printf("%d messages in %2.3f sec, %2.5f sec per message\n",
           CALLS, elapsed, elapsed / CALLS);

    return 0;
}
