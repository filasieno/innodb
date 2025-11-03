#include "ib_condig.hpp"

#ifdef IB_COMPILE_TEST_FUNCS

#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>

/// \brief structure used for recording usage statistics
typedef struct speedo_struct {
    struct rusage	ru;	//!< getrusage() result
    struct timeval	tv;	//!< gettimeofday() result
} speedo_t;

/// \brief Resets a speedo (records the current time in it).
/// \param speedo out: speedo
IB_INTERN void speedo_reset(speedo_t* speedo);

/// \brief Shows the time elapsed and usage statistics since the last reset of a speedo.
/// \param speedo in: speedo
IB_INTERN void speedo_show(const speedo_t*	speedo);

#endif // IB_COMPILE_TEST_FUNCS

