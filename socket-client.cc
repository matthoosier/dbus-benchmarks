#include <arpa/inet.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/un.h>
#include <string.h>
#include <string>

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

    for (unsigned int i = 0; i < 5000; ++i) {
        char message[] = "Hello";
        uint32_t len = strlen(message);
        uint32_t len_buf = htonl(len);

        write(client_fd, &len_buf, sizeof(len_buf));
        write(client_fd, &message[0], len);

        read(client_fd, &len_buf, sizeof(len_buf));
        len = ntohl(len_buf);

        char* buffer = new char[len + 1];
        read(client_fd, &buffer[0], len);
        buffer[len] = '\0';

        delete[] buffer;
    }

    gettimeofday(&after, NULL);

    double elapsed = after.tv_sec + (after.tv_usec / (double)1000000);
    elapsed -= before.tv_sec + (before.tv_usec / (double)1000000);

    printf("%d calls in %.3f sec, %f sec per call\n",
           CALLS, elapsed, elapsed / CALLS);

    return 0;
}
