#pragma once

/* Workaround for the fact that the xkb header uses the word 'explicit' as a variable name, that
 * doesn't work in c++ as its a keyword.
 */
#define explicit explicit_name

#include <xcb/xkb.h>

#undef explicit
