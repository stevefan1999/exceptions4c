/*
 * exceptions4c lightweight version 2.1
 *
 * Copyright (c) 2016 Guillermo Calvo
 * Modified 2018 Steve Fan
 * Licensed under the GNU Lesser General Public License
 */

#ifndef EXCEPTIONS4C_LITE
#define EXCEPTIONS4C_LITE

#include <stddef.h>
#include <setjmp.h>

 /* Maximum number of nested `try` blocks */
#ifndef E4C_MAX_FRAMES
# define E4C_MAX_FRAMES 128
#endif

/* Controls whether file/line info is attached to exceptions */
#ifndef NDEBUG
# define E4C_DEBUG_INFO __FILE__, __LINE__
#else
# define E4C_DEBUG_INFO NULL, 0
#endif

/* Represents an exception type */
struct e4c_exception_type {
    const char * name;
    const struct e4c_exception_type * supertype;
};

/* Declarations and definitions of exception types */
#define E4C_DECLARE_EXCEPTION(name) extern const struct e4c_exception_type name
#define E4C_DEFINE_EXCEPTION(name, supertype) const struct e4c_exception_type name = { #name, &supertype }

/* Predefined exception types */
E4C_DECLARE_EXCEPTION(Exception);

/* Represents an instance of an exception type */
typedef struct e4c_exception {
    const struct e4c_exception_type * type;
    void *user;
    const char *file;
    int line;
} e4c_exception;

/* Returns whether current exception is of a given type */
#define E4C_EXCEPTION struct e4c_exception
#define E4C_GET_EXCEPTION e4c_get_exception
#define E4C_IS_INSTANCE_OF(t) ( E4C_GET_EXCEPTION()->type == &t || e4c_extends(E4C_GET_EXCEPTION()->type, &t) )

/* Implementation details */
#define E4C_TRY if(e4c_try(E4C_DEBUG_INFO) && setjmp(e4c.jump[e4c.frames - 1]) >= 0) while(e4c_hook(0)) if(e4c.frame[e4c.frames].stage == e4c_trying)
#define E4C_CATCH(type) else if(e4c.frame[e4c.frames].stage == e4c_catching && E4C_IS_INSTANCE_OF(type) && e4c_hook(1))
#define E4C_FINALLY else if(e4c.frame[e4c.frames].stage == e4c_finalizing)
#define E4C_THROW(type, user) e4c_throw(&type, E4C_DEBUG_INFO, user)
#define E4C_RETHROW e4c_throw(E4C_GET_EXCEPTION()->type, E4C_DEBUG_INFO, E4C_GET_EXCEPTION()->user)

#if !defined(E4C_THREAD_LOCAL) && !defined(E4C_DISABLE_TLS)
# if __STDC_VERSION__ >= 201112 && !defined __STDC_NO_THREADS__
#  define E4C_THREAD_LOCAL _Thread_local
#  define E4C_THREAD_LOCAL_PROLOGUE
#  define E4C_THREAD_LOCAL_EPILOGUE
/* note that ICC (linux) and Clang are covered by __GNUC__ */
# elif defined __GNUC__ || \
       defined __SUNPRO_C || \
       defined __xlC__
#  define E4C_THREAD_LOCAL __thread
#  define E4C_THREAD_LOCAL_PROLOGUE
#  define E4C_THREAD_LOCAL_EPILOGUE
# elif defined _WIN32 && ( \
       defined _MSC_VER || \
       defined __ICL || \
       defined __DMC__ || \
       defined __BORLANDC__ )
#  define E4C_THREAD_LOCAL __declspec(thread) 
#  define E4C_THREAD_LOCAL_PROLOGUE E4C_THREAD_LOCAL
#  define E4C_THREAD_LOCAL_EPILOGUE 
# else
#  define E4C_THREAD_LOCAL 
#  define E4C_THREAD_LOCAL_PROLOGUE
#  define E4C_THREAD_LOCAL_EPILOGUE 
# endif
#endif

/* This functions must be called only via E4C_TRY, E4C_CATCH, E4C_FINALLY and E4C_THROW */
enum e4c_stage { e4c_beginning, e4c_trying, e4c_catching, e4c_finalizing, e4c_done };
extern E4C_THREAD_LOCAL_PROLOGUE struct e4c_context {
    jmp_buf jump[E4C_MAX_FRAMES]; 
    struct e4c_exception err;
    int frames;
    struct {
        unsigned char stage : e4c_done + 1;
        unsigned char uncaught : 1;
    } frame[E4C_MAX_FRAMES + 1];
} e4c E4C_THREAD_LOCAL_EPILOGUE;
int e4c_try(const char * file, int line);
int e4c_hook(int is_catch);
int e4c_extends(const struct e4c_exception_type * child, const struct e4c_exception_type * parent);
void e4c_throw(const struct e4c_exception_type * exception_type, const char * file, int line, void *user);
struct e4c_exception *e4c_get_exception();

#ifndef E4C_NO_KEYWORDS
# define try E4C_TRY
# define catch E4C_CATCH
# define finally E4C_FINALLY
# define throw E4C_THROW
# define rethrow E4C_RETHROW
# define get_exception E4C_GET_EXCEPTION
# define exception_extends E4C_IS_INSTANCE_OF
# define exception E4C_EXCEPTION
#endif

/* OpenMP support */
#ifdef _OPENMP
# pragma omp threadprivate(e4c)
#endif

#endif
