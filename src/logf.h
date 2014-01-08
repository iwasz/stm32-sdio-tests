/****************************************************************************
 *                                                                          *
 *  Author : lukasz.iwaszkiewicz@gmail.com                                  *
 *  ~~~~~~~~                                                                *
 *  License : see COPYING file for details.                                 *
 *  ~~~~~~~~~                                                               *
 ****************************************************************************/

#ifndef SYSLOG_H_
#define SYSLOG_H_

#include <stdio.h>

#ifndef NDEBUG
#define logf(...) printf (__VA_ARGS__);
#else
#define logf(...)
#endif

#endif /* SYSLOG_H_ */
