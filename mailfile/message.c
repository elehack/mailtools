#include <stdio.h>
#include <stdlib.h>

#include <tcl.h>
#include <notmuch.h>

int msg_id(Tcl_Interp *interp, notmuch_message_t *msg) {
    char *msg_id = strdup(notmuch_message_get_message_id(msg));
    if (msg_id == NULL) {
        return TCL_ERROR;
    }

    Tcl_SetResult(interp, msg_id, free);
    return TCL_OK;
}

int cmd_msg(ClientData data, Tcl_Interp *interp, int argc, const char *argv[])
{
    const char *subcmd;
    notmuch_message_t *msg = (notmuch_message_t*) data;
    if (argc <= 1) {
        Tcl_SetResult(interp, "msg: no subcommand", NULL);
        return TCL_ERROR;
    }

    subcmd = argv[1];
    if (!strcmp(subcmd, "id")) {
        if (argc != 2) {
            Tcl_SetResult(interp, "msg id takes no arguments", NULL);
            return TCL_ERROR;
        }
        return msg_id(interp, msg);
    } else {
        Tcl_Obj* obj = Tcl_NewObj();
        Tcl_AppendStringsToObj(obj, "msg: invalid subcommand ", subcmd, NULL);
        Tcl_SetObjResult(interp, obj);
        return TCL_ERROR;
    }
}

