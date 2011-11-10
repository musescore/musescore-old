//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id$
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
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
      virtual void draw(QPainter*) const;
      virtual void layout();
      };

enum MarkerType {
      MARKER_SEGNO,
      MARKER_VARSEGNO,
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

      MarkerType _markerType;
      QString _label;               ///< referenced from Jump() element

      MarkerType markerType(const QString&) const;

   public:
      Marker(Score*);

      void setMarkerType(MarkerType t);
      MarkerType markerType() const    { return _markerType; }

      virtual Marker* clone() const    { return new Marker(*this); }
      virtual ElementType type() const { return MARKER; }

      Segment* segment() const         { return (Segment*)parent(); }
      Measure* measure() const         { return (Measure*)parent()->parent(); }

      virtual void read(QDomElement);
      virtual void write(Xml& xml) const;


      QString label() const            { return _label; }
      void setLabel(const QString& s)  { _label = s; }

      virtual void layout();
      virtual QPointF pagePos() const;
      virtual QLineF dragAnchor() const;
      virtual void styleChanged();
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

   public:
      Jump(Score*);

      void setJumpType(int t);
      int jumpType() const;

      virtual Jump* clone() const      { return new Jump(*this); }
      virtual ElementType type() const { return JUMP; }

      virtual void read(QDomElement);
      virtual void write(Xml& xml) const;

      QString jumpTo()               const { return _jumpTo;     }
      QString playUntil()            const { return _playUntil;  }
      QString continueAt()           const { return _continueAt; }
      void setJumpTo(const QString& s)     { _jumpTo = s;        }
      void setPlayUntil(const QString& s)  { _playUntil = s;     }
      void setContinueAt(const QString& s) { _continueAt = s;    }
      };

#endif

