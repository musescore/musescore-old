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

#ifndef __TEMPOTEXT_H__
#define __TEMPOTEXT_H__

#include "text.h"
#include "ui_tempoproperties.h"

//---------------------------------------------------------
//   TempoText
//---------------------------------------------------------

class TempoText : public Text  {
      Q_DECLARE_TR_FUNCTIONS(TempoText)
      double _tempo;     // beats per second

   public:
      TempoText(Score*);
      virtual TempoText* clone() const { return new TempoText(*this); }
      virtual ElementType type() const { return TEMPO_TEXT; }
      virtual void write(Xml& xml) const;
      virtual void read(QDomElement);
      virtual bool genPropertyMenu(QMenu*) const;
      virtual void propertyAction(ScoreView*, const QString&);
      Segment* segment() const { return (Segment*)parent(); }
      Measure* measure() const { return (Measure*)parent()->parent(); }
      double tempo() const     { return _tempo; }
      void setTempo(double v)  { _tempo = v;  }
      };

//---------------------------------------------------------
//   TempoProperties
//    Dialog
//---------------------------------------------------------

class TempoProperties : public QDialog, public Ui::TempoProperties {
      Q_OBJECT

      TempoText* tempoText;

   private slots:
      void saveValues();

   public:
      TempoProperties(TempoText*, QWidget* parent = 0);
      };

#endif
