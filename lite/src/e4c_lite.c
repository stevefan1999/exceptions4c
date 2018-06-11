/*
 * exceptions4c lightweight version 2.0
 *
 * Copyright (c) 2016 Guillermo Calvo
 * Modified 2018 Steve Fan
 * Licensed under the GNU Lesser General Public License
 */

#include <stdlib.h>
#include <stdio.h>
#include "e4c_lite.h"

#ifndef NDEBUG
#include "debugbreak.h"
#endif

E4C_DEFINE_EXCEPTION(Exception, Exception);

E4C_THREAD_LOCAL struct e4c_context e4c = { 0 };

static void e4c_propagate(void) {
    e4c.frame[e4c.frames].uncaught = 1;

    if (e4c.frames > 0) {
        longjmp(e4c.jump[e4c.frames - 1], 1);
    }

#ifndef NDEBUG
    debug_break();
#endif

    if (fprintf(stderr, e4c.err.file ? "\nError: %s\n" : "\nUncaught %s thrown at %s:%d\n", e4c.err.type->name, e4c.err.file, e4c.err.line) > 0) {
        (void)fflush(stderr);
    }
    exit(EXIT_FAILURE);
}

int e4c_try(const char * file, int line) {
    if (e4c.frames >= E4C_MAX_FRAMES) {
        e4c_throw(&Exception, file, line, "Too many `try` blocks nested.");
    }

    e4c.frames++;

    e4c.frame[e4c.frames].stage = e4c_beginning;
    e4c.frame[e4c.frames].uncaught = 0;

    return 1;
}

int e4c_hook(int is_catch) {

    int uncaught;

    if (is_catch) {
        e4c.frame[e4c.frames].uncaught = 0;
        return 1;
    }

    uncaught = e4c.frame[e4c.frames].uncaught;

    e4c.frame[e4c.frames].stage++;
    if (e4c.frame[e4c.frames].stage == e4c_catching && !uncaught) {
        e4c.frame[e4c.frames].stage++;
    }

    if (e4c.frame[e4c.frames].stage < e4c_done) {
        return 1;
    }

    e4c.frames--;

    if (uncaught) {
        e4c_propagate();
    }

    return 0;
}

int e4c_extends(const struct e4c_exception_type * child, const struct e4c_exception_type * parent) {

    for (; child && child->supertype != child; child = child->supertype) {
        if (child->supertype == parent) {
            return 1;
        }
    }

    return 0;
}

void e4c_throw(const struct e4c_exception_type * exception_type, const char * file, int line, void *user) {
    e4c.err.type = exception_type;
    e4c.err.file = file;
    e4c.err.line = line;
    e4c.err.user = user;

    e4c_propagate();
}

/* Retrieve current thrown exception */
struct e4c_exception *e4c_get_exception() {
    return &e4c.err;
}
