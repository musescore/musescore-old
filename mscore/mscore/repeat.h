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

#ifndef __REPEAT_H__
#define __REPEAT_H__

#include "text.h"
#include "rest.h"

class Score;
class Segment;

//---------------------------------------------------------
//   RepeatMeasure
//---------------------------------------------------------

class RepeatMeasure : public Rest {
      QPainterPath path;

   public:
      RepeatMeasure(Score*);
      RepeatMeasure &operator=(const RepeatMeasure&);
      virtual RepeatMeasure* clone() const  { return new RepeatMeasure(*this); }
      virtual ElementType type() const      { return REPEAT_MEASURE; }
      virtual void draw(QPainter&) const;
      virtual void layout();
      virtual void read(QDomElement e)   { Element::read(e);    }
      virtual void write(Xml& xml) const { Element::write(xml); }
      virtual QRectF bbox() const        { return _bbox;        }
      virtual void scanElements(void* data, void (*func)(void*, Element*)) { func(data, this); }
      };

enum {
      MARKER_SEGNO,
      MARKER_CODA,
      MARKER_VARCODA,
      MARKER_CODETTA,
      MARKER_FINE,
      MARKER_TOCODA,
      MARKER_USER
      };

//---------------------------------------------------------
//   Marker
//---------------------------------------------------------

class Marker : public Text {
      Q_DECLARE_TR_FUNCTIONS(Marker)

      QString _label;               ///< referenced from Jump() element
      virtual bool isMovable() const   { return true; }

   public:
      Marker(Score*);

      void setMarkerType(int t);
      int markerType() const;

      virtual Marker* clone() const    { return new Marker(*this); }
      virtual ElementType type() const { return MARKER; }

      virtual void read(QDomElement);
      virtual void write(Xml& xml) const;

      virtual bool genPropertyMenu(QMenu*) const;
      virtual void propertyAction(ScoreView*, const QString&);

      QString label() const            { return _label; }
      void setLabel(const QString& s)  { _label = s; }
      };

enum {
      JUMP_DC,
      JUMP_DC_AL_FINE,
      JUMP_DC_AL_CODA,
      JUMP_DS_AL_CODA,
      JUMP_DS_AL_FINE,
      JUMP_DS,
      JUMP_USER
      };

//---------------------------------------------------------
//   Jump
//---------------------------------------------------------

class Jump : public Text {
      Q_DECLARE_TR_FUNCTIONS(Jump)

      QString _jumpTo;
      QString _playUntil;
      QString _continueAt;

      virtual bool isMovable() const { return true; }

   public:
      Jump(Score*);

      void setJumpType(int t);
      int jumpType() const;

      virtual Jump* clone() const      { return new Jump(*this); }
      virtual ElementType type() const { return JUMP; }

      virtual void read(QDomElement);
      virtual void write(Xml& xml) const;

      virtual bool genPropertyMenu(QMenu*) const;
      virtual void propertyAction(ScoreView*, const QString&);

      QString jumpTo()               const { return _jumpTo;     }
      QString playUntil()            const { return _playUntil;  }
      QString continueAt()           const { return _continueAt; }
      void setJumpTo(const QString& s)     { _jumpTo = s;        }
      void setPlayUntil(const QString& s)  { _playUntil = s;     }
      void setContinueAt(const QString& s) { _continueAt = s;    }
      };

#endif

