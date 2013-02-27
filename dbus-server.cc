#include <dbus/dbus.h>
#include <unistd.h>
#include "dbus-util.h"
#include "dbus-strings.h"

static void handle_echo (DBusMessage* msg, DBusConnection* conn)
{
    DBusMessageIter args;
    char* param = NULL;

    if (!dbus_message_iter_init(msg, &args)) {
        fprintf(stderr, "Message has no arguments\n");
        abort();
    }

    if (dbus_message_iter_get_arg_type(&args) != DBUS_TYPE_STRING) {
        fprintf(stderr, "Unexpcted argument type\n");
        abort();
    }

    dbus_message_iter_get_basic(&args, &param);

    DBusMessage* reply = dbus_message_new_method_return(msg);

    dbus_message_iter_init_append(reply, &args);

    if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &param)) {
        fprintf(stderr, "Out of memory\n");
        abort();
    }

    dbus_uint32_t serial = 0;
    if (!dbus_connection_send(conn, reply, &serial)) {
        fprintf(stderr, "Out of memory sending reply\n");
        abort();
    }

    dbus_message_unref(reply);
}

static DBusHandlerResult message_handler (DBusConnection* conn,
                                          DBusMessage* msg,
                                          void* user_data)
{
    if (dbus_message_is_method_call(msg, ECHO_SERVICE_INTERFACE, ECHO_SERVICE_METHODNAME_ECHO)) {
        handle_echo(msg, conn);
        return DBUS_HANDLER_RESULT_HANDLED;
    }
    else {
        return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
    }
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

    int ret = dbus_bus_request_name(conn, ECHO_SERVICE_NAME, 0, &error);
    enforce_error(&error);

    if (ret != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER)
    {
        fprintf(stderr, "Didn't get name\n");
        abort();
    }

    if (!dbus_connection_add_filter(conn, message_handler, NULL, NULL))
    {
        fprintf(stderr, "Couldn't install filter\n");
        abort();
    }

    while (dbus_connection_read_write_dispatch(conn, DBUS_TIMEOUT_INFINITE)) {
        ;
    }
    
    return 0;
}
