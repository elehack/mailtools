#include <glib.h>

int main(int argc, char *argv[])
{
    g_test_init(&argc, &argv, NULL);

    register_maildir_tests();

    return g_test_run();
}
