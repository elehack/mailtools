#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <glib.h>

#include "maildir.h"

struct md_fixture {
    char *maildir;
};

void md_setup(struct md_fixture *fix, void *data)
{
    const char *tmpdir = g_get_tmp_dir();
    char *template = g_strdup_printf("%s/md_test.XXXXXX", tmpdir);
    fix->maildir = mkdtemp(template);
    if (fix->maildir == NULL) {
        perror("mkdtemp");
        abort();
    }
    g_debug("creating temporary maildir %s", fix->maildir);
    char *subdir = g_strdup_printf("%s/cur", fix->maildir);
    if (mkdir(subdir, 0700)) {
        perror(subdir);
        abort();
    }
    g_free(subdir);
    subdir = g_strdup_printf("%s/new", fix->maildir);
    if (mkdir(subdir, 0700)) {
        perror(subdir);
        abort();
    }
    g_free(subdir);
    subdir = g_strdup_printf("%s/tmp", fix->maildir);
    if (mkdir(subdir, 0700)) {
        perror(subdir);
        abort();
    }
    g_free(subdir);

    if (maildir_init()) {
        perror("maildir_init");
        abort();
    }
}

void md_teardown(struct md_fixture *fix, void *data)
{
    g_debug("tearing down %s\n", fix->maildir);
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        abort();
    } else if (pid == 0) {
        execlp("rm", "rm", "-rf", fix->maildir, (char*) NULL);
    }
}

void md_test_deliver_link(struct md_fixture *fix, void *data)
{
    char *fn = g_strdup_printf("%s/foo.txt", fix->maildir);
    const char *content =
        "From: foo@example.com\n"
        "Subject: test message\n"
        "\n"
        "Hello!\n";
    if (!g_file_set_contents(fn, content, strlen(content), NULL)) {
        abort();
    }
    char *out;
    int rc = maildir_deliver_link(fn, fix->maildir, &out);
    g_free(fn);
    g_assert_cmpint(rc, ==, 0);
    g_assert_nonnull(out);

    char *new_dir = g_strdup_printf("%s/new", fix->maildir);
    GDir *dir = g_dir_open(new_dir, 0, NULL);
    int file_count = 0;
    const char *path;
    while ((path = g_dir_read_name(dir))) {
        g_debug("found delivered file %s", path);
        file_count += 1;
    }
    g_dir_close(dir);
    g_free(new_dir);

    g_assert_cmpint(file_count, ==, 1);
}

void register_maildir_tests()
{
    g_test_add("/maildir/deliver-link", struct md_fixture, NULL,
            md_setup,
            md_test_deliver_link,
            md_teardown);
}
