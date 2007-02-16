#=============================================================================
#  Mscore
#  Linux Music Score Editor
#  $Id:$
#
#  Copyright (C) 2002-2006 by Werner Schweer and others
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
            echo "+creating build directory";       \
            mkdir build;                            \
            echo "+entering build directory";       \
            cd build;                               \
            echo "+calling cmake" ;                 \
            cmake ../mscore;                        \
         else                                       \
            echo "+entering build directory";       \
            cd build;                               \
         fi;                                        \
      echo "+start top level make...";              \
      make -f Makefile

release:
	if test ! -d build;                           \
         then                                       \
            echo "+creating build directory";       \
            mkdir build;                            \
            echo "+entering build directory";       \
            cd build;                               \
            echo "+calling cmake" ;                 \
            cmake -DCMAKE_BUILD_TYPE=RELEASE ../mscore;                        \
         else                                       \
            echo "build directory does alread exist";       \
            exit;                               \
         fi; \
         echo "release build is configured; now type make"

debug:
	if test ! -d build;                           \
         then                                       \
            echo "+creating build directory";       \
            mkdir build;                            \
            echo "+entering build directory";       \
            cd build;                               \
            echo "+calling cmake" ;                 \
            cmake -DCMAKE_BUILD_TYPE=DEBUG ../mscore;                        \
         else                                       \
            echo "build directory does alread exist";       \
            exit;                               \
         fi; \
         echo "debug build is configured; now type make"

win32:
	if test ! -d win32build;                           \
         then                                       \
            echo "+creating build directory";       \
            mkdir win32build;                            \
            echo "+entering build directory";       \
            cd win32build;                               \
            echo "+calling cmake" ;                 \
            cmake -DCMAKE_BUILD_TYPE=RLEASE -DCROSS_MINGW=ON ../mscore;                        \
         else                                       \
            echo "build directory does alread exist";       \
            exit;                               \
         fi; \
         echo "win32 build is now configured;"


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
	cd mscore.dist; svn co https://mscore.svn.sourceforge.net/svnroot/mscore/trunk mscore-0.4.0
	cd mscore.dist; find . -name .svn -print0 | xargs -0 /bin/rm -rf
	cd mscore.dist; tar cvfj mscore-0.4.0.tar.bz2 mscore-0.4.0
	mv mscore.dist/mscore-0.4.0.tar.bz2 .

install:
	cd build; make install

#
# this creates a shell archive / installer for
#     Mscore binary
#

package:
	cd build; make package
	mv build/mscore-*.sh .

