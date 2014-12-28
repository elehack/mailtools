#include <stdio.h>
#include <stdlib.h>

#include <tcl.h>
#include <notmuch.h>

#include "filterscript.h"

int main(int argc, char *argv[])
{
    Tcl_Interp *interp = NULL;
    notmuch_database_t *nm_db = NULL;
    notmuch_status_t nm_stat;
    int i;
    int status = 0;

    nm_stat = notmuch_database_open("/home/michael/Mail", 0, &nm_db);
    if (nm_stat != NOTMUCH_STATUS_SUCCESS) {
        return 2;
    }

    interp = create_script_interpreter(nm_db);

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
    return status;
}
