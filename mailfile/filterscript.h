#ifndef MF_COMMANDS_H
#define MF_COMMANDS_H

#include <tcl.h>
#include <notmuch.h>

typedef struct filter_context {
    notmuch_database_t *database;
    notmuch_message_t *current_message;
} filter_context_t;

#define FILTER_CONTEXT(x) ((struct filter_context*)(x))

filter_context_t* create_filter_context();
void destroy_context(filter_context_t *ctx);

Tcl_Interp* create_script_interpreter(filter_context_t *context);
void print_script_error(Tcl_Interp* interp, int code);

#endif
