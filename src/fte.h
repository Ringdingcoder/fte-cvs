/*    fte.h
 *
 *    Copyright (c) 1994-1996, Marko Macek
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

#ifndef FTE_H
#define FTE_H

#include "feature.h"
#include "stl_fte.h"

#if defined(_DEBUG) && defined(MSVC) && defined(MSVCDEBUG)
#include <crtdbg.h>

#define new new( _CLIENT_BLOCK, __FILE__, __LINE__)

#endif //_DEBUG && MSVC && MSVCDEBUG

#define FTE_ARRAY_SIZE(a) (sizeof (a) / sizeof ((a)[0]))

#endif // FTE_H
