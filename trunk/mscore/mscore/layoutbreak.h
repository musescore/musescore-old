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

#ifndef __LAYOUTBREAK_H__
#define __LAYOUTBREAK_H__

#include "element.h"
#include "ui_sectionbreak.h"

// layout break subtypes:

enum { LAYOUT_BREAK_PAGE, LAYOUT_BREAK_LINE, LAYOUT_BREAK_SECTION };

//---------------------------------------------------------
//   LayoutBreak
//    symbols for line break, page break etc.
//---------------------------------------------------------

class LayoutBreak : public Element {
      qreal lw;
      QPainterPath path;
      double _pause;

      virtual void draw(QPainter&, ScoreView*) const;
      virtual void layout();

   public:
      LayoutBreak(Score*);
      virtual LayoutBreak* clone() const { return new LayoutBreak(*this); }
      virtual ElementType type() const { return LAYOUT_BREAK; }
      virtual void setSubtype(const QString&);
      virtual void setSubtype(int st) { Element::setSubtype(st); }
      virtual const QString subtypeName() const;
      virtual bool acceptDrop(ScoreView*, const QPointF&, int, int) const;
      virtual Element* drop(ScoreView*, const QPointF&, const QPointF&, Element*);
      virtual bool genPropertyMenu(QMenu*) const;
      virtual void propertyAction(ScoreView*, const QString&);
      virtual void write(Xml&) const;
      virtual void read(QDomElement);
      double pause() const    { return _pause; }
      void setPause(double v) { _pause = v; }
      };

//---------------------------------------------------------
//   SectionBreakProperties
//---------------------------------------------------------

class SectionBreakProperties : public QDialog, public Ui::SectionBreakProperties {
      Q_OBJECT

   public:
      SectionBreakProperties(LayoutBreak*, QWidget* parent = 0);
      double pause() const;
      };

#endif
