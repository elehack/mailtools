#include <glib.h>

#include "logging.h"

extern void register_maildir_tests(void);

int main(int argc, char *argv[])
{
    g_test_init(&argc, &argv, NULL);

    register_maildir_tests();

    if (g_test_verbose()) {
        log_level = LOG_DEBUG;
    }

    return g_test_run();
}
