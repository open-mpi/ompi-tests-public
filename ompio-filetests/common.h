#ifndef COMMON_UTILS
#define COMMON_UTILS

#ifndef GPFS_SUPER_MAGIC
#define GPFS_SUPER_MAGIC  0x47504653
#endif

void SL_read ( int hdl, void *buf, int num );
void SL_write ( int hdl, void *buf, int num );

int  isGpfsFS(char *path, int *isGpfs);

#endif
