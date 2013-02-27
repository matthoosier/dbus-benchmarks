#include <dbus/dbus.h>
#include <stdio.h>
#include <sys/time.h>

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

    char const* sent_string = "A";

    dbus_message_append_args(message, DBUS_TYPE_STRING, &sent_string, DBUS_TYPE_INVALID);

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

    DBusConnection* conn = dbus_bus_get(DBUS_BUS_SESSION, &error);
    enforce_error(&error);

    enum { CALLS = 30000 };

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
