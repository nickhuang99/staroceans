prefix=/usr
exec_prefix=${prefix}
bindir=${exec_prefix}/bin
libdir=${exec_prefix}/lib
includedir=${prefix}/include
ARCH=X86
SYS=LINUX
CC=gcc
CFLAGS=-Wall -I. -O4 -ffast-math -D__X264__ -DHAVE_MALLOC_H -DHAVE_MMXEXT -DHAVE_SSE2 -DARCH_X86 -DSYS_LINUX -DHAVE_PTHREAD -s -fomit-frame-pointer
LDFLAGS= -lm -lpthread -s
AS=nasm
ASFLAGS=-O2 -f elf
VFW=no
EXE=
VIS=no
HAVE_GETOPT_LONG=1
DEVNULL=/dev/null
CONFIGURE_ARGS= '--enable-pthread' '--prefix=/usr'
