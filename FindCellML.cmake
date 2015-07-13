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
# - Find CellML
# Find the CellML API includes and library
# This module defines
#  CELLML_INCLUDE_DIR, where to find CellML API header files.
#  CELLML_LIBRARIES, the libraries needed to use the CellML API.
#  CELLML_FOUND, If false, do not try to use the CellML API.
# also defined, but not for general use are
#  CELLML_LIBRARY, where to find the CellML library.

FIND_PATH(CELLML_INCLUDE_DIR IfaceCellML_APISPEC.hxx 
        /usr/include/
        /usr/local/include/
)

FIND_LIBRARY(CELLML_LIBRARY cellml
        /usr/lib 
        /usr/local/lib
) 

IF (CELLML_INCLUDE_DIR AND CELLML_LIBRARY)
   SET(CELLML_LIBRARIES ${CELLML_LIBRARY})
   SET(CELLML_FOUND TRUE)
ENDIF (CELLML_INCLUDE_DIR AND CELLML_LIBRARY)


IF (CELLML_FOUND)
   IF (NOT CellML_FIND_QUIETLY)
      MESSAGE(STATUS "Found CellML: ${CELLML_LIBRARIES}")
   ENDIF (NOT CellML_FIND_QUIETLY)
ELSE (CELLML_FOUND)
   IF (CellML_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could not find CellML")
   ELSE (CellML_FIND_REQUIRED)
      MESSAGE(STATUS "CellML API not found")
   ENDIF (CellML_FIND_REQUIRED)
ENDIF (CELLML_FOUND)
