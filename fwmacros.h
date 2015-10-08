/*
 * Copyright (C) Zbigniew Kempczyński wegorz76@gmail.com
 */
/* ---------------------------------------------------------------------------- */

/*! \file fwmacros.h
  \brief Common macros.
*/

#if !defined __FW_MACROS__H
#define __FW_MACROS__H

/* Includes */
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <pwd.h>
#include <math.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>

/* GLIB */
#include <glib.h>
#include <gmodule.h>
#include <pthread.h>
#include <locale.h>

/* ---------------------------------------------------------------------------- */
#define FW_MAX_BUF 8192

/* Detail level of messages, warnings and errors (0-no infos, 1-errors, 2-errors+warnings, 3=errors,warnings,messages
   4-3+messages-more-details, 5-4+messages-max-details */
#define FWDEBUG_LEVELMIN          1
#define FWDEBUG_LEVELMAX          5

/* External variables for debug purposes */
extern gint      FWDebugLevel, FWDebugFuncsLevel;

/* Debug filepos: 0-no, 1-function, 2-line-function, 3-function-line, 4-file-function, 5-file-line-function */
extern gint      FWDebugFilePosLevel; 
/* ---------------------------------------------------------------------------- */ 
#define fw_debug_on()             FWDebugLevel = FWDEBUG_LEVELMAX
#define fw_debug_off()            FWDebugLevel = FWDEBUG_LEVELMIN
#define fw_debug_set(LEV)         FWDebugLevel = LEV
#define fw_debug_funcs_on()       FWDebugFuncsLevel = FWDEBUG_LEVELMAX
#define fw_debug_funcs_off()      FWDebugFuncsLevel = FWDEBUG_LEVELMIN
#define fw_debug_funcs_set(LEV)   FWDebugFuncsLevel = LEV
#define fw_debug_filepos_on()     FWDebugFilePosLevel = FWDEBUG_LEVELMAX
#define fw_debug_filepos_off()    FWDebugFilePosLevel = FWDEBUG_LEVELMIN
#define fw_debug_filepos_set(LEV) FWDebugFilePosLevel = LEV

/* Include fileposition, line and function info when using fw_message*, fw_warning or fw_error macros */
#define FWFILEPOSDEBUG 1
//#undef FWFILEPOSDEBUG
#if defined (FWFILEPOSDEBUG) && FWFILEPOSDEBUG == 1
  #define fw_filepos_debug() G_STMT_START{ fw_filepos_print(__FILE__, __LINE__, (gchar *)__FUNCTION__); }G_STMT_END
#else
  #define fw_filepos_debug() {}
#endif

/* Messages, warnings and errors displaying - variable: FWDEBUG     */
/* Function and file position displaying    - variable: FWFUNCSDEBUG */

/* === Messaging displaying === */
#if defined (FWDEBUG) && FWDEBUG == 1
  #define fw_debug() if (MIN(FWDebugLevel, 1) >= 1) g_message("File:%s(line %d) fn:%s", __FILE__, __LINE__, __FUNCTION__)
  #define fw_message(format, args...)  G_STMT_START{ \
                                       if (MIN(FWDebugLevel, 3) >= 3) { fw_filepos_debug(); g_message(format, ##args); } \
                                       }G_STMT_END
  #define fw_message2(format, args...) G_STMT_START{ \
                                       if (MIN(FWDebugLevel, 4) >= 4) { fw_filepos_debug(); g_message(format, ##args); } \
                                       }G_STMT_END
  #define fw_message3(format, args...) G_STMT_START{ \
                                       if (MIN(FWDebugLevel, 5) >= 5) { fw_filepos_debug(); g_message(format, ##args); } \
                                       }G_STMT_END
  #define fw_warning(format, args...)  G_STMT_START{ \
                                       if (MIN(FWDebugLevel, 2) >= 2) { fw_filepos_debug(); g_warning(format, ##args); } \
                                       }G_STMT_END
  #define fw_error(format, args...)    G_STMT_START{ \
                                       if (MIN(FWDebugLevel, 1) >= 1) { fw_filepos_debug(); g_error(format, ##args); } \
                                       }G_STMT_END
#else 
  #undef FWDETDEBUG
  #define fw_message(txt, args...) 
  #define fw_message2(txt, args...) 
  #define fw_message3(txt, args...) 
  #define fw_warning(txt, args...) 
  #define fw_error(txt, args...) 
#endif

/* === Function and file position displaying === */
#if defined (FWFUNCSDEBUG) && FWFUNCSDEBUG == 1
  #define fw_func_debug() if (MIN(FWDebugFuncLevel, 1) >= 1) g_log(G_LOG_DOMAIN, G_LOG_LEVEL_INFO, "File:%s(line %d) fn:%s", __FILE__, __LINE__, __FUNCTION__)
  #define fw_func_message(format, args...)  if (MIN(FWDebugFuncsLevel, 3) >= 3) g_log(G_LOG_DOMAIN, G_LOG_LEVEL_INFO, format, ##args) 
  #define fw_func_message2(format, args...)  if (MIN(FWDebugFuncsLevel, 4)>= 4) g_log(G_LOG_DOMAIN, G_LOG_LEVEL_INFO, format, ##args) 
  #define fw_func_message3(format, args...)  if (MIN(FWDebugFuncsLevel, 5)>= 5) g_log(G_LOG_DOMAIN, G_LOG_LEVEL_INFO, format, ##args)
  #define fw_func_warning(format, args...)  if (MIN(FWDebugFuncsLevel, 2) >= 2) g_log(G_LOG_DOMAIN, G_LOG_LEVEL_INFO, format, ##args)
  #define fw_func_error(format, args...)  if (MIN(FWDebugFuncsLevel, 1) >= 1) g_error(format, ##args) 
#else 
  #define fw_func_message(txt, args...) 
  #define fw_func_message2(txt, args...) 
  #define fw_func_message3(txt, args...) 
  #define fw_func_warning(txt, args...) 
  #define fw_func_error(txt, args...) 
#endif

#define fw_errmsg_exit() G_STMT_START{ \
                         g_message("===== File:%s(line %d) fn:%s =====", __FILE__, __LINE__, __FUNCTION__); \
                         g_message("Error: '%s'", uu_errmsg_get()); exit(0); }G_STMT_END


#define FW_DEBUG_INFO()           fw_func_message("=== File:%s(%d) fn:%s ===", __FILE__, __LINE__, __FUNCTION__)
#define FW_DEBUG_INFO2()          fw_func_message2("=== File:%s(%d) fn:%s ===", __FILE__, __LINE__, __FUNCTION__)
#define FW_DEBUG_INFO3()          fw_func_message3("=== File:%s(%d) fn:%s ===", __FILE__, __LINE__, __FUNCTION__)
#define FW_DEBUG_INFOPARAM(info)  fw_func_message("=== File:%s(%d) fn:%s [%s] ===", __FILE__, __LINE__, __FUNCTION__, info)
#define FW_DEBUG_WARN()           fw_func_warning("=== File:%s(%d) fn:%s ===", __FILE__, __LINE__, __FUNCTION__)
#define FW_DEBUG_WARNPARAM(info)  fw_func_warning("=== File:%s(%d) fn:%s [%s] ===", __FILE__, __LINE__, __FUNCTION__, info)
#define FW_DEBUG_ERROR()          fw_func_error("=== File:%s(%d) fn:%s ===", __FILE__, __LINE__, __FUNCTION__)
#define FW_DEBUG_ERRORPARAM(info) fw_func_error("=== File:%s(%d) fn:%s [%s] ===", __FILE__, __LINE__, __FUNCTION__, info)


/* ---------------------------------------------------------------------------- */
/*! \brief Free a memory if \a ptr is \c NOT \c NULL and show debug info.
  \note Don't forget - twice using of this macro make a core dump. 
  More safe is using \a FW_FREEN() macro cause it will nullify \a ptr. */
#define FW_FREE(ptr) G_STMT_START{ \
                       fw_message3("=== File:%s(%d) fn:%s (FREEING) ===", __FILE__, __LINE__, __FUNCTION__); \
                       if (ptr) g_free(ptr); }G_STMT_END

/*! \brief Free a memory if \a ptr is \c NOT \c NULL, set \a ptr to \c NULL and show debug info. */
#define FW_FREEN(ptr) G_STMT_START{ \
                        fw_message3("=== File:%s(%d) fn:%s (FREEING and set to NULL) ===", __FILE__, __LINE__, __FUNCTION__); \
                        if (ptr) { g_free(ptr); ptr = NULL; } }G_STMT_END


/*! \brief Create and start a new \a GTimer. Macro argument is a visible \a GTimer variable. */
#define FW_GTIMER_START(gt) G_STMT_START{ gt = g_timer_new(); }G_STMT_END

/*! \brief Restart previosly created \a GTimer. Elapsed seconds are stored in a \a retsec variable. */
#define FW_GTIMER_RESTART(gt, retsec) G_STMT_START{ g_timer_stop(gt); \
                                                    retsec = g_timer_elapsed(gt, NULL); \
                                                    g_timer_reset(gt); \
                                                    g_timer_start(gt); }G_STMT_END

/*! \brief Stop, store elapsed time and destroy a \a GTimer. Elapsed seconds are stored in a \a retsec variable. */
#define FW_GTIMER_DESTROY(gt, retsec) G_STMT_START{ g_timer_stop(gt); \
                                                    retsec = g_timer_elapsed(gt, NULL); \
                                                    g_timer_destroy(gt); }G_STMT_END

/*! \brief Get an integer value if environment variable exists or return a default value. */
#define FW_ENVINT(envname, defaultint) (g_getenv(envname) ? atoi(g_getenv(envname)) : defaultint)
  
#endif
