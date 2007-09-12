#=============================================================================
#  Mscore
#  Linux Music Score Editor
#  $Id:$
#
#  Copyright (C) 2002-2007 by Werner Schweer and others
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License version 2.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#=============================================================================

default:
	if test ! -d build;                           \
         then                                       \
            mkdir build;                            \
            cd build;                               \
            cmake ../mscore;                        \
            make;                                   \
         else                                       \
            cd build; make -f Makefile;             \
         fi;

release:
	if test ! -d build;                           \
         then                                       \
            mkdir build;                            \
            cd build;                               \
            cmake -DCMAKE_BUILD_TYPE=RELEASE ../mscore; \
            make -f Makefile;                       \
         else                                       \
            echo "build directory does already exist, please remove first with 'make clean'";       \
         fi;

debug:
	if test ! -d build;                           \
         then                                       \
            mkdir build;                            \
            cd build;                               \
            cmake -DCMAKE_BUILD_TYPE=DEBUG ../mscore;         \
            make -f Makefile;                       \
         else                                       \
            echo "build directory does already exist, please remove first with 'make clean'";       \
         fi

win32:
	if test ! -d win32build;                      \
         then                                       \
            mkdir win32build;                       \
      	if test ! -d win32install;              \
               then                                 \
                  mkdir win32install;               \
            fi;                                     \
            cd win32build;                          \
            cmake -DCMAKE_TOOLCHAIN_FILE=../mscore/cmake/mingw32.cmake -DCMAKE_INSTALL_PREFIX=../win32install -DCMAKE_BUILD_TYPE=RELEASE  ../mscore; \
            make -f Makefile;                       \
            make install;                           \
         else                                       \
            echo "build directory win32build does alread exist, please remove first";       \
         fi

#
# clean out of source build
#

clean:
	-rm -rf build

#
# create source distribution
#

dist:
	mkdir mscore.dist
	cd mscore.dist; svn co https://mscore.svn.sourceforge.net/svnroot/mscore/trunk mscore-0.7.0
	cd mscore.dist; find . -name .svn -print0 | xargs -0 /bin/rm -rf
	cd mscore.dist; tar cvfj mscore-0.7.0.tar.bz2 mscore-0.7.0
	mv mscore.dist/mscore-0.7.0.tar.bz2 .

install:
	cd build; make install

#
# this creates a shell archive / installer for
#     Mscore binary
#

package:
	cd build; make package
	mv build/mscore-*.sh .
