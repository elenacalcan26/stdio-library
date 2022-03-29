#include <sys/types.h>	/* open */
#include <sys/stat.h>	/* open */
#include <fcntl.h>	/* O_RDWR, O_CREAT, O_TRUNC, O_WRONLY */
#include <unistd.h>	/* close */
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>


#include "so_stdio.h"

#define BUFF_LEN 4096

struct _so_file {
    int fd;
    char *buffer;
    int buff_pos;
    int currsor;
    int buff_read_len;
    int feof_flag;
    int ferror_flag;
    char last_op; // r = read; w = write
    int pid;
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
        fd = open(pathname, O_APPEND | O_RDWR | O_CREAT, 0644);
    } else {
        return NULL;
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
    so_file->currsor = 0;

    return so_file;
}

int so_fileno(SO_FILE *stream) {
    return stream->fd;
}

int so_fflush(SO_FILE *stream) {
    int r;
    if (stream->last_op == 'w') {
        stream->buff_pos = 0;
        while (stream->buff_read_len > 0) {
            r = write(stream->fd, stream->buffer + stream->buff_pos, stream->buff_read_len);

            if (r >= 0) {
                stream->buff_read_len -= r;
                stream->buff_pos += r;
                stream->currsor += r;
            } else {
                stream->ferror_flag = SO_EOF;
                return SO_EOF;
            }
        }
        stream->buff_pos = 0;
    }
    memset(stream->buffer, 0, stream->buff_read_len);
    stream->buff_read_len = 0;
    return 0;
}

int so_fseek(SO_FILE *stream, long offset, int whence) {
    int r;

    if (stream->last_op == 'r') {
        stream->buff_read_len = 0;
        stream->buff_pos = 0;
        memset(stream->buffer, 0, stream->buff_read_len);
    } else if (stream->last_op == 'w') {
        r = so_fflush(stream);

        if (r < 0) {
            stream->feof_flag = SO_EOF;
            return SO_EOF;
        }

    }
    r = lseek(stream->fd, offset, whence);

    if (r < 0) {
        stream->ferror_flag = SO_EOF;
        return SO_EOF;
    }

    stream->currsor = r;

    return 0;
}

long so_ftell(SO_FILE *stream) {

    if (stream->last_op != 'r') {
        int r = so_fseek(stream, 0, SEEK_CUR);

        if (r < 0) {
            return SO_EOF;
        }
    }

    return stream->currsor;
}

size_t so_fread(void *ptr, size_t size, size_t nmemb, SO_FILE *stream) {
    int read_elem = size * nmemb;
    int ch, i;

    if (stream->ferror_flag != 0 || stream->feof_flag != 0) {
        return 0;
    }

    for (i = 0; i < read_elem; i++) {
        ch = so_fgetc(stream);

        if (ch < 0) {
           if (so_feof(stream) != 0) {
               return i / size;
           } else {
               return 0;
           }
        }

        ((char *)ptr)[i] = ch;
    }

    return read_elem / size;
}

size_t so_fwrite(const void *ptr, size_t size, size_t nmemb, SO_FILE *stream) {
    int write_elem = size * nmemb;
    int i, r;
    char ch;

    for (i = 0; i < write_elem; i++) {

        ch = ((char *) ptr)[i];
        r = so_fputc(ch, stream);

        // if (so_ferror(stream) == SO_EOF || so_feof(stream) == SO_EOF) {
        //     break;
        // }
    }

    return nmemb;
}

int so_fgetc(SO_FILE *stream) {
    int r;
    unsigned char ch;

    if (stream->buff_pos == 0 || stream->buff_pos >= stream->buff_read_len) {

        memset(stream->buffer, 0, stream->buff_pos);

        r = read(stream->fd, stream->buffer, BUFF_LEN);

        if (r < 0) {
            stream->ferror_flag = SO_EOF;
            return SO_EOF;
        }

        if (r == 0) {
            stream->feof_flag = SO_EOF;
            return SO_EOF;
        }
        stream->buff_pos = 0;
        stream->buff_read_len = r;
    }

    ch = stream->buffer[stream->buff_pos];
    stream->buff_pos++;
    stream->last_op = 'r';

    stream->currsor++;

    return (unsigned int)ch;
}

int so_fputc(int c, SO_FILE *stream) {
    int r;

    stream->last_op = 'w';

    if (stream->buff_read_len == BUFF_LEN) {

        int r = so_fflush(stream);

        if (r != 0) {
            stream->ferror_flag = -1;
            return SO_EOF;
        }
    }

    stream->buffer[stream->buff_read_len] = c;
    stream->buff_read_len++;
    stream->currsor++;

    return c;
}

int so_feof(SO_FILE *stream) {
    return stream->feof_flag;
}

int so_ferror(SO_FILE *stream) {
    return stream->ferror_flag;
}

SO_FILE *so_popen(const char *command, const char *type) {
    pid_t pid;
    int status, r, fd = -1;
    int fields[2];
    SO_FILE *stream;

    pipe(fields);

    pid = fork();
    switch (pid) {
	case -1:
		close(fields[0]);
        close(fields[1]);

		return NULL;
	case 0:
		/* child process */
        if (strstr(type, "r") != NULL) {
            dup2(fields[1], STDOUT_FILENO);
            close(fields[0]);
        } else if (strstr(type, "w") != NULL) {
            dup2(fields[0], STDIN_FILENO);
            close(fields[1]);
        }

        execlp("/bin/sh", "sh", "-c", command, NULL);

		/* only if exec failed */
		return NULL;
	default:
		/* parent process */
		break;
	}

    if (strstr(type, "r") != NULL) {
        fd = fields[0];
        close(fields[1]);
    } else if (strstr(type, "w") != NULL) {
        fd = fields[1];
        close(fields[0]);
    }

    stream = (SO_FILE*)malloc(sizeof(SO_FILE));

    if (stream == NULL)
        return NULL;

    stream->buffer = (char *)calloc(BUFF_LEN, sizeof(char));

    if (stream->buffer == NULL) {
        return NULL;
    }

    stream->pid = pid;
    stream->fd = fd;
    stream->buff_pos = 0;
    stream->feof_flag = 0;
    stream->ferror_flag = 0;
    stream->buff_read_len = 0;
    stream->last_op ='\0';
    stream->currsor = 0;

    return stream;
}

int so_pclose(SO_FILE *stream) {
    int pid, status, wait;

    pid = stream->pid;
    so_fclose(stream);
    wait = waitpid(pid, &status, 0);

    if (wait < 0) {
        return -1;
    }

    return 0;
}

int so_fclose(SO_FILE *stream) {
    int ret;

    if (stream->last_op == 'w') {
        ret = so_fflush(stream);
        if (ret < 0) {
            free(stream->buffer);
            free(stream);
            return SO_EOF;
        }
    }

    ret = close(stream->fd);

    if (ret < 0) {
        free(stream->buffer);
        free(stream);
        return SO_EOF;
    }

    free(stream->buffer);
    free(stream);

    return 0;
}
