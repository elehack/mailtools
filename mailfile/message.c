#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <tcl.h>
#include <notmuch.h>
#include <glib.h>

#include "filterscript.h"
#include "logging.h"

typedef int (*msg_cmd_t)(Tcl_Interp*, notmuch_message_t*, int, const char**);

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

static int
msg_header(Tcl_Interp *interp, notmuch_message_t *msg, int argc, char *argv[]) {
    if (argc != 1) {
        Tcl_SetResult(interp, "expected: msg header <header>", NULL);
        return TCL_ERROR;
    }

    const char *value = notmuch_message_get_header(msg, argv[0]);

    if (value == NULL) {
        return TCL_ERROR;
    }

    Tcl_SetResult(interp, g_strdup(value), g_free);
    return TCL_OK;
}

static int
msg_filenames(Tcl_Interp *interp, notmuch_message_t *msg, int argc, char *argv[]) {
    if (argc != 0) {
        Tcl_SetResult(interp, "msg filenames takes no arguments", NULL);
        return TCL_ERROR;
    }

    Tcl_Obj* list = Tcl_NewListObj(0, NULL);
    if (!list) {
        return TCL_ERROR;
    }
    notmuch_filenames_t *fns = notmuch_message_get_filenames(msg);
    while (notmuch_filenames_valid(fns)) {
        const char *fn = notmuch_filenames_get(fns);
        Tcl_ListObjAppendElement(interp, list, Tcl_NewStringObj(fn, -1));
        notmuch_filenames_move_to_next(fns);
    }
    notmuch_filenames_destroy(fns);

    Tcl_SetObjResult(interp, list);
    return TCL_OK;
}

static struct msg_command msg_commands[] = {
    { "id", msg_id },
    { "header", msg_header },
    { "filenames", msg_filenames },
    { NULL }
};

int
cmd_tag_message(ClientData data, Tcl_Interp *interp, int argc, const char *argv[]) {
    filter_context_t *ctx = FILTER_CONTEXT(data);
    notmuch_message_t *msg = ctx->current_message;
    log_debug("tagging message %s", notmuch_message_get_message_id(msg));

    if (!msg) {
        Tcl_SetResult(interp, "no active message", NULL);
        return TCL_ERROR;
    }

    for (int i = 1; i < argc; i++) {
        const char *tspec = argv[i];
        notmuch_status_t status = NOTMUCH_STATUS_SUCCESS;
        switch (*tspec) {
            case '+':
                log_debug("adding tag %s", tspec + 1);
                if (!ctx->dry_run) {
                    status = notmuch_message_add_tag(msg, tspec+1);
                }
                break;
            case '-':
                log_debug("removing tag %s", tspec + 1);
                if (!ctx->dry_run) {
                    status = notmuch_message_remove_tag(msg, tspec+1);
                }
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
    filter_context_t *ctx = FILTER_CONTEXT(data);
    notmuch_message_t *msg = ctx->current_message;
    if (argc <= 1) {
        Tcl_SetResult(interp, "msg: no subcommand", NULL);
        return TCL_ERROR;
    }
    if (!msg) {
        Tcl_SetResult(interp, "no active message", NULL);
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

void setup_message_commands(Tcl_Interp *interp, filter_context_t *context)
{
    Tcl_CreateCommand(interp, "msg", cmd_msg, context, NULL);
    Tcl_CreateCommand(interp, "tag", cmd_tag_message, context, NULL);
}
