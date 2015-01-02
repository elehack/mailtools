#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/utsname.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "logging.h"
#include "maildir.h"

static const char *md_hostname = NULL;
static int md_counter = 0;

static char*
md_make_filename(struct stat *sbuf)
{
    if (md_hostname == NULL) {
        log_error("maildir subsystem not initialized");
        errno = EINVAL;
        return NULL;
    }

    struct timeval tv;
    if (gettimeofday(&tv, NULL)) {
        return NULL;
    }

    pid_t pid = getpid();
    char *buf = NULL;
    int size = 0;
    while (buf == NULL) {
        if (size > 0) {
            buf = malloc(size);
            if (!buf) return NULL;
        }
        int rc = snprintf(buf, size,
                "%ld.M%ldP%dV%ldI%ld.%s,S=%ld",
                tv.tv_sec, tv.tv_usec, pid,
                sbuf->st_dev, sbuf->st_ino,
                md_hostname,
                sbuf->st_size);
        if (rc < 0) {
            return NULL;
        } else if (rc >= size && buf) {
            free(buf);
            buf = NULL;
        }
        size = rc + 1;
    }
    return buf;
}

int maildir_init(void)
{
    struct utsname name;

    if (uname(&name)) {
        perror("uname");
        return -1;
    }

    md_hostname = name.nodename;
    log_debug("initialized maildir delivery for host %s", md_hostname);
    return 0;
}

int maildir_deliver_link(const char *src, const char *mdir)
{
    int status = -1;  // default to failure
    char *fn = NULL;
    char *tgt = NULL;

    log_debug("delivering file %s to maildir %s", src, mdir);

    struct stat buf;
    if (stat(src, &buf)) {
        log_error("maildir: %s: %s", src, strerror(errno));
        return -1;
    }

    fn = md_make_filename(&buf);
    if (!fn) {
        goto done;
    }
    log_debug("delivering mail with filename %s", fn);

    size_t md_len = strlen(mdir);
    size_t fn_len = strlen(fn);
    // dir + /new/ + fn + null
    size_t tgt_size = md_len + 5 + fn_len + 1;
    tgt = malloc(tgt_size);
    if (!tgt) {
        goto done;
    }
    memset(tgt, 0, tgt_size);

    memcpy(tgt, mdir, md_len);
    memcpy(tgt + md_len, "/new/", 5);
    memcpy(tgt + md_len + 5, fn, fn_len);

    log_debug("delivering mail to file %s", tgt);

    if (link(src, tgt)) {
        log_error("delivery to %s failed: %s", mdir, strerror(errno));
    } else {
        status = 0;
    }

done:
    if (fn) {
        free(fn);
    }
    if (tgt) {
        free(tgt);
    }
    return status;
}
