// DEFINES NULL FOR C OR CPP

// Undefine NULL if exists
#ifdef NULL
#undef NULL
#endif

#ifdef __cplusplus
extern "C" {
#endif
/* standard NULL declaration */
#define NULL 0
#ifdef __cplusplus
}
#else
/* standard NULL declaration */
#define NULL (void *)0
#endif

/*
 * This is redundant it will be fixed later
 * WHEN C++
 * extern "C"{
 * #define NULL 0
 * }
 *
 * WHEN C
 * #define NULL 0
 * #define NULL (void *) 0
 *
 *
 *
 * */
