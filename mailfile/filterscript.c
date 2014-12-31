#include <stdio.h>
#include <tcl.h>

#include "message.h"
#include "filterscript.h"
#include "logging.h"

static int
cmd_database(ClientData data, Tcl_Interp *interp,
        int argc, const char *argv[])
{
    struct filter_context *ctx = FILTER_CONTEXT(data);
    notmuch_status_t status;

    if (argc != 2) {
        Tcl_SetResult(interp, "wrong # of arguments: expected database path",
                NULL);
        return TCL_ERROR;
    }

    if (ctx->database) {
        notmuch_database_destroy(ctx->database);
    }

    log_info("opening database %s", argv[1]);
    
    status = notmuch_database_open(argv[1],
            ctx->dry_run ? 0 : NOTMUCH_DATABASE_MODE_READ_WRITE,
            &(ctx->database));
    if (status != NOTMUCH_STATUS_SUCCESS) {
        Tcl_SetResult(interp, "cannot open database", NULL);
        return TCL_ERROR;
    }

    return TCL_OK;
}

static int
iter_messages(filter_context_t *ctx, notmuch_messages_t *messages,
        Tcl_Interp *interp, const char *script)
{
    notmuch_message_t *message;
    int result;
    int n = 0;

    while (notmuch_messages_valid(messages)) {
        n += 1;
        message = notmuch_messages_get(messages);
        ctx->current_message = message;
        result = Tcl_Eval(interp, script);
        ctx->current_message = NULL;
        if (result != TCL_OK) {
            Tcl_AddErrorInfo(interp, "\n    for message ");
            Tcl_AddErrorInfo(interp, notmuch_message_get_message_id(message));
            return -1;
        }
        notmuch_messages_move_to_next(messages);
    }

    return n;
}

static int
cmd_matching(ClientData data, Tcl_Interp *interp,
        int argc, const char *argv[])
{
    char *qstr, *script;
    Tcl_Command *cmd;
    struct filter_context *ctx = FILTER_CONTEXT(data);
    notmuch_database_t *nm_db = ctx->database;
    notmuch_query_t *query;
    notmuch_messages_t *results;
    notmuch_message_t *message;
    int result = TCL_ERROR;

    if (!nm_db) {
        Tcl_SetResult(interp, "no database open", NULL);
        return TCL_ERROR;
    }

    if (argc != 3) {
        Tcl_SetResult(interp, "matching: invalid arguments", NULL);
        return TCL_ERROR;
    }

    qstr = argv[1];
    script = argv[2];
    query = notmuch_query_create(nm_db, qstr);
    if (query == NULL) {
        Tcl_SetResult(interp, "matching: could not create query", NULL);
        goto done;
    }

    results = notmuch_query_search_messages(query);
    if (results == NULL) {
        Tcl_SetResult(interp, "matching: could not get results", NULL);
        goto done;
    }

    int n = iter_messages(ctx, results, interp, script);
    if (n >= 0) {
        log_info("processed %d messages", n);
        result = TCL_OK;
    }
done:
    if (results) {
        notmuch_messages_destroy(results);
    }
    if (query) {
        notmuch_query_destroy(query);
    }

    return result;
}

void destroy_context(struct filter_context* ctx)
{
    if (ctx->database) {
        notmuch_database_destroy(ctx->database);
    }
    free(ctx);
}

filter_context_t* create_filter_context()
{
    struct filter_context *context = malloc(sizeof(struct filter_context));
    if (!context) {
        abort();
    }
    context->database = NULL;
    context->current_message = NULL;
    context->dry_run = false;
    return context;
}

Tcl_Interp* create_script_interpreter(filter_context_t *context)
{
    Tcl_Interp *interp = Tcl_CreateInterp();
    Tcl_CreateCommand(interp, "database", cmd_database, context, NULL);
    Tcl_CreateCommand(interp, "matching", cmd_matching, context, NULL);
    setup_message_commands(interp, context);
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
