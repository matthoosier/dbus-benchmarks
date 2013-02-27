#include <arpa/inet.h>
#include <errno.h>
#include <pthread.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>

#include <string>

static void * thread_body (void * arg)
{
    int* client_fd_ptr = (int*)arg;
    int client_fd = *client_fd_ptr;
    delete client_fd_ptr;

    while (true)
    {
        uint32_t msg_char_cnt_buf;
        int n = read(client_fd, &msg_char_cnt_buf, sizeof(msg_char_cnt_buf));

        if (n == 0) {
            fprintf(stderr, "Client disconnected\n");
            close(client_fd);
            return NULL;
        }

        if (n != sizeof(msg_char_cnt_buf)) {
            fprintf(stderr, "Incomplete message header\n");
            close(client_fd);
            return NULL;
        }

        uint32_t msg_char_cnt = ntohl(msg_char_cnt_buf);

        char* buffer = new char[msg_char_cnt + 1];

        for (int num_to_read = msg_char_cnt; num_to_read > 0; ) {
            n = read(client_fd, &buffer[msg_char_cnt - num_to_read], num_to_read);
            if (n == 0) {
                fprintf(stderr, "Incomplete message payload\n");
                close(client_fd);
                delete[] buffer;
                return NULL;
            }

            else if (n < 0) {
                fprintf(stderr, "Error reading: %s\n", strerror(errno));
                close(client_fd);
                delete[] buffer;
                return NULL;
            }

            num_to_read -= n;
        }

        buffer[msg_char_cnt] = '\0';

        msg_char_cnt_buf = htonl(msg_char_cnt);
        n = write(client_fd, &msg_char_cnt_buf, sizeof(msg_char_cnt_buf));

        if (n == 0) {
            fprintf(stderr, "Client disconnected\n");
            close(client_fd);
            delete[] buffer;
            return NULL;
        }

        if (n != sizeof(msg_char_cnt_buf)) {
            fprintf(stderr, "Unable to write full message header\n");
            close(client_fd);
            delete[] buffer;
            return NULL;
        }

        for (int num_to_write = msg_char_cnt; num_to_write > 0; ) {
            n = write(client_fd, &buffer[msg_char_cnt - num_to_write], num_to_write);

            if (n == 0) {
                fprintf(stderr, "Incomplete message payload write\n");
                close(client_fd);
                delete[] buffer;
                return NULL;
            }

            else if (n < 0) {
                fprintf(stderr, "Error writing: %s\n", strerror(errno));
                close(client_fd);
                delete[] buffer;
                return NULL;
            }

            num_to_write -= n;
        }

        delete[] buffer;
    }
}

int main (int argc, char* argv[])
{
    int listen_fd = socket(PF_UNIX, SOCK_STREAM, 0);

    std::string path(getenv("HOME"));
    path += "/socket-test";

    // Clean out any previous stale copy of the socket
    unlink(path.c_str());

    struct sockaddr_un addr;
    size_t path_len;

    bzero(&addr, sizeof(addr));
    addr.sun_family = AF_UNIX;
    path_len = strlen(path.c_str());
    strncpy(addr.sun_path, path.c_str(), path_len);

    if (bind(listen_fd, (struct sockaddr*)&addr, offsetof(struct sockaddr_un, sun_path) + path_len) < 0) {
        fprintf(stderr, "Couldn't set socket name: %s\n", strerror(errno));
        exit(1);
    }

    if (listen(listen_fd, 30) < 0) {
        unlink(path.c_str());
        fprintf(stderr, "Couldn't listen on socket: %s\n", strerror(errno));
        exit(1);
    }

    while (true)
    {
        struct sockaddr client_addr;
        socklen_t client_addr_len;

        printf("Waiting for client...\n");
        int client_fd = accept(listen_fd, &client_addr, &client_addr_len);

        if (client_fd < 0) {
            fprintf(stderr, "Couldn't accept(): %s\n", strerror(errno));
            exit(1);
        }

        int* arg = new int;
        *arg = client_fd;

        // Fire thread to handle the client
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
        pthread_t thread;
        pthread_create(&thread, &attr, thread_body, arg);
        pthread_attr_destroy(&attr);
    }

    return 0;
}
