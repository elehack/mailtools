#ifndef MF_COMMANDS_H
#define MF_COMMANDS_H

#include <tcl.h>
#include <notmuch.h>

Tcl_Interp* create_script_interpreter(notmuch_database_t *db);
void print_script_error(Tcl_Interp* interp, int code);

#endif
