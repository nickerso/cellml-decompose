# ***** BEGIN LICENSE BLOCK *****
# Version: MPL 1.1/GPL 2.0/LGPL 2.1
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License. You may obtain a copy of the License at
# http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# The Original Code is decompose.
#
# The Initial Developer of the Original Code is
# David Nickerson <nickerso@users.sourceforge.net>.
# Portions created by the Initial Developer are Copyright (C) 2008
# the Initial Developer. All Rights Reserved.
#
# Contributor(s):
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 2 or later (the "GPL"), or
# the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
# in which case the provisions of the GPL or the LGPL are applicable instead
# of those above. If you wish to allow use of your version of this file only
# under the terms of either the GPL or the LGPL, and not to allow others to
# use your version of this file under the terms of the MPL, indicate your
# decision by deleting the provisions above and replace them with the notice
# and other provisions required by the GPL or the LGPL. If you do not delete
# the provisions above, a recipient may use your version of this file under
# the terms of any one of the MPL, the GPL or the LGPL.
#
# ***** END LICENSE BLOCK *****
# - Find CCGS
# Find the CellML API C Code Generation Service includes and library
# This module defines
#  CCGS_INCLUDE_DIR, where to find CCGS header files.
#  CCGS_LIBRARIES, the libraries needed to use the CCGS.
#  CCGS_FOUND, If false, do not try to use the CCGS.
# also defined, but not for general use are
#  CCGS_LIBRARY, where to find the CCGS library.

FIND_PATH(CCGS_INCLUDE_DIR IfaceCCGS.hxx 
        /usr/include/
        /usr/local/include/
)

FIND_LIBRARY(CCGS_LIBRARY ccgs
        /usr/lib 
        /usr/local/lib
)
FIND_LIBRARY(ANNO_TOOLS_LIBRARY annotools
        /usr/lib 
        /usr/local/lib
)
FIND_LIBRARY(CEVAS_LIBRARY cevas
        /usr/lib 
        /usr/local/lib
)
FIND_LIBRARY(CUSES_LIBRARY cuses
        /usr/lib 
        /usr/local/lib
)
FIND_LIBRARY(MALAES_LIBRARY malaes
        /usr/lib 
        /usr/local/lib
)

IF (CCGS_INCLUDE_DIR AND CCGS_LIBRARY AND ANNO_TOOLS_LIBRARY AND CEVAS_LIBRARY AND CUSES_LIBRARY AND MALAES_LIBRARY)
   SET(CCGS_LIBRARIES ${CCGS_LIBRARY} ${ANNO_TOOLS_LIBRARY} ${CEVAS_LIBRARY} ${CUSES_LIBRARY} ${MALAES_LIBRARY})
   SET(CCGS_FOUND TRUE)
ENDIF (CCGS_INCLUDE_DIR AND CCGS_LIBRARY AND ANNO_TOOLS_LIBRARY AND CEVAS_LIBRARY AND CUSES_LIBRARY AND MALAES_LIBRARY)


IF (CCGS_FOUND)
   IF (NOT CCGS_FIND_QUIETLY)
      MESSAGE(STATUS "Found CCGS: ${CCGS_LIBRARIES}")
   ENDIF (NOT CCGS_FIND_QUIETLY)
ELSE (CCGS_FOUND)
   IF (CCGS_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could not find CCGS")
   ELSE (CCGS_FIND_REQUIRED)
      MESSAGE(STATUS "CCGS not found")
   ENDIF (CCGS_FIND_REQUIRED)
ENDIF (CCGS_FOUND)
