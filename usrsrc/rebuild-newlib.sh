OSNAME=chickenos

BINUTILS_VER=2.20
GCC_VER=4.5.0
GMP_VER=5.0.1
MPFR_VER=2.4.2
NEWLIB_VER=1.20.0
MPC_VER=0.8.1

export TARGET=i386-pc-${OSNAME}
export PREFIX=`pwd`/local

# Fix patches with osname
PERLCMD="s/{{OSNAME}}/${OSNAME}/g"
perl -pi -e $PERLCMD *.patch
perl -pi -e $PERLCMD gcc-files/gcc/config/os.h

mkdir -p build
mkdir -p local
cd build

WFLAGS=-c

export PATH=$PREFIX/bin:$PATH

# Fetch each package


echo "PASS-2 COMPILE NEWLIB"
cp ../newlib-files/syscalls.c newlib-${NEWLIB_VER}/newlib/libc/sys/${OSNAME}/syscalls.c
cp -r -L  ../newlib-files/* newlib-${NEWLIB_VER}/newlib/libc/sys/${OSNAME}/.
cp ../newlib-files/syscall.h newlib-${NEWLIB_VER}/newlib/libc/sys/${OSNAME}/syscall.h

cd newlib-obj
../newlib-${NEWLIB_VER}/configure --enable-malloc-debugging --target=$TARGET --prefix=$PREFIX --with-gmp=$PREFIX --with-mpfr=$PREFIX || exit
make || exit
make install || exit
cd ..
