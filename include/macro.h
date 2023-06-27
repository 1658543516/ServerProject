//
// Created by mrpiao on 23-6-26.
//

#ifndef SERVERPROJECT_MACRO_H
#define SERVERPROJECT_MACRO_H

#include <string.h>
#include <assert.h>
#include "util.h"

#define SRVPRO_ASSERT1(x) \
    if (!(x)) { \
    	SRVPRO_LOG_ERROR(SRVPRO_LOG_ROOT()) << "ASSERTION: " #x << "\nbacktrace:\n" << srvpro::BacktraceToString(100, 2, "      "); \
    	assert(x); \
    }
    
#define SRVPRO_ASSERT2(x, w) \
    if (!(x)) { \
    	SRVPRO_LOG_ERROR(SRVPRO_LOG_ROOT()) << "ASSERTION: " #x << "\n" << w << "\nbacktrace:\n" << srvpro::BacktraceToString(100, 2, "      "); \
    	assert(x); \
    }

#endif //SERVERPROJECT_MACRO_H
