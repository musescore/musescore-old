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

#ifndef __MSCOREVIEW_H__
#define __MSCOREVIEW_H__

class Element;
class Score;

//---------------------------------------------------------
//   MuseScoreView
//---------------------------------------------------------

class MuseScoreView {
   public:
      MuseScoreView() {}
      virtual void dataChanged(const QRectF&) = 0;
      virtual void updateAll() = 0;
      virtual void moveCursor() = 0;
      virtual void adjustCanvasPosition(const Element* el, bool playBack) = 0;
      virtual void setScore(Score*) = 0;
      virtual void removeScore() = 0;
      };

#endif



