#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/statfs.h>
#include "common.h"

void SL_read ( int hdl, void *buf, int num )
{
    int lcount=0;
    int a;
    char *wbuf = ( char *)buf;
    do {
        a = read ( hdl, wbuf, num);

        if(0 == a && num > 0){
            printf("\nSL_read: Warning: # Bytes read are less than expected file size %d %s\n", hdl, strerror(errno));
            return;
        }

        if ( a == -1 ) {
            if ( errno == EINTR || errno == EAGAIN ||
                 errno == EINPROGRESS || errno == EWOULDBLOCK) {
                continue;
            }
            else {
                printf("SL_read: error while reading from file %d %s\n", hdl, strerror(errno));
                return;
            }
            lcount++;
            a=0;
        }

        num -= a;
        wbuf += a;
        
    } while ( num > 0 &&  lcount < 20 );

    return;
}

void SL_write ( int hdl, void *buf, int num )
{
    int lcount=0;
    int a;
    char *wbuf = ( char *)buf;
    
    do {
    a = write ( hdl, wbuf, num);
    if ( a == -1 ) {
        if ( errno == EINTR || errno == EAGAIN ||
             errno == EINPROGRESS || errno == EWOULDBLOCK) {
            continue;
        }
        else {
            printf("SL_write: error while writing to file %d %s\n", hdl, strerror(errno));
            return;
        }
        lcount++;
        a=0;
    }   

    num -= a;
    wbuf += a;
    
    } while ( num > 0 );

    return;
}

int isGpfsFS(char *path, int *isGpfs)
{
    struct statfs fsbuf;
    int ret;

    ret = statfs(path, &fsbuf);
    if ((0 == ret) && (GPFS_SUPER_MAGIC == (fsbuf.f_type & 0xffffffff))) {
        *isGpfs = 1;
    }
    else {
        *isGpfs = 0;
    }
    return ret;
}
