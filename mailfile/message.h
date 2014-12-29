#ifndef MF_MESSAGE_H
#define MF_MESSAGE_H

#include <tcl.h>

int cmd_tag_message(ClientData data, Tcl_Interp *interp, int argc, const char *argv[]);
int cmd_msg(ClientData data, Tcl_Interp *interp, int argc, const char *argv[]);

#endif
