/*
   +----------------------------------------------------------------------+
   | Zend Engine                                                          |
   +----------------------------------------------------------------------+
   | Copyright (c) 1998-2017 Zend Technologies Ltd. (http://www.zend.com) |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.00 of the Zend license,     |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.zend.com/license/2_00.txt.                                |
   | If you did not receive a copy of the Zend license and are unable to  |
   | obtain it through the world-wide-web, please send a note to          |
   | license@zend.com so we can mail you a copy immediately.              |
   +----------------------------------------------------------------------+
   | Authors: Andi Gutmans <andi@zend.com>                                |
   |          Zeev Suraski <zeev@zend.com>                                |
   +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifndef ZEND_GLOBALS_MACROS_H
#define ZEND_GLOBALS_MACROS_H

typedef struct _zend_compiler_globals zend_compiler_globals;
typedef struct _zend_executor_globals zend_executor_globals;
typedef struct _zend_php_scanner_globals zend_php_scanner_globals;
typedef struct _zend_ini_scanner_globals zend_ini_scanner_globals;

BEGIN_EXTERN_C()

/* Compiler */
# define CG(v) (compiler_globals.v)
extern ZEND_API struct _zend_compiler_globals compiler_globals;
ZEND_API int zendparse(void);


/* Executor */
# define EG(v) (executor_globals.v)
extern ZEND_API zend_executor_globals executor_globals;

/* Language Scanner */
# define LANG_SCNG(v) (language_scanner_globals.v)
extern ZEND_API zend_php_scanner_globals language_scanner_globals;


/* INI Scanner */
# define INI_SCNG(v) (ini_scanner_globals.v)
extern ZEND_API zend_ini_scanner_globals ini_scanner_globals;

END_EXTERN_C()

#endif /* ZEND_GLOBALS_MACROS_H */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * indent-tabs-mode: t
 * End:
 */
