#include <stdio.h>
#include <stdlib.h>

#include <tcl.h>
#include <notmuch.h>

#include <popt.h>

#include "logging.h"
#include "filterscript.h"

static int flag_debug = 0;
static int flag_quiet = 0;
static int flag_dry = 0;

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

    struct poptOption main_options[] = {
        { "debug", 'd', POPT_ARG_NONE, &flag_debug, "Print debugging messages", NULL },
        { "quiet", 'q', POPT_ARG_NONE, &flag_quiet, "Suppress non-error messages", NULL },
        { "dry-run", 'n', POPT_ARG_NONE, &flag_dry, "Don't make any changes", NULL },
        POPT_AUTOHELP
        { NULL, 0, 0, NULL, 0 }
    };

    optCon = poptGetContext(NULL, argc, argv, main_options, 0);
    while (poptGetNextOpt(optCon) > 0) {
        /* pass */
    }
    if (flag_debug) {
        log_level = LOG_DEBUG;
    } else if (flag_quiet) {
        log_level = LOG_QUIET;
    }

    context = create_filter_context();
    context->dry_run = flag_dry;
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
