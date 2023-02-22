/*
 *
 */
#ifndef _UTILS_H
#define _UTILS_H

#include <limits.h>
#include <stdbool.h>

#ifndef HOST_NAME_MAX
#define HOST_NAME_MAX 64
#endif

#define PRETTY_LEN 1024

// Configure header
#include "include/autogen/config.h"

/*
 * Access the global/local rank/size from environment variables set by the launcher
 */
int get_rank_size_info(int *global_rank, int *global_size,
                       int *local_rank, int *local_size);

/*
 * Create a string representation of the binding
 * 0/2 on node18)  Process Bound  : socket 0[core  1[hwt 0-7]],socket 1[core 10[hwt 0-7]]
 */
int opal_hwloc_base_cset2str(char *str, int len,
                             hwloc_topology_t topo,
                             hwloc_cpuset_t cpuset,
                             bool is_full);

/*
 * Create a string representation of the binding using a bracketed notion
 * 0/2 on node18)  Process Bound  : [......../BBBBBBBB/......../......../......../......../......../......../......../........][BBBBBBBB/......../......../......../......../......../......../......../......../........]
 */
int opal_hwloc_base_cset2mapstr(char *str, int len,
                                hwloc_topology_t topo,
                                hwloc_cpuset_t cpuset);

#endif /* _UTILS_H */

