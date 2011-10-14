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

#include "glissando.h"
#include "chord.h"
#include "segment.h"
#include "note.h"
#include "style.h"
#include "score.h"
#include "sym.h"
#include "painter.h"

//---------------------------------------------------------
//   Glissando
//---------------------------------------------------------

Glissando::Glissando(Score* s)
  : Element(s)
      {
      setFlags(ELEMENT_MOVABLE | ELEMENT_SELECTABLE);
      _text = "gliss.";
      _showText = true;
      qreal _spatium = spatium();
      setSize(QSizeF(_spatium * 2, _spatium * 4));    // for use in palettes
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Glissando::layout()
      {
      Chord* chord = static_cast<Chord*>(parent());
      if (chord == 0)
            return;
      Note* anchor2   = chord->upNote();
      Segment* s = chord->segment();
      s = s->prev1();
      while (s) {
            if ((s->subtype() == SegChordRest || s->subtype() == SegGrace) && s->element(track()))
                  break;
            s = s->prev1();
            }
      if (s == 0) {
            printf("no segment for first note of glissando found\n");
            return;
            }
      ChordRest* cr = static_cast<ChordRest*>(s->element(track()));
      if (cr == 0 || cr->type() != CHORD) {
            printf("no first note for glissando found, track %d\n", track());
            return;
            }
      Note* anchor1 = static_cast<Chord*>(cr)->upNote();

      setPos(0.0, 0.0);

      QPointF cp1    = anchor1->pagePos();
      QPointF cp2    = anchor2->pagePos();

      // construct line from notehead to notehead
      qreal x1 = (anchor1->headWidth()) - (cp2.x() - cp1.x());
      qreal y1 = anchor1->pos().y();
      qreal x2 = anchor2->pos().x();
      qreal y2 = anchor2->pos().y();
      QLineF fullLine(x1, y1, x2, y2);

      // shorten line on each side by offsets
      qreal xo = spatium() * .5;
      qreal yo = xo;   // spatium() * .5;
      QPointF p1 = fullLine.pointAt(xo / fullLine.length());
      QPointF p2 = fullLine.pointAt(1 - (yo / fullLine.length()));

      line = QLineF(p1, p2);
      setbbox(QRectF(line.p1(), line.p2()).normalized());
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Glissando::write(Xml& xml) const
      {
      xml.stag("Glissando");
      if (_showText && !_text.isEmpty())
            xml.tag("text", _text);
      Element::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Glissando::read(QDomElement e)
      {
      _showText = false;
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            const QString& tag(e.tagName());
            if (tag == "text") {
                  _showText = true;
                  _text = e.text();
                  }
            else if (!Element::readProperties(e))
                  domError(e);
            }
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Glissando::draw(Painter* painter) const
      {
      qreal _spatium = spatium();

      painter->save();
      painter->setLineWidth(_spatium * .15);
      painter->setCapStyle(Qt::RoundCap);

      qreal w = line.dx();
      qreal h = line.dy();

      qreal l = sqrt(w * w + h * h);
      painter->translate(line.p1());
      qreal wi = asin(-h / l) * 180.0 / M_PI;
      painter->rotate(-wi);

      if (subtype() == 0) {
            painter->drawLine(0.0, 0.0, l, 0.0);
            }
      else if (subtype() == 1) {
            qreal mags = magS();
            QRectF b = symbols[score()->symIdx()][trillelementSym].bbox(mags);
            qreal w  = symbols[score()->symIdx()][trillelementSym].width(mags);
            int n    = lrint(l / w);
            symbols[score()->symIdx()][trillelementSym].draw(painter, mags, 0.0, b.height()*.5, n);
            }
      if (_showText) {
            const TextStyle& st = score()->textStyle(TEXT_STYLE_GLISSANDO);
            QFont f = st.fontPx(_spatium);
            QRectF r = QFontMetricsF(f).boundingRect(_text);
            if (r.width() < l) {
                  QFont f = st.fontPx(_spatium);
                  painter->setFont(f);
                  qreal x = (l - r.width()) * .5;
                  painter->drawText(x, -_spatium * .5, _text);
                  }
            }
      painter->restore();
      }

//---------------------------------------------------------
//   space
//---------------------------------------------------------

Space Glissando::space() const
      {
      return Space(0.0, spatium() * 2.0);
      }

//---------------------------------------------------------
//   setSize
//    used for palette
//---------------------------------------------------------

void Glissando::setSize(const QSizeF& s)
      {
      line = QLineF(0.0, s.height(), s.width(), 0.0);
      }

