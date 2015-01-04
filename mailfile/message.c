#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <tcl.h>
#include <notmuch.h>
#include <glib.h>

#include "filterscript.h"
#include "logging.h"
#include "strutil.h"

typedef int (*msg_cmd_t)(Tcl_Interp*, notmuch_message_t*, int, const char**);

struct msg_command {
    const char *name;
    msg_cmd_t command;
};

static int
msg_id(Tcl_Interp *interp, notmuch_message_t *msg, int argc, const char *argv[]) {
    if (argc != 0) {
        tcl_result_printf(interp, "msg id takes no arguments");
        return TCL_ERROR;
    }

    char *msg_id = strdup(notmuch_message_get_message_id(msg));

    if (msg_id == NULL) {
        return TCL_ERROR;
    }

    tcl_result_printf(interp, "%s", msg_id);
    return TCL_OK;
}

static int
msg_header(Tcl_Interp *interp, notmuch_message_t *msg, int argc, const char *argv[]) {
    if (argc != 1) {
        tcl_result_printf(interp, "expected: msg header <header>");
        return TCL_ERROR;
    }

    const char *value = notmuch_message_get_header(msg, argv[0]);

    if (value == NULL) {
        return TCL_ERROR;
    }

    tcl_result_printf(interp, "%s", value);
    return TCL_OK;
}

static int
msg_date(Tcl_Interp *interp, notmuch_message_t *msg, int argc, const char *argv[]) {
    if (argc != 0) {
        tcl_result_printf(interp, "expected: msg date");
        return TCL_ERROR;
    }

    Tcl_SetObjResult(interp, Tcl_NewLongObj(notmuch_message_get_date(msg)));

    return TCL_OK;
}

static int
msg_filenames(Tcl_Interp *interp, notmuch_message_t *msg, int argc, const char *argv[]) {
    if (argc != 0) {
        tcl_result_printf(interp, "msg filenames takes no arguments");
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
    { "date", msg_date },
    { "filenames", msg_filenames },
    { NULL }
};

int
cmd_move_message(ClientData data, Tcl_Interp *interp, int argc, const char *argv[]) {
    filter_context_t *ctx = FILTER_CONTEXT(data);
    notmuch_message_t *msg = ctx->current_message;
    notmuch_status_t nmrc;

    if (argc != 2) {
        tcl_result_printf(interp, "wrong # of args: got %d, expected move folder", argc);
        return TCL_ERROR;
    }
    
    const char *folder = argv[1];
    if (folder[0] == '/') {
        tcl_result_printf(interp, "invalid folder '%s'", folder);
        return TCL_ERROR;
    }

    log_debug("moving message %s to folder %s",
              notmuch_message_get_message_id(msg), folder);
    char *folder_path = g_strdup_printf("%s/%s",
                                        notmuch_database_get_path(ctx->database),
                                        folder);
    int status = TCL_ERROR;

    notmuch_filenames_t *fns = notmuch_message_get_filenames(msg);
    int nmoved = 0;
    while (notmuch_filenames_valid(fns)) {
        const char *fn = notmuch_filenames_get(fns);
        char *new_fn = NULL;
        if (g_str_has_prefix(fn, folder_path)) {
            log_debug("file %s already in folder %s", fn, folder);
        } else if (ctx->dry_run) {
            log_debug("moving %s to folder %s", fn, folder);
        } else {
            log_debug("moving %s to folder %s", fn, folder);
            int rc = maildir_deliver_link(fn, folder_path, &new_fn);
            if (rc) {
                tcl_result_printf(interp, "delivery error: %s", strerror(errno));
                goto done;
            }
            log_debug("delivered as %s", new_fn);
            nmrc = notmuch_database_add_message(ctx->database, new_fn, NULL);
            switch (nmrc) {
                case NOTMUCH_STATUS_SUCCESS:
                    log_warning("%s: message was not in database", new_fn);
                case NOTMUCH_STATUS_DUPLICATE_MESSAGE_ID:
                    log_debug("added new file to database");
                    break;
                default:
                    tcl_result_printf(interp, "error adding %s to database: %s",
                                      new_fn,
                                      notmuch_status_to_string(nmrc));
                    goto done;
            }
            nmrc = notmuch_database_remove_message(ctx->database, fn);
            switch (nmrc) {
                case NOTMUCH_STATUS_SUCCESS:
                    log_warning("%s: file was only file for message", fn);
                case NOTMUCH_STATUS_DUPLICATE_MESSAGE_ID:
                    log_debug("removed file %s from database", fn);
                    break;
                default:
                    tcl_result_printf(interp, "error removing %s from database: %s",
                                      fn, notmuch_status_to_string(nmrc));
                    goto done;
            }
            if (unlink(fn)) {
                tcl_result_printf(interp, "error unlinking %s: %s", fn, strerror(errno));
                goto done;
            }
            nmoved += 1;
        }
        notmuch_filenames_move_to_next(fns);
    }
    notmuch_filenames_destroy(fns);
    fns = NULL;

    if (nmoved == 0) {
        Tcl_SetObjResult(interp, Tcl_NewIntObj(0));
        status = TCL_OK;
        goto done;
    }

    log_debug("syncing maildir flags");
    
    const char *mid = notmuch_message_get_message_id(msg);
    nmrc = notmuch_database_find_message(ctx->database, mid, &msg);
    if (nmrc != NOTMUCH_STATUS_SUCCESS) {
        tcl_result_printf(interp, "cannot re-find message %s", mid);
        goto done;
    }
    notmuch_message_destroy(ctx->current_message);
    ctx->current_message = msg;
    nmrc = notmuch_message_tags_to_maildir_flags(msg);
    if (nmrc != NOTMUCH_STATUS_SUCCESS) {
        tcl_result_printf(interp, "error syncing tags back to flags");
        goto done;
    }

    Tcl_SetObjResult(interp, Tcl_NewIntObj(nmoved));
    status = TCL_OK;

done:
    if (fns) {
        notmuch_filenames_destroy(fns);
    }
    g_free(folder_path);
    return status;
}

int
cmd_tag_message(ClientData data, Tcl_Interp *interp, int argc, const char *argv[]) {
    filter_context_t *ctx = FILTER_CONTEXT(data);
    notmuch_message_t *msg = ctx->current_message;
    log_debug("tagging message %s", notmuch_message_get_message_id(msg));

    if (!msg) {
        tcl_result_printf(interp, "no active message");
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
                tcl_result_printf(interp, "invalid tag spec %s", tspec);
                return TCL_ERROR;
                break;
        }
        if (status != NOTMUCH_STATUS_SUCCESS) {
            tcl_result_printf(interp, "failed to apply tag %s", tspec);
            return TCL_ERROR;
        }
    }
    notmuch_message_tags_to_maildir_flags(msg);

    return TCL_OK;
}

int cmd_msg(ClientData data, Tcl_Interp *interp, int argc, const char *argv[])
{
    const char *subcmd;
    struct msg_command *cmd_desc;
    filter_context_t *ctx = FILTER_CONTEXT(data);
    notmuch_message_t *msg = ctx->current_message;
    if (argc <= 1) {
        tcl_result_printf(interp, "msg: no subcommand");
        return TCL_ERROR;
    }
    if (!msg) {
        tcl_result_printf(interp, "no active message");
        return TCL_ERROR;
    }

    subcmd = argv[1];
    for (cmd_desc = msg_commands; cmd_desc->name; cmd_desc++) {
        if (!strcmp(subcmd, cmd_desc->name)) {
            return (*(cmd_desc->command))(interp, msg, argc-2, argv+2);
        }
    }

    tcl_result_printf(interp, "msg: invalid subcommand %s", subcmd);
    return TCL_ERROR;
}

void setup_message_commands(Tcl_Interp *interp, filter_context_t *context)
{
    Tcl_CreateCommand(interp, "msg", cmd_msg, context, NULL);
    Tcl_CreateCommand(interp, "tag", cmd_tag_message, context, NULL);
    Tcl_CreateCommand(interp, "move", cmd_move_message, context, NULL);
}
