#ifndef MF_MESSAGE_H
#define MF_MESSAGE_H

#include <tcl.h>
#include <notmuch.h>

void* activate_message_commands(Tcl_Interp *interp, notmuch_message_t *msg);
void deactivate_message_commands(Tcl_Interp *interp, void* cmds);

#endif
