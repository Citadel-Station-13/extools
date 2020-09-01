#include "../core/core.h"
#include "logging.h"

extern "C" EXPORT const char* init_logging(int n_args, const char** args)
{
	if (!Core::initialize())
	{
		return "Extools Init Failed";
	}
	return enable_logging();
}