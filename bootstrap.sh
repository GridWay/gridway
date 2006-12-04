#! /bin/sh

if [ ! -d config ] ; then
    mkdir config
    autoscan
    touch makefile-header
fi    

libtoolize --force 2>&1 \
&& aclocal  2>&1 \
&& autoheader 2>&1 \
&& automake -i --foreign --copy --add-missing 2>&1 \
&& autoconf  2>&1
