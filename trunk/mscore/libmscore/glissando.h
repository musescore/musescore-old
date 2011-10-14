//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id$
//
//  Copyright (C) 2008-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __GLISSANDO_H__
#define __GLISSANDO_H__

#include "element.h"

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

      virtual void draw(Painter*) const;
      virtual void layout();
      virtual void write(Xml&) const;
      virtual void read(QDomElement);

      void setSize(const QSizeF&);        // used for palette

      QString text() const           { return _text;     }
      void setText(const QString& t) { _text = t;        }
      bool showText() const          { return _showText; }
      void setShowText(bool v)       { _showText = v;    }
      };

#endif

