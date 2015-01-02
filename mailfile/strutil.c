#include <stdlib.h>
#include <string.h>

#include <glib.h>

#include "strutil.h"

void tcl_result_printf(Tcl_Interp *interp, const char *fmt, ...)
{
    va_list ap;
    char *message;
    va_start(ap, fmt);
    message = g_strdup_vprintf(fmt, ap);
    va_end(ap);
    Tcl_Obj *obj = Tcl_NewStringObj(message, strlen(message));
    g_free(message);
    Tcl_SetObjResult(interp, obj);
}
