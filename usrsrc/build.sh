OSNAME=chickenos

BINUTILS_VER=2.20
GCC_VER=4.5.4
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

echo "FETCH BINUTILS"
if [ ! -f binutils-${BINUTILS_VER}.tar.gz ];
then
	wget $WFLAGS http://mirrors.usc.edu/pub/gnu/binutils/binutils-${BINUTILS_VER}.tar.gz
else
	echo "ALREADY FETCHED BINUTILS"
fi
tar -xf binutils-${BINUTILS_VER}.tar.gz

echo "FETCH GCC"
if [ ! -f gcc-core-${GCC_VER}.tar.gz ];
then
	wget $WFLAGS http://ftp.gnu.org/gnu/gcc/gcc-${GCC_VER}/gcc-core-${GCC_VER}.tar.gz
fi
tar -xf gcc-core-${GCC_VER}.tar.gz
if [ ! -f gcc-g++-${GCC_VER}.tar.gz ];
then
	wget $WFLAGS http://ftp.gnu.org/gnu/gcc/gcc-${GCC_VER}/gcc-g++-${GCC_VER}.tar.gz
fi
tar -xf gcc-g++-${GCC_VER}.tar.gz
if [ ! -f gcc-fortran-${GCC_VER}.tar.gz ];
then
	wget $WFLAGS http://ftp.gnu.org/gnu/gcc/gcc-${GCC_VER}/gcc-fortran-${GCC_VER}.tar.gz
fi
tar -xf gcc-fortran-${GCC_VER}.tar.gz

echo "FETCH GMP"
if [ ! -f gmp-${GMP_VER}.tar.gz ];
then
	wget $WFLAGS http://ftp.gnu.org/gnu/gmp/gmp-${GMP_VER}.tar.gz
fi
tar -xf gmp-${GMP_VER}.tar.gz

echo "FETCH MPFR"
if [ ! -f mpfr-${MPFR_VER}.tar.gz ];
then
	wget $WFLAGS http://ftp.gnu.org/gnu/mpfr/mpfr-${MPFR_VER}.tar.gz
fi
tar -xf mpfr-${MPFR_VER}.tar.gz

echo "FETCH MPC"
wget $WFLAGS http://www.multiprecision.org/mpc/download/mpc-${MPC_VER}.tar.gz
tar -xf mpc-${MPC_VER}.tar.gz

echo "FETCH NEWLIB"
if [ ! -f ewlib-${NEWLIB_VER}.tar.gz ];
then
	wget $WFLAGS ftp://sources.redhat.com/pub/newlib/newlib-${NEWLIB_VER}.tar.gz
fi
tar -xf newlib-${NEWLIB_VER}.tar.gz

# Patch and push new code into each package

echo "PATCH BINUTILS"
patch -p0 -d binutils-${BINUTILS_VER} < ../binutils.patch || exit
cp ../binutils-files/ld/emulparams/os_i386.sh binutils-${BINUTILS_VER}/ld/emulparams/${OSNAME}_i386.sh

echo "PATCH GCC"
patch -p0 -d gcc-${GCC_VER} < ../gcc.patch || exit
cp ../gcc-files/gcc/config/os.h gcc-${GCC_VER}/gcc/config/${OSNAME}.h

echo "PATCH NEWLIB"
patch -p0 -d newlib-${NEWLIB_VER} < ../newlib.patch || exit
mkdir -p newlib-${NEWLIB_VER}/newlib/libc/sys/${OSNAME}
cp -r -L  ../newlib-files/* newlib-${NEWLIB_VER}/newlib/libc/sys/${OSNAME}/.
cp ../newlib-files/vanilla-syscalls.c newlib-${NEWLIB_VER}/newlib/libc/sys/${OSNAME}/syscalls.c
cp ../newlib-files/syscall.h newlib-${NEWLIB_VER}/newlib/libc/sys/${OSNAME}/syscall.h

echo "MAKE OBJECT DIRECTORIES"
mkdir -p binutils-obj
mkdir -p gcc-obj
mkdir -p newlib-obj
mkdir -p gmp-obj
mkdir -p mpfr-obj
mkdir -p mpc-obj

# Compile all packages

echo "COMPILE BINUTILS"
cd binutils-obj
../binutils-${BINUTILS_VER}/configure --target=$TARGET --prefix=$PREFIX --disable-werror || exit
make || exit
make install || exit
cd ..

echo "COMPILE GMP"
cd gmp-obj
../gmp-${GMP_VER}/configure --prefix=$PREFIX --disable-shared || exit
make || exit
make check || exit
make install || exit
cd ..

echo "COMPILE MPFR"
cd mpfr-obj
../mpfr-${MPFR_VER}/configure --prefix=$PREFIX --with-gmp=$PREFIX --disable-shared
make || exit
make check || exit
make install || exit
cd ..

echo "COMPILE MPC"
cd mpc-obj
../mpc-${MPC_VER}/configure --target=$TARGET --prefix=$PREFIX --with-gmp=$PREFIX --with-mpfr=$PREFIX --disable-shared || exit
make || exit
make check || exit
make install || exit
cd ..


echo "AUTOCONF GCC"
cd gcc-${GCC_VER}/libstdc++-v3
#autoconf || exit
cd ../..

echo "COMPILE GCC"
cd gcc-obj
../gcc-${GCC_VER}/configure --target=$TARGET --prefix=$PREFIX --enable-languages=c,c++ --disable-libssp --with-gmp=$PREFIX --with-mpfr=$PREFIX --with-mpc=$PREFIX --disable-nls --with-newlib || exit

make all-gcc || exit
make install-gcc || exit
cd ..

echo "AUTOCONF NEWLIB"
cd newlib-${NEWLIB_VER}/newlib/libc/sys
autoconf || exit
cd ${OSNAME}
autoreconf || exit
cd ../../../../..

echo "CONFIGURE NEWLIB"
cd newlib-obj
../newlib-${NEWLIB_VER}/configure --target=$TARGET --prefix=$PREFIX --with-gmp=$PREFIX --with-mpfr=$PREFIX || exit

echo "COMPILE NEWLIB"
make || exit
make install || exit
cd ..

echo "PASS-2 COMPILE GCC"
cd gcc-obj
#make all-target-libgcc
#make install-target-libgcc
make all-target-libstdc++-v3 || exit
make install-target-libstdc++-v3 || exit
make || exit
make install || exit
cd ..

echo "PASS-2 COMPILE NEWLIB"
cp ../newlib-files/syscalls.c newlib-${NEWLIB_VER}/newlib/libc/sys/${OSNAME}/syscalls.c

cd newlib-obj
#../newlib-${NEWLIB_VER}/configure --target=$TARGET --prefix=$PREFIX --with-gmp=$PREFIX --with-mpfr=$PREFIX || exit
make || exit
make install || exit
cd ..
