#include <stdio.h>
#include <stdlib.h>

#include <tcl.h>
#include <notmuch.h>

#include <popt.h>

#include "logging.h"
#include "maildir.h"
#include "filterscript.h"

static void
set_arg_list(Tcl_Interp *interp, poptContext optCon)
{
    Tcl_Obj *obj = Tcl_NewListObj(0, NULL);
    const char *arg;

    while ((arg = poptGetArg(optCon)) != NULL) {
        Tcl_ListObjAppendElement(interp, obj, Tcl_NewStringObj(arg, -1));
    }
    Tcl_ObjSetVar2(interp, Tcl_NewStringObj("args", -1), NULL, obj, 0);
}

int main(int argc, const char *argv[])
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

    maildir_init();
    context = create_filter_context();

    optCon = poptGetContext(NULL, argc, argv, main_options,
                            POPT_CONTEXT_POSIXMEHARDER);
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
    filename = poptGetArg(optCon);
    if (!filename) {
        log_error("no file specified");
        status = 1;
        goto done;
    }
    set_arg_list(interp, optCon);

    log_info("running script %s", filename);
    int code = Tcl_EvalFile(interp, filename);
    if (code != TCL_OK) {
        print_script_error(interp, code);
        status = 1;
        goto done;
    }

done:
    Tcl_DeleteInterp(interp);
    destroy_context(context);
    return status;
}
