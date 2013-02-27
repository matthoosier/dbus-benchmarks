#include <arpa/inet.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#include "socket-util.h"

int main (int argc, char* argv[])
{
    std::string path(getenv("HOME"));
    path += "/socket-test";

    int client_fd = socket(PF_UNIX, SOCK_STREAM, 0);

    struct sockaddr_un addr;
    size_t path_len;

    bzero(&addr, sizeof(addr));
    addr.sun_family = AF_UNIX;
    path_len = strlen(path.c_str());
    strncpy(addr.sun_path, path.c_str(), path_len);

    if (connect(client_fd, (struct sockaddr*)&addr, offsetof(struct sockaddr_un, sun_path) + path_len) != 0) {
        fprintf(stderr, "Couldn't connect to %s: %s\n", path.c_str(), strerror(errno));
        exit(1);
    }

    enum { CALLS = 5000 };

    struct timeval before, after;

    gettimeofday(&before, NULL);

    char message[1024];
    memset(message, 'a', sizeof(message) - 1);
    message[sizeof(message) - 1] = '\0';

    char* ptr_message = &message[0];

    for (unsigned int i = 0; i < CALLS; ++i) {
        uint32_t len = strlen(ptr_message);
        uint32_t len_buf = htonl(len);

        ssize_t n = write_full(client_fd, &len_buf, sizeof(len_buf));

        if (n == 0) {
            fprintf(stderr, "Server disconnected while sending message header\n");
            exit(1);
        }
        else if (n < 0) {
            fprintf(stderr, "Error while sending message header: %s\n", strerror(errno));
            exit(1);
        }

        n = write_full(client_fd, &ptr_message[0], len);

        if (n == 0) {
            fprintf(stderr, "Server disconnected while sending message payload\n");
            exit(1);
        }
        else if (n < 0) {
            fprintf(stderr, "Error while sending message payload: %s\n", strerror(errno));
            exit(1);
        }

        n = read_full(client_fd, &len_buf, sizeof(len_buf));

        if (n < 0) {
            fprintf(stderr, "Couldn't read reply header: %s\n", strerror(errno));
            exit(1);
        }
        else if (n == 0) {
            fprintf(stderr, "Server disconnected during header\n");
            exit(1);
        }

        len = ntohl(len_buf);

        char* buffer = new char[len + 1];
        n = read_full(client_fd, &buffer[0], len);
        buffer[len] = '\0';

        if (n < 0) {
            fprintf(stderr, "Couldn't read reply payload: %s\n", strerror(errno));
            exit(1);
        }
        else if (n == 0) {
            fprintf(stderr, "Server disconnected during payload\n");
            exit(1);
        }

        delete[] buffer;
    }

    gettimeofday(&after, NULL);

    double elapsed = after.tv_sec + (after.tv_usec / (double)1000000);
    elapsed -= before.tv_sec + (before.tv_usec / (double)1000000);

    printf("%d calls in %.3f sec, %f sec per call\n",
           CALLS, elapsed, elapsed / CALLS);

    return 0;
}
