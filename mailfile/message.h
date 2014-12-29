#ifndef MF_MESSAGE_H
#define MF_MESSAGE_H

#include <tcl.h>
#include <notmuch.h>

typedef struct filter_context filter_context_t;

void setup_message_commands(Tcl_Interp *interp, filter_context_t *context);

#endif
