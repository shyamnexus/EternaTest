#ifndef __NVT_PERF_UTIL_H__
#define __NVT_PERF_UTIL_H__
#include <linux/time.h>

static inline unsigned int time_interval(ktime_t *time_result, ktime_t *time_start, ktime_t *time_end)
{
	if (unlikely(*time_end < *time_start))
		return -EINVAL;

	*time_result = ktime_sub(*time_end, *time_start);
	return 0;
}

#endif
