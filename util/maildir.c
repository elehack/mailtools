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
    md_counter++;
    while (buf == NULL) {
        if (size > 0) {
            buf = malloc(size);
            if (!buf) return NULL;
        }
        int rc = snprintf(buf, size,
                "%ld.M%ldP%dV%ldI%ld_%d.%s,S=%ld",
                tv.tv_sec, tv.tv_usec, pid,
                sbuf->st_dev, sbuf->st_ino,
                md_counter,
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

    md_hostname = strdup(name.nodename);
    if (!md_hostname) {
        return -1;
    }
    log_debug("initialized maildir delivery for host %s", md_hostname);
    return 0;
}

static int mkdir_if_nonexistent(const char *dir)
{
    struct stat buf;
    int rc = stat(dir, &buf);
    if (rc) {
        if (errno == ENOENT) {
            log_debug("creating directory %s", dir);
            // does not exist
            if (mkdir(dir, 0777)) {
                log_error("error creating %s: %s", dir, strerror(errno));
                return -1;
            }
        } else {
            log_error("error stat'ing %s: %s", dir, strerror(errno));
            return -1;
        }
    }
    return 0;
}

int maildir_ensure_exists(const char *mdir)
{
    int len, rc;
    char *tmp;

    if (mkdir_if_nonexistent(mdir)) {
        return -1;
    }

    len = strlen(mdir);
    tmp = malloc(len + 6);
    memset(tmp, 0, len + 6);
    memcpy(tmp, mdir, len);
    // ensure trailing slash
    if (tmp[len-1] != '/') {
        tmp[len] = '/';
        len += 1;
    }

    strcpy(tmp + len, "cur");
    rc = mkdir_if_nonexistent(tmp);
    if (rc) {
        goto done;
    }

    strcpy(tmp + len, "tmp");
    rc = mkdir_if_nonexistent(tmp);
    if (rc) {
        goto done;
    }

    strcpy(tmp + len, "new");
    rc = mkdir_if_nonexistent(tmp);

done:
    free(tmp);
    return rc;
}

int maildir_deliver_link(const char *src, const char *mdir, char **out_fn)
{
    int status = -1;  // default to failure
    char *fn = NULL;
    char *tgt = NULL;

    log_debug("delivering file %s to maildir %s", src, mdir);
    if (maildir_ensure_exists(mdir)) {
        return -1;
    }

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
        if (out_fn) {
            // transfer ownership to the client
            *out_fn = tgt;
            tgt = NULL;
        }
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
