#include <stdio.h>
#include <tcl.h>

#include "message.h"
#include "filterscript.h"

static int
cmd_matching(ClientData data, Tcl_Interp *interp,
        int argc, const char *argv[])
{
    char *qstr, *script;
    Tcl_Command *cmd;
    notmuch_database_t *nm_db = (notmuch_database_t*) data;
    notmuch_query_t *query;
    notmuch_messages_t *results;
    notmuch_message_t *message;
    int result = TCL_ERROR;

    if (argc != 3) {
        fprintf(stderr, "matching: invalid arguments\n");
        return TCL_ERROR;
    }

    qstr = argv[1];
    script = argv[2];
    query = notmuch_query_create(nm_db, qstr);
    if (query == NULL) {
        fprintf(stderr, "matching: could not create query\n");
        goto done;
    }

    results = notmuch_query_search_messages(query);
    if (results == NULL) {
        fprintf(stderr, "matching: could not get results\n");
        goto done;
    }

    while (notmuch_messages_valid(results)) {
        message = notmuch_messages_get(results);
        printf("found message %s\n", notmuch_message_get_message_id(message));
        Tcl_CreateCommand(interp, "msg", cmd_msg, message, NULL);
        result = Tcl_Eval(interp, script);
        if (result != TCL_OK) {
            Tcl_AddErrorInfo(interp, "\n    for message ");
            Tcl_AddErrorInfo(interp, notmuch_message_get_message_id(message));
            goto done;
        }
        Tcl_DeleteCommand(interp, "msg");
        notmuch_messages_move_to_next(results);
    }

    result = TCL_OK;
done:
    if (results) {
        notmuch_messages_destroy(results);
    }
    if (query) {
        notmuch_query_destroy(query);
    }

    return result;
}

Tcl_Interp* create_script_interpreter(notmuch_database_t *db)
{
    Tcl_Interp *interp = Tcl_CreateInterp();
    Tcl_CreateCommand(interp, "matching", cmd_matching, db, notmuch_database_close);
    return interp;
}

void print_script_error(Tcl_Interp* interp, int code)
{
    Tcl_Obj *opts = Tcl_GetReturnOptions(interp, code);
    Tcl_Obj *key = Tcl_NewStringObj("-errorinfo", -1);
    Tcl_Obj *info;
    Tcl_IncrRefCount(key);
    Tcl_DictObjGet(NULL, opts, key, &info);
    Tcl_DecrRefCount(key);
    fprintf(stderr, "error: %s\n", Tcl_GetString(info));
    Tcl_DecrRefCount(opts);
}
