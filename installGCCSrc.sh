#mkdir gccsource
wget https://ftp.gnu.org/gnu/gcc/gcc-7.1.0/gcc-7.1.0.tar.gz

tar -xvzf gcc-7.1.0.tar.gz
rm -f gcc-7.1.0.tar.gz

yum install  gmp  gmp-devel  mpfr  mpfr-devel  libmpc  libmpc-devel

#cd gcc-7.1.0
#./contrib/download_prerequisites


mkdir gccbuild
cd gccbuild
../gcc-7.1.0/configure --disable-multilib


make BOOT_CFLAGS='-O' bootstrap
make install