// $detav.hpp 3.0 milbo$ generate VJ and Rowley start shapes
// Forden jul 08
//-----------------------------------------------------------------------------
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// A copy of the GNU General Public License is available at
// http://www.r-project.org/Licenses/
//-----------------------------------------------------------------------------

#if !defined(detav_hpp)
#define detav_hpp

void GenDetAvShapes(SHAPE &VjAv,            // out
             SHAPE &RowleyAv,               // out
             const char sShapeFile[],       // in: shape filename e.g. muct68.shape
             bool  fSkipIfNotInShapeFile,   // in:
             const vec_SHAPE Shapes,        // in: ref shapes from file
             const vec_string &Tags,        // in
             const vec_int &nUsedLandmarks, // in
             const char sDataDir[]);        // in: for face detector data files

#endif // detav_hpp
