//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 2010 Werner Schweer and others
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

#ifndef __TREMOLOBAR_H__
#define __TREMOLOBAR_H__

#include "element.h"
#include "ui_tremolobar.h"
#include "pitchvalue.h"

//---------------------------------------------------------
//   TremoloBar
//---------------------------------------------------------

class TremoloBar : public Element {
      QList<PitchValue> _points;
      double _lw;
      QPointF notePos;
      double noteWidth;

   public:
      TremoloBar(Score* s);
      virtual TremoloBar* clone() const { return new TremoloBar(*this); }
      virtual ElementType type() const { return TREMOLOBAR; }
      virtual void layout();
      virtual void draw(QPainter&, ScoreView*) const;
      virtual void write(Xml&) const;
      virtual void read(QDomElement e);
      QList<PitchValue>& points()                { return _points; }
      const QList<PitchValue>& points() const    { return _points; }
      void setPoints(const QList<PitchValue>& p) { _points = p;    }
      virtual bool genPropertyMenu(QMenu*) const;
      virtual void propertyAction(ScoreView*, const QString&);
      };

//---------------------------------------------------------
//   TremoloBarProperties
//---------------------------------------------------------

class TremoloBarProperties : public QDialog, public Ui::TremoloBarDialog {
      Q_OBJECT
      TremoloBar* bend;
      QButtonGroup* bendTypes;

   private slots:
      void bendTypeChanged(int);

   public:
      TremoloBarProperties(TremoloBar*, QWidget* parent = 0);
      const QList<PitchValue>& points() const;
      };

#endif

