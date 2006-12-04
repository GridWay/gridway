#! /bin/sh

libtoolize --force 2>&1 \
&& aclocal  2>&1 \
&& autoheader 2>&1 \
&& automake -i --gnu --add-missing 2>&1 \
&& autoconf  2>&1
