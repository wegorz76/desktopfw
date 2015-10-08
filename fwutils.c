/* 
 * Copyright (C) Zbigniew Kempczyński wegorz76@gmail.com
 */
/* ---------------------------------------------------------------------------- */

#include <fwcommon.h>

/*! \file fwutils.c
  \brief FW utilities.
*/

/*! \file fwutils.c */

/* ---------------------------------------------------------------------------- */
/*! Find index for elem in sortet garray
 */
gint        fwutils_find_elem_in_sorted_garray(GArray *array, FWGArrayElemFunc elemfunc, GCompareFunc compfunc, gpointer elem)
{
  gint lo, hi, i, ret;
  gint len = array->len;
  gpointer p;

  lo = 0;
  hi = len-1;

  do {
    fw_message3("[lo = %d, hi = %d], i = %d, elem = %d\n", lo, hi, i, (gint) elem);
    i = (lo + hi) / 2;

    p = elemfunc(array, i);
    ret = compfunc(p, elem);

    if (!ret) {
      fw_message3("EQUAL: index = %d\n", i);
      return i;
    }

    if (ret > 0)   
      hi = i;
    else if (ret < 0)
      lo = i;

    fw_message3("RET: %d\n", ret);
  } while (ret && (hi - lo != 1));
  fw_message("[lo = %d, hi = %d], i = %d, elem = %d\n", lo, hi, i, (gint) elem);

  if (hi - lo == 1) {
    /* Test if 'hi' index is equal to elem */ 
    p = elemfunc(array, hi);
    if ( !(ret = compfunc(p, elem)) ) 
      return hi;

    /* Test if 'lo' index is equal to elem */ 
    p = elemfunc(array, lo);
    if ( !(ret = compfunc(p, elem)) ) 
      return lo;
  }

  return -1;
}
