#include <stdio.h>
#include <stdlib.h>

#include <tcl.h>
#include <notmuch.h>

typedef int (*msg_cmd_t)(Tcl_Interp*, notmuch_message_t*, int, char**);

struct msg_command {
    const char *name;
    msg_cmd_t command;
};

static int
msg_id(Tcl_Interp *interp, notmuch_message_t *msg, int argc, char *argv[]) {
    char *msg_id = strdup(notmuch_message_get_message_id(msg));

    if (argc != 0) {
        Tcl_SetResult(interp, "msg id takes no arguments", NULL);
        return TCL_ERROR;
    }

    if (msg_id == NULL) {
        return TCL_ERROR;
    }

    Tcl_SetResult(interp, msg_id, free);
    return TCL_OK;
}

static struct msg_command msg_commands[] = {
    { "id", msg_id },
    { NULL }
};

int cmd_msg(ClientData data, Tcl_Interp *interp, int argc, const char *argv[])
{
    const char *subcmd;
    struct msg_command *cmd_desc;
    notmuch_message_t *msg = (notmuch_message_t*) data;
    if (argc <= 1) {
        Tcl_SetResult(interp, "msg: no subcommand", NULL);
        return TCL_ERROR;
    }

    subcmd = argv[1];
    for (cmd_desc = msg_commands; cmd_desc->name; cmd_desc++) {
        if (!strcmp(subcmd, cmd_desc->name)) {
            return (*(cmd_desc->command))(interp, msg, argc-2, argv+2);
        }
    }

    Tcl_Obj* obj = Tcl_NewObj();
    Tcl_AppendStringsToObj(obj, "msg: invalid subcommand ", subcmd, NULL);
    Tcl_SetObjResult(interp, obj);
    return TCL_ERROR;
}

