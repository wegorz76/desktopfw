/* 
 * Copyright (C) Zbigniew Kempczyński wegorz76@gmail.com
 */
/* ---------------------------------------------------------------------------- */
#if !defined __FWUTILS_H
#define __FWUTILS_H

#include <fwcommon.h>

/* ---------------------------------------------------------------------------- */

typedef     gpointer (*FWGArrayElemFunc) (GArray *array, gint i);

gint        fwutils_find_elem_in_sorted_garray(GArray *array, FWGArrayElemFunc elemfunc, GCompareFunc compfunc, gpointer elem);

#endif 
