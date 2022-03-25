#include <sys/types.h>	/* open */
#include <sys/stat.h>	/* open */
#include <fcntl.h>	/* O_RDWR, O_CREAT, O_TRUNC, O_WRONLY */
#include <unistd.h>	/* close */
#include <stdlib.h>
#include <string.h>

#include "so_stdio.h"

#define BUFF_LEN

typedef struct _so_file {
    int fd;
    char *buffer;
    int currsor;
    int feof_flag;
    int ferror_flag;

} SO_FILE;

// SO_FILE *so_fopen(const char *pathname, const char *mode) {
//     SO_FILE *so_file;
//     int fd;

//     if (strcmp(mode, "r") == 0) {
//         fd = open(pathname, O_RDONLY, 0644);
//     } else if (strcmp(mode, "r+") == 0) {
//         fd = open(pathname, O_RDWR, 0644);
//     } else if (strcmp(mode, "w") == 0) {
//         fd = open(pathname, O_WRONLY | O_CREAT | O_TRUNC, 0644);
//     } else if (strcmp(mode, "w+") == 0) {
//         fd = open(pathname, O_RDWR | O_CREAT | O_TRUNC, 0644);
//     } else if (strcmp(mode, "a") == 0) {
//         fd = open(pathname, O_APPEND | O_WRONLY | O_CREAT, 0644);
//     } else if (strcmp(mode, "a+") == 0) {
//         fd = open(pathname, O_APPEND | O_RDONLY | O_CREAT, 0644);
//     }

//     if (fd < 0) {
//         exit(-1);
//     }

//     return so_file;
// }