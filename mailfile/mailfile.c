#include <stdio.h>
#include <stdlib.h>

#include <tcl.h>
#include <notmuch.h>

#include <popt.h>

#include "logging.h"
#include "filterscript.h"

int main(int argc, char *argv[])
{
    Tcl_Interp *interp = NULL;
    filter_context_t *context = NULL;
    notmuch_database_t *nm_db = NULL;
    notmuch_status_t nm_stat;
    int i;
    int status = 0;
    poptContext optCon;
    const char *filename;
    int opt;

    struct poptOption main_options[] = {
        { "debug", 'd', POPT_ARG_NONE, NULL, 'd', "Print debugging messages", NULL },
        { "quiet", 'q', POPT_ARG_NONE, NULL, 'q', "Suppress non-error messages", NULL },
        { "dry-run", 'n', POPT_ARG_NONE, NULL, 'n', "Don't make any changes", NULL },
        POPT_AUTOHELP
        { NULL, 0, 0, NULL, 0 }
    };

    context = create_filter_context();

    optCon = poptGetContext(NULL, argc, argv, main_options, 0);
    while ((opt = poptGetNextOpt(optCon)) > 0) {
        switch (opt) {
            case 'd':
                log_level = LOG_DEBUG;
                break;
            case 'q':
                log_level = LOG_QUIET;
                break;
            case 'n':
                context->dry_run = true;
                break;
            default:
                log_error("unexpected argument %c", opt);
                abort();
        }
    }

    interp = create_script_interpreter(context);

    while ((filename = poptGetArg(optCon)) != NULL) {
        log_info("running script %s", filename);
        int code = Tcl_EvalFile(interp, filename);
        if (code != TCL_OK) {
            print_script_error(interp, code);
            status = 1;
            goto done;
        }
    }

done:
    Tcl_DeleteInterp(interp);
    destroy_context(context);
    return status;
}
