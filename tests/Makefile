SRCS=test_kos_single_threaded_put_get.c test_kos_single_threaded_put_remove_get.c test_kos_single_threaded_put_get_put_get.c test_kos_single_threaded_put_get_remove_put_get.c test_kos_single_threaded_put_dump.c test_kos_single_threaded_put_get_remove_put_dump.c  test_kos_multi_threaded_put_get.c test_kos_multi_threaded_put_get_remove_get.c   test_kos_multi_threaded_all.c test_kos_multi_threaded_all_getAll.c test_kos_multi_threaded_all_shared.c test_kos_multi_threaded_all_getAll_fromFile.c
OBJS=${SRCS:.c=}

INCLUDES =  -I ../include 
CC = gcc
LIBKOS =  ../kos/libkos.a
CFLAGS = -g -O0 -Wall -pthread $(INCLUDES) 

COMPILE = $(CC) $(DEFS) $(INCLUDES) $(CFLAGS)

all: $(OBJS)

test_kos_single_threaded_put_get: test_kos_single_threaded_put_get.c $(LIBKOS)

test_kos_single_threaded_put_remove_get: test_kos_single_threaded_put_remove_get.c $(LIBKOS)

test_kos_single_threaded_put_get_put_get: test_kos_single_threaded_put_get_put_get.c $(LIBKOS)

test_kos_single_threaded_put_get_remove_put_get: test_kos_single_threaded_put_get_remove_put_get.c $(LIBKOS)

test_kos_single_threaded_put_dump: test_kos_single_threaded_put_dump.c $(LIBKOS)

test_kos_single_threaded_put_get_remove_put_dump: test_kos_single_threaded_put_get_remove_put_dump.c $(LIBKOS)

test_kos_multi_threaded_put_get: test_kos_multi_threaded_put_get.c $(LIBKOS)

test_kos_multi_threaded_put_get_remove_get: test_kos_multi_threaded_put_get_remove_get.c $(LIBKOS)

test_kos_multi_threaded_all: test_kos_multi_threaded_all.c $(LIBKOS)

test_kos_multi_threaded_all_getAll: test_kos_multi_threaded_all_getAll.c $(LIBKOS)

test_kos_multi_threaded_all_shared: test_kos_multi_threaded_all_shared.c $(LIBKOS)

test_kos_multi_threaded_all_getAll_fromFile: test_kos_multi_threaded_all_getAll_fromFile.c $(LIBKOS)


clean: 
	rm -f $(OBJS)


# Tell versions [3.59,3.63) of GNU make to not export all variables.
# Otherwise a system limit (for SysV at least) may be exceeded.
.NOEXPORT:
