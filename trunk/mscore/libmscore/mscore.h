//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
//
//  Copyright (C) 2011 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#ifndef __MSCORE_H__
#define __MSCORE_H__

class Style;

//---------------------------------------------------------
//   MScore
//    MuseScore application object
//---------------------------------------------------------

class MScore {
      static Style* _defaultStyle;
      static Style* _baseStyle;
      static QString _globalShare;

   public:
      static void init();
      static Style* defaultStyle();
      static Style* baseStyle();
      static const QString& globalShare() { return _globalShare; }
      };


#endif

