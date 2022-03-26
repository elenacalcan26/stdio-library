#include <sys/types.h>	/* open */
#include <sys/stat.h>	/* open */
#include <fcntl.h>	/* O_RDWR, O_CREAT, O_TRUNC, O_WRONLY */
#include <unistd.h>	/* close */
#include <stdlib.h>
#include <string.h>

#include "so_stdio.h"

#define BUFF_LEN 4096

struct _so_file {
    int fd;
    char *buffer;
    int buff_pos;
    int buff_read_len;
    int feof_flag;
    int ferror_flag;
    char last_op; // r = read; w = write
};

SO_FILE *so_fopen(const char *pathname, const char *mode) {
    SO_FILE *so_file;
    int fd = -1;

    if (strcmp(mode, "r") == 0) {
        fd = open(pathname, O_RDONLY, 0644);
    } else if (strcmp(mode, "r+") == 0) {
        fd = open(pathname, O_RDWR, 0644);
    } else if (strcmp(mode, "w") == 0) {
        fd = open(pathname, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    } else if (strcmp(mode, "w+") == 0) {
        fd = open(pathname, O_RDWR | O_CREAT | O_TRUNC, 0644);
    } else if (strcmp(mode, "a") == 0) {
        fd = open(pathname, O_APPEND | O_WRONLY | O_CREAT, 0644);
    } else if (strcmp(mode, "a+") == 0) {
        fd = open(pathname, O_APPEND | O_RDONLY | O_CREAT, 0644);
    }

    if (fd < 0) {
        return NULL;
    }

    so_file = (SO_FILE *)malloc(sizeof(SO_FILE));

    if (so_file == NULL) {
        return NULL;
    }

    so_file->buffer = (char*)calloc(BUFF_LEN, sizeof(char));

    if (so_file->buffer == NULL) {
        so_file->ferror_flag = -1;
        return NULL;
    }

    so_file->fd = fd;
    so_file->buff_pos = 0;
    so_file->feof_flag = 0;
    so_file->ferror_flag = 0;
    so_file->buff_read_len = 0;
    so_file->last_op ='\0';

    return so_file;
}

int so_fileno(SO_FILE *stream) {
    return stream->fd;
}

int so_fflush(SO_FILE *stream) {
    int r;
    if (stream->last_op == 'w') {
        r = write(stream->fd, stream->buffer, stream->buff_read_len);

        if (r < 0) {
            stream->ferror_flag = -1;
            return SO_EOF;
        }

    }

    return 0;
}

int so_fseek(SO_FILE *stream, long offset, int whence) {
    return 0;
}

long so_ftell(SO_FILE *stream) {
    return stream->buff_pos;
}

size_t so_fread(void *ptr, size_t size, size_t nmemb, SO_FILE *stream) {
    int read_elem = size * nmemb;
    int ch, i, j;

    for (i = 0; i < read_elem; i++) {
        ch = so_fgetc(stream);

        if (ch < 0) {
            return 0;
        }

        ((char *)ptr)[i] = ch;
    }

    return read_elem / size;
}

size_t so_fwrite(const void *ptr, size_t size, size_t nmemb, SO_FILE *stream) {
    return 0;
}

int so_fgetc(SO_FILE *stream) {
    int r;
    unsigned char ch;

    if (stream->feof_flag != 0 || stream->buff_pos == SO_EOF) {
        return SO_EOF;
    }

    if (stream->buff_pos == 0 || stream->buff_pos >= BUFF_LEN ||
        stream->buff_pos >= stream->buff_read_len) {

        r = read(stream->fd, stream->buffer, BUFF_LEN);

        if (r < 0) {
            stream->ferror_flag = -1;
            return SO_EOF;
        }

        if (r == 0) {
            stream->feof_flag = -1;
            return SO_EOF;
        }
        stream->buff_pos = 0;
        stream->buff_read_len = r;
    }

    ch = stream->buffer[stream->buff_pos];
    stream->buff_pos++;
    stream->last_op = 'r';

    return (unsigned int)ch;
}

int so_fputc(int c, SO_FILE *stream) {
    int r;

    if (stream->buff_read_len >= BUFF_LEN) {
        int r = write(stream->fd, stream->buffer, stream->buff_read_len);

        if (r < 0) {
            stream->ferror_flag = -1;
            return SO_EOF;
        }

        if (r == 0) {
            stream->feof_flag = -1;
            return SO_EOF;
        }

        stream->buff_read_len = 0;
        stream->buff_pos = 0;
    }

    stream->buffer[stream->buff_pos] = (char)c;
    stream->buff_pos++;
    stream->buff_read_len++;
    stream->last_op = 'w';
    return (unsigned int)stream->buffer[stream->buff_pos - 1];
}

int so_feof(SO_FILE *stream) {
    return stream->feof_flag;
}

int so_ferror(SO_FILE *stream) {
    return stream->ferror_flag;
}

SO_FILE *so_popen(const char *command, const char *type) {
    return 0;
}

int so_pclose(SO_FILE *stream) {
    return 0;
}

int so_fclose(SO_FILE *stream) {
    int ret;

    ret = so_fflush(stream);

    if (ret == SO_EOF) {
        return SO_EOF;
    }

    ret = close(stream->fd);
    free(stream->buffer);
    free(stream);

    if (ret < 0) {
        return SO_EOF;
    }

    return 0;
}