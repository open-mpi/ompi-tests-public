/*
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <hwloc.h>

#include "include/utils.h"

static int build_map(int *num_sockets_arg, int *num_cores_arg,
                     hwloc_cpuset_t cpuset, int ***map, hwloc_topology_t topo);
static char *bitmap2rangestr(int bitmap);


int get_rank_size_info(int *global_rank, int *global_size,
                       int *local_rank, int *local_size)
{
    char *envar = NULL;

    *global_rank = 0;
    *global_size = 0;
    *local_rank  = 0;
    *local_size  = 0;

    // JSM
    if( NULL != getenv("JSM_NAMESPACE_SIZE") ) {
        envar = getenv("JSM_NAMESPACE_RANK");
        *global_rank = atoi(envar);
        envar = getenv("JSM_NAMESPACE_SIZE");
        *global_size = atoi(envar);

        envar = getenv("JSM_NAMESPACE_LOCAL_RANK");
        *local_rank = atoi(envar);
        envar = getenv("JSM_NAMESPACE_LOCAL_SIZE");
        *local_size = atoi(envar);
    }
    // ORTE/PRRTE
    else if( NULL != getenv("OMPI_COMM_WORLD_SIZE") ) {
        envar = getenv("OMPI_COMM_WORLD_RANK");
        *global_rank = atoi(envar);
        envar = getenv("OMPI_COMM_WORLD_SIZE");
        *global_size = atoi(envar);

        envar = getenv("OMPI_COMM_WORLD_LOCAL_RANK");
        *local_rank = atoi(envar);
        envar = getenv("OMPI_COMM_WORLD_LOCAL_SIZE");
        *local_size = atoi(envar);
    }
    // MVAPICH2
    else if( NULL != getenv("MV2_COMM_WORLD_SIZE") ) {
        envar = getenv("MV2_COMM_WORLD_RANK");
        *global_rank = atoi(envar);
        envar = getenv("MV2_COMM_WORLD_SIZE");
        *global_size = atoi(envar);

        envar = getenv("MV2_COMM_WORLD_LOCAL_RANK");
        *local_rank = atoi(envar);
        envar = getenv("MV2_COMM_WORLD_LOCAL_SIZE");
        *local_size = atoi(envar);
    }

    return 0;
}

int opal_hwloc_base_cset2str(char *str, int len,
                             hwloc_topology_t topo,
                             hwloc_cpuset_t cpuset,
                             bool is_full)
{
    bool first;
    int num_sockets, num_cores;
    int ret, socket_index, core_index;
    char tmp[BUFSIZ];
    const int stmp = sizeof(tmp) - 1;
    int **map=NULL;
    //hwloc_obj_t root;
    //opal_hwloc_topo_data_t *sum;

    str[0] = tmp[stmp] = '\0';

    /* if the cpuset is all zero, then not bound */
    if (hwloc_bitmap_iszero(cpuset)) {
        return -1;
    }

    if (0 != (ret = build_map(&num_sockets, &num_cores, cpuset, &map, topo))) {
        return ret;
    }
    /* Iterate over the data matrix and build up the string */
    first = true;
    for (socket_index = 0; socket_index < num_sockets; ++socket_index) {
        for (core_index = 0; core_index < num_cores; ++core_index) {
            if (map[socket_index][core_index] > 0) {
                if (!first) {
                    if( is_full ) {
                        //strncat(str, ",\n", len - strlen(str));
                        strncat(str, ",", len - strlen(str));
                    } else {
                        strncat(str, ",", len - strlen(str));
                    }
                }
                first = false;

                if( is_full ) {
                    snprintf(tmp, stmp, "socket %d[core %2d[hwt %s]]",
                             socket_index, core_index,
                             bitmap2rangestr(map[socket_index][core_index]));
                } else {
                    snprintf(tmp, stmp, "%2d", core_index);
                }
                strncat(str, tmp, len - strlen(str));
            }
        }
    }
    if (NULL != map) {
        if (NULL != map[0]) {
            free(map[0]);
        }
        free(map);
    }

    return 0;
}

int opal_hwloc_base_cset2mapstr(char *str, int len,
                                hwloc_topology_t topo,
                                hwloc_cpuset_t cpuset)
{
    char tmp[BUFSIZ];
    int core_index, pu_index;
    const int stmp = sizeof(tmp) - 1;
    hwloc_obj_t socket, core, pu;
    //hwloc_obj_t root;
    //opal_hwloc_topo_data_t *sum;

    str[0] = tmp[stmp] = '\0';

    /* if the cpuset is all zero, then not bound */
    if (hwloc_bitmap_iszero(cpuset)) {
        return -1; //OPAL_ERR_NOT_BOUND;
    }

    /* Iterate over all existing sockets */
    for (socket = hwloc_get_obj_by_type(topo, HWLOC_OBJ_SOCKET, 0);
         NULL != socket;
         socket = socket->next_cousin) {
        strncat(str, "[", len - strlen(str));

        /* Iterate over all existing cores in this socket */
        core_index = 0;
        for (core = hwloc_get_obj_inside_cpuset_by_type(topo,
                                                        socket->cpuset,
                                                        HWLOC_OBJ_CORE, core_index);
             NULL != core;
             core = hwloc_get_obj_inside_cpuset_by_type(topo,
                                                        socket->cpuset,
                                                        HWLOC_OBJ_CORE, ++core_index)) {
            if (core_index > 0) {
                strncat(str, "/", len - strlen(str));
            }

            /* Iterate over all existing PUs in this core */
            pu_index = 0;
            for (pu = hwloc_get_obj_inside_cpuset_by_type(topo,
                                                          core->cpuset,
                                                          HWLOC_OBJ_PU, pu_index);
                 NULL != pu;
                 pu = hwloc_get_obj_inside_cpuset_by_type(topo,
                                                          core->cpuset,
                                                          HWLOC_OBJ_PU, ++pu_index)) {

                /* Is this PU in the cpuset? */
                if (hwloc_bitmap_isset(cpuset, pu->os_index)) {
                    strncat(str, "B", len - strlen(str));
                } else {
                    strncat(str, ".", len - strlen(str));
                }
            }
        }
        strncat(str, "]", len - strlen(str));
    }
    
    return 0;
}

/*
 * Make a map of socket/core/hwthread tuples
 */
static int build_map(int *num_sockets_arg, int *num_cores_arg,
                     hwloc_cpuset_t cpuset, int ***map, hwloc_topology_t topo)
{
    int num_sockets, num_cores;
    int socket_index, core_index, pu_index;
    hwloc_obj_t socket, core, pu;
    int **data;

    /* Find out how many sockets we have */
    num_sockets = hwloc_get_nbobjs_by_type(topo, HWLOC_OBJ_SOCKET);
    /* some systems (like the iMac) only have one
     * socket and so don't report a socket
     */
    if (0 == num_sockets) {
        num_sockets = 1;
    }
    /* Lazy: take the total number of cores that we have in the
       topology; that'll be more than the max number of cores
       under any given socket */
    num_cores = hwloc_get_nbobjs_by_type(topo, HWLOC_OBJ_CORE);
    *num_sockets_arg = num_sockets;
    *num_cores_arg = num_cores;

    /* Alloc a 2D array: sockets x cores. */
    data = malloc(num_sockets * sizeof(int *));
    if (NULL == data) {
        return -1; //OPAL_ERR_OUT_OF_RESOURCE;
    }
    data[0] = calloc(num_sockets * num_cores, sizeof(int));
    if (NULL == data[0]) {
        free(data);
        return -1; //OPAL_ERR_OUT_OF_RESOURCE;
    }
        for (socket_index = 1; socket_index < num_sockets; ++socket_index) {
        data[socket_index] = data[socket_index - 1] + num_cores;
    }

    /* Iterate the PUs in this cpuset; fill in the data[][] array with
       the socket/core/pu triples */
    for (pu_index = 0,
             pu = hwloc_get_obj_inside_cpuset_by_type(topo,
                                                      cpuset, HWLOC_OBJ_PU,
                                                      pu_index);
         NULL != pu;
         pu = hwloc_get_obj_inside_cpuset_by_type(topo,
                                                  cpuset, HWLOC_OBJ_PU,
                                                  ++pu_index)) {
        /* Go upward and find the core this PU belongs to */
        core = pu;
        while (NULL != core && core->type != HWLOC_OBJ_CORE) {
            core = core->parent;
        }
        core_index = 0;
        if (NULL != core) {
            core_index = core->logical_index;
        }

        /* Go upward and find the socket this PU belongs to */
        socket = pu;
        while (NULL != socket && socket->type != HWLOC_OBJ_SOCKET) {
            socket = socket->parent;
        }
        socket_index = 0;
        if (NULL != socket) {
            socket_index = socket->logical_index;
        }

        /* Save this socket/core/pu combo.  LAZY: Assuming that we
           won't have more PU's per core than (sizeof(int)*8). */
        data[socket_index][core_index] |= (1 << pu->sibling_rank);
    }

    *map = data;
    return 0;
}

/*
 * Turn an int bitmap to a "a-b,c" range kind of string
 */
static char *bitmap2rangestr(int bitmap)
{
    size_t i;
    int range_start, range_end;
    bool first, isset;
    char tmp[BUFSIZ];
    const int stmp = sizeof(tmp) - 1;
    static char ret[BUFSIZ];

    memset(ret, 0, sizeof(ret));

    first = true;
    range_start = -999;
    for (i = 0; i < sizeof(int) * 8; ++i) {
        isset = (bitmap & (1 << i));

        /* Do we have a running range? */
        if (range_start >= 0) {
            if (isset) {
                continue;
            } else {
                /* A range just ended; output it */
                if (!first) {
                    strncat(ret, ",", sizeof(ret) - strlen(ret) - 1);
                } else {
                    first = false;
                }

                range_end = i - 1;
                if (range_start == range_end) {
                    snprintf(tmp, stmp, "%d", range_start);
                } else {
                    snprintf(tmp, stmp, "%d-%d", range_start, range_end);
                }
                strncat(ret, tmp, sizeof(ret) - strlen(ret) - 1);

                range_start = -999;
            }
        }

        /* No running range */
        else {
            if (isset) {
                range_start = i;
            }
        }
    }

    /* If we ended the bitmap with a range open, output it */
    if (range_start >= 0) {
        if (!first) {
            strncat(ret, ",", sizeof(ret) - strlen(ret) - 1);
            first = false;
        }

        range_end = i - 1;
        if (range_start == range_end) {
            snprintf(tmp, stmp, "%d", range_start);
        } else {
            snprintf(tmp, stmp, "%d-%d", range_start, range_end);
        }
        strncat(ret, tmp, sizeof(ret) - strlen(ret) - 1);
    }

    return ret;
}
