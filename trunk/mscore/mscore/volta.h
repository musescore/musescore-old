//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2002-2009 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#ifndef __VOLTA_H__
#define __VOLTA_H__

#include "textline.h"

class Score;
class Xml;
class Volta;

//---------------------------------------------------------
//   VoltaSegment
//---------------------------------------------------------

class VoltaSegment : public TextLineSegment {
      Q_DECLARE_TR_FUNCTIONS(VoltaSegment)

   protected:

   public:
      VoltaSegment(Score* s) : TextLineSegment(s) {}
      virtual ElementType type() const     { return VOLTA_SEGMENT; }
      virtual VoltaSegment* clone() const  { return new VoltaSegment(*this); }
      Volta* volta() const                 { return (Volta*)parent(); }
      virtual bool genPropertyMenu(QMenu*) const;
      virtual void propertyAction(ScoreView*, const QString&);
      };

//---------------------------------------------------------
//   Volta
//---------------------------------------------------------

class Volta : public TextLine {
      QList<int> _endings;

   public:
      enum { VOLTA_OPEN, VOLTA_CLOSED };

      Volta(Score* s);
      virtual Volta* clone() const { return new Volta(*this); }
      virtual ElementType type() const { return VOLTA; }
      virtual LineSegment* createLineSegment();
      virtual void layout();

      virtual void write(Xml&) const;
      virtual void read(QDomElement e);

      QList<int> endings() const           { return _endings; }
      QList<int>& endings()                { return _endings; }
      void setEndings(const QList<int>& l) { _endings = l;    }
      void setText(const QString& s);
      QString text() const;
      virtual void setSubtype(int val);
      bool hasEnding(int repeat) const;
      };

#endif

