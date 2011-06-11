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
//  the file LICENCE.GPL
//=============================================================================

#ifndef __PAINTER_H__
#define __PAINTER_H__

//---------------------------------------------------------
//   class Painter
//    This class defines the interface to the render
//    a score.
//---------------------------------------------------------

class Painter {

   public:
      Painter() {}

      void translate(const QPointF& pt);
      void scale(qreal v);
      void drawText(const QPointF& p, const QString& s);
      void drawText(qreal x, qreal y, const QString& s);
      };

#endif

