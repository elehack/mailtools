#include <stdio.h>
#include <stdlib.h>

#include <tcl.h>
#include <notmuch.h>

#include "filterscript.h"

int main(int argc, char *argv[])
{
    Tcl_Interp *interp = NULL;
    filter_context_t *context = NULL;
    notmuch_database_t *nm_db = NULL;
    notmuch_status_t nm_stat;
    int i;
    int status = 0;

    context = create_filter_context();
    interp = create_script_interpreter(context);

    for (i = 1; i < argc; i++) {
        printf("running script %s\n", argv[i]);
        int code = Tcl_EvalFile(interp, argv[i]);
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
