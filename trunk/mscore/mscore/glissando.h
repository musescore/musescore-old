//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2008-2009 Werner Schweer and others
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

#ifndef __GLISSANDO_H__
#define __GLISSANDO_H__

#include "element.h"
#include "ui_glissandoprop.h"

class Note;

//---------------------------------------------------------
//   Glissando
//---------------------------------------------------------

class Glissando : public Element {
      Q_DECLARE_TR_FUNCTIONS(Glissando)
      QLineF line;
      QString _text;
      bool _showText;

   public:
      Glissando(Score* s);
      virtual Glissando* clone() const { return new Glissando(*this); }
      virtual ElementType type() const { return GLISSANDO; }
      virtual Space space() const;
      virtual QRectF bbox() const;

      virtual void draw(Painter*) const;
      virtual void layout();
      virtual void write(Xml&) const;
      virtual void read(QDomElement);

      virtual bool genPropertyMenu(QMenu*) const;
      virtual void propertyAction(ScoreView*, const QString&);

      void setSize(const QSizeF&);        // used for palette

      QString text() const           { return _text;     }
      void setText(const QString& t) { _text = t;        }
      bool showText() const          { return _showText; }
      void setShowText(bool v)       { _showText = v;    }
      };

//---------------------------------------------------------
//   GlissandoProperties
//---------------------------------------------------------

class GlissandoProperties : public QDialog, public Ui::GlissandoProperties {
      Q_OBJECT
      Glissando* glissando;

   public slots:
      virtual void accept();

   public:
      GlissandoProperties(Glissando* tuplet, QWidget* parent = 0);
      };

#endif

