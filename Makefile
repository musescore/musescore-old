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

REVISION  = `cat mscore/mscore/revision.h`
CPUS      = `grep -c processor /proc/cpuinfo`

PREFIX    = "/usr/local"
#VERSION   = "1.0r${REVISION}"
VERSION   = 1.1

ROOT=`pwd`

release:
	if test ! -d build;                              \
         then                                          \
            mkdir build;                               \
            cd build;                                  \
            cmake -DCMAKE_BUILD_TYPE=RELEASE	       \
            	  -DCMAKE_INSTALL_PREFIX="${PREFIX}" \
            	   ../mscore; 			       \
            make lrelease;                             \
            make -j ${CPUS};                           \
         else                                          \
            echo "build directory does already exist, please remove first with 'make clean'"; \
         fi;

debug:
	if test ! -d build;                              \
         then                                          \
            mkdir build;                               \
            cd build;                                  \
            cmake -DCMAKE_BUILD_TYPE=DEBUG	       \
            	  -DCMAKE_INSTALL_PREFIX="${PREFIX}" \
            	   ../mscore; 			       \
            make lrelease;                             \
            make -j ${CPUS};                           \
         else                                          \
            echo "build directory does already exist, please remove first with 'make clean'";       \
         fi

#
#  win32
#     cross compile windows package
#     NOTE: there are some hardcoded path in CMake - files
#           will probably only work on my setup (ws)
#
win32:
	if test ! -d win32build;                         \
         then                                          \
            mkdir win32build;                          \
      	if test ! -d win32install;                 \
               then                                    \
                  mkdir win32install;                  \
            fi;                                        \
            cd win32build;                             \
            cmake -DCMAKE_TOOLCHAIN_FILE=../mscore/cmake/mingw32.cmake -DCMAKE_INSTALL_PREFIX=../win32install -DCMAKE_BUILD_TYPE=DEBUG  ../mscore; \
            make lrelease;                             \
            make -j ${CPUS};                           \
            make install;                              \
            make package;                              \
         else                                          \
            echo "build directory win32build does alread exist, please remove first"; \
         fi

#
# clean out of source build
#

clean:
	-rm -rf build
	-rm -rf win32build win32install

#
# dist
#     create source distribution
#     - get current version from sourceforge
#     - remove .svn directories
#     - tar
#     - untar & test build
#

dist:
	-rm -rf mscore.dist
	mkdir mscore.dist
	cd mscore.dist; svn co https://mscore.svn.sourceforge.net/svnroot/mscore/trunk mscore-${VERSION}
	cd mscore.dist; find . -name .svn -print0 | xargs -0 /bin/rm -rf
	cd mscore.dist; tar cvfj mscore-${VERSION}.tar.bz2 mscore-${VERSION}
	mv mscore.dist/mscore-${VERSION}.tar.bz2 .

testdist:
	tar xvofj mscore-${VERSION}.tar.bz2
	cd mscore-${VERSION}; make release

revision:
	@svnversion -n  > mscore/mscore/revision.h

version: revision
	@echo ${VERSION}

install:
	cd build; make install

#
#  linux
#     linux binary package build
#
unix:
	if test ! -d linux;                          \
         then                                      \
            mkdir linux;                           \
            cd linux; \
            cmake -DCMAKE_BUILD_TYPE=RELEASE  ../mscore; \
            make -j${CPUS} -f Makefile;            \
            make package;                          \
         else                                      \
            echo "build directory linux does alread exist, please remove first";  \
         fi


