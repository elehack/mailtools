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

int cmd_matching(ClientData data, Tcl_Interp *interp,
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

int cmd_tag(ClientData data ,Tcl_Interp *interp,
        int argc, const char *argv[])
{
    printf("received tag command %s\n", argc > 1 ? argv[1] : NULL);
    return TCL_OK;
}

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

    interp = Tcl_CreateInterp();
    Tcl_CreateCommand(interp, "tag", cmd_tag, nm_db,
            notmuch_database_close);
    Tcl_CreateCommand(interp, "matching", cmd_matching, nm_db,
            notmuch_database_close);

    for (i = 1; i < argc; i++) {
        printf("running script %s\n", argv[i]);
        int code = Tcl_EvalFile(interp, argv[i]);
        if (code != TCL_OK) {
            Tcl_Obj *opts = Tcl_GetReturnOptions(interp, code);
            Tcl_Obj *key = Tcl_NewStringObj("-errorinfo", -1);
            Tcl_Obj *info;
            Tcl_IncrRefCount(key);
            Tcl_DictObjGet(NULL, opts, key, &info);
            Tcl_DecrRefCount(key);
            fprintf(stderr, "error: %s\n", Tcl_GetString(info));
            Tcl_DecrRefCount(opts);
            status = 1;
            goto done;
        }
    }

done:
    Tcl_DeleteInterp(interp);
    return status;
}
