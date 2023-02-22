#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include <hwloc.h>

#include "include/utils.h"

static hwloc_topology_t topology;
static int global_rank, global_size;
static int local_rank, local_size;
static char hostname[HOST_NAME_MAX] = { '\0' };

static void display_message(char * fmt, ...);

bool is_verbose = false;
bool is_quiet = false;
bool report_smallest = false;
bool report_full = false;
bool report_full_map = false;
bool report_hwloc_bind = true;

int main(int argc, char **argv)
{
    hwloc_topology_t topology;
    hwloc_bitmap_t bound_set;
    hwloc_obj_t obj;
    char type[64];
    int i;
    char pretty_str[PRETTY_LEN];
    char *buffer_str = NULL;
    char whoami_str[PRETTY_LEN];


    /*
     * Simple arg parsing
     */
    if( argc > 0 ) {
        for( i = 1; i < argc; ++i ) {
            if( 0 == strcmp(argv[i], "-v") ||
                0 == strcmp(argv[i], "--v") ||
                0 == strcmp(argv[i], "-verbose") ||
                0 == strcmp(argv[i], "--verbose") ) {
                is_verbose = true;
            }
            else if( 0 == strcmp(argv[i], "-q") ||
                     0 == strcmp(argv[i], "--q") ||
                     0 == strcmp(argv[i], "-quiet") ||
                     0 == strcmp(argv[i], "--quiet") ) {
                is_quiet = true;
            }
            else if( 0 == strcmp(argv[i], "-s") ||
                     0 == strcmp(argv[i], "--s") ||
                     0 == strcmp(argv[i], "-smallest") ||
                     0 == strcmp(argv[i], "--smallest") ) {
                report_smallest = true;
            }
            else if( 0 == strcmp(argv[i], "-f") ||
                     0 == strcmp(argv[i], "--f") ||
                     0 == strcmp(argv[i], "-full") ||
                     0 == strcmp(argv[i], "--full") ) {
                report_full = true;
            }
            else if( 0 == strcmp(argv[i], "-m") ||
                     0 == strcmp(argv[i], "--m") ||
                     0 == strcmp(argv[i], "-map") ||
                     0 == strcmp(argv[i], "--map") ) {
                report_full_map = true;
            }
            else if( 0 == strcmp(argv[i], "-b") ||
                     0 == strcmp(argv[i], "--b") ||
                     0 == strcmp(argv[i], "-no-bind") ||
                     0 == strcmp(argv[i], "--no-bind") ) {
                report_hwloc_bind = false;
            }
        }
    }

    gethostname(hostname, HOST_NAME_MAX);

    /* Get rank/size information from the launching environment */
    get_rank_size_info(&global_rank, &global_size,
                       &local_rank, &local_size);
    sprintf(whoami_str, "%3d/%3d on %s) ", global_rank, global_size, hostname);

    /* Allocate and initialize topology object. */
    hwloc_topology_init(&topology);

    /* Perform the topology detection. */
    hwloc_topology_load(topology);

    /* retrieve the CPU binding of the current entire process */
    bound_set = hwloc_bitmap_alloc();

    hwloc_get_cpubind(topology, bound_set, HWLOC_CPUBIND_PROCESS);

    /* print the smallest object covering the current process binding */
    if( report_smallest ) {
        obj = hwloc_get_obj_covering_cpuset(topology, bound_set);
        if( NULL == obj ) {
            display_message("Not bound\n");
        } else {
            hwloc_obj_type_snprintf(type, sizeof(type), obj, 0);
            display_message("Bound to \"%s\" logical index %u (physical index %u)\n",
                            type, obj->logical_index, obj->os_index);
        }
    }

    /* print the full descriptive output */
    if( report_full ) {
        opal_hwloc_base_cset2str(pretty_str, PRETTY_LEN, topology, bound_set, true );
        if( is_verbose ) {
            printf("%s Process Bound  :\n%s\n", whoami_str, pretty_str);
        } else {
            printf("%s Process Bound  : %s\n", whoami_str, pretty_str);
        }
    }

    /* print the full bracketed map output */
    if( report_full_map ) {
        opal_hwloc_base_cset2mapstr(pretty_str, PRETTY_LEN, topology, bound_set);
        if( is_verbose ) {
            printf("%s Process Bound  :\n%s\n", whoami_str, pretty_str);
        } else {
            printf("%s Process Bound  : %s\n", whoami_str, pretty_str);
        }
    }

    /* print the hwloc binding bitmap */
    if( report_hwloc_bind ) {
        hwloc_bitmap_asprintf(&buffer_str, bound_set);
        if( is_verbose ) {
            printf("%s Process Bound  :\n%s\n", whoami_str, buffer_str);
        } else {
            printf("%s Process Bound  : %s\n", whoami_str, buffer_str);
        }        
        free(buffer_str);
    }

    /* Destroy topology object. */
    hwloc_topology_destroy(topology);

    return 0;
}


static void display_message(char *fmt, ...)
{
    va_list args;

    printf("%3d/%3d on %s (%3d/%3d): ",
           global_rank, global_size,
           hostname,
           local_rank, local_size);
    va_start(args, fmt);

    vprintf(fmt, args);
    if( '\n' != fmt[strlen(fmt)-1] ) {
        printf("\n");
    }

    va_end(args);
}
