# Pretty Print HWLOC Process Binding

## Building

```shell
./autogen.sh
./configure --prefix=${YOUR_INSTALL_DIR} --with-hwloc=${HWLOC_INSTALL_PATH}
make
make install
````

## Running

### Default: Print HWLOC bitmap

```shell
shell$ get-pretty-cpu
  0/  0 on c660f5n18)  Process Bound  : 0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff
```

```shell
shell$ hwloc-bind core:2 get-pretty-cpu
  0/  0 on c660f5n18)  Process Bound  : 0x00ff0000
```

```shell
shell$ mpirun -np 2 get-pretty-cpu
  0/  2 on c660f5n18)  Process Bound  : 0x000000ff
  1/  2 on c660f5n18)  Process Bound  : 0x0000ff00
```

### Full descriptive output

```shell
shell$ get-pretty-cpu -b -f
  0/  0 on c660f5n18)  Process Bound  : socket 0[core  0[hwt 0-7]],socket 0[core  1[hwt 0-7]],socket 0[core  2[hwt 0-7]],socket 0[core  3[hwt 0-7]],socket 0[core  4[hwt 0-7]],socket 0[core  5[hwt 0-7]],socket 0[core  6[hwt 0-7]],socket 0[core  7[hwt 0-7]],socket 0[core  8[hwt 0-7]],socket 0[core  9[hwt 0-7]],socket 1[core 10[hwt 0-7]],socket 1[core 11[hwt 0-7]],socket 1[core 12[hwt 0-7]],socket 1[core 13[hwt 0-7]],socket 1[core 14[hwt 0-7]],socket 1[core 15[hwt 0-7]],socket 1[core 16[hwt 0-7]],socket 1[core 17[hwt 0-7]],socket 1[core 18[hwt 0-7]],socket 1[core 19[hwt 0-7]]
```

```shell
shell$ hwloc-bind core:2 get-pretty-cpu -b -f
  0/  0 on c660f5n18)  Process Bound  : socket 0[core  2[hwt 0-7]]
```

```shell
shell$ mpirun -np 2 get-pretty-cpu -b -f
  1/  2 on c660f5n18)  Process Bound  : socket 0[core  1[hwt 0-7]]
  0/  2 on c660f5n18)  Process Bound  : socket 0[core  0[hwt 0-7]]
```

### Full descriptive bracketed output

```shell
shell$ get-pretty-cpu -b -m
  0/  0 on c660f5n18)  Process Bound  : [BBBBBBBB/BBBBBBBB/BBBBBBBB/BBBBBBBB/BBBBBBBB/BBBBBBBB/BBBBBBBB/BBBBBBBB/BBBBBBBB/BBBBBBBB][BBBBBBBB/BBBBBBBB/BBBBBBBB/BBBBBBBB/BBBBBBBB/BBBBBBBB/BBBBBBBB/BBBBBBBB/BBBBBBBB/BBBBBBBB]
```

```shell
shell$ hwloc-bind core:2 get-pretty-cpu -b -m
  0/  0 on c660f5n18)  Process Bound  : [......../......../BBBBBBBB/......../......../......../......../......../......../........][......../......../......../......../......../......../......../......../......../........]
```

```shell
shell$ mpirun -np 2 get-pretty-cpu -b -m
  1/  2 on c660f5n18)  Process Bound  : [......../BBBBBBBB/......../......../......../......../......../......../......../........][......../......../......../......../......../......../......../......../......../........]
  0/  2 on c660f5n18)  Process Bound  : [BBBBBBBB/......../......../......../......../......../......../......../......../........][......../......../......../......../......../......../......../......../......../........]
```
