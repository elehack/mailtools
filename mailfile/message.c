#include <stdio.h>
#include <stdlib.h>

#include <tcl.h>
#include <notmuch.h>
#include <glib.h>

typedef int (*msg_cmd_t)(Tcl_Interp*, notmuch_message_t*, int, char**);

struct msg_command {
    const char *name;
    msg_cmd_t command;
};

static int
msg_id(Tcl_Interp *interp, notmuch_message_t *msg, int argc, char *argv[]) {
    if (argc != 0) {
        Tcl_SetResult(interp, "msg id takes no arguments", NULL);
        return TCL_ERROR;
    }

    char *msg_id = strdup(notmuch_message_get_message_id(msg));

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

int
cmd_tag_message(ClientData data, Tcl_Interp *interp, int argc, char *argv[]) {
    notmuch_message_t *msg = (notmuch_message_t*) data;

    for (int i = 1; i < argc; i++) {
        const char *tspec = argv[i];
        notmuch_status_t status;
        switch (*tspec) {
            case '+':
                status = notmuch_message_add_tag(msg, tspec+1);
                break;
            case '-':
                status = notmuch_message_remove_tag(msg, tspec+1);
                break;
            default:
                Tcl_SetResult(interp,
                        g_strdup_printf("invalid tag spec %s", tspec),
                        g_free);
                return TCL_ERROR;
                break;
        }
        if (status != NOTMUCH_STATUS_SUCCESS) {
            Tcl_SetResult(interp, g_strdup_printf("failed to apply tag %s", tspec),
                    g_free);
            return TCL_ERROR;
        }
    }

    return TCL_OK;
}

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

void* activate_message_commands(Tcl_Interp *interp, notmuch_message_t *msg)
{
    Tcl_CreateCommand(interp, "msg", cmd_msg, msg, NULL);
    Tcl_CreateCommand(interp, "tag", cmd_tag_message, msg, NULL);
    return NULL;
}

void deactivate_message_commands(Tcl_Interp *interp, void* cmds)
{
    Tcl_DeleteCommand(interp, "msg");
    Tcl_DeleteCommand(interp, "tag");
}
