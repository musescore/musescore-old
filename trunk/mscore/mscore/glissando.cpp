//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 2008 Werner Schweer and others
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

#include "glissando.h"
#include "chord.h"
#include "segment.h"
#include "note.h"
#include "style.h"
#include "score.h"
#include "sym.h"

//---------------------------------------------------------
//   Glissando
//---------------------------------------------------------

Glissando::Glissando(Score* s)
  : Element(s)
      {
      _text = "gliss.";
      _showText = true;
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Glissando::layout(ScoreLayout*)
      {
      Chord* chord = static_cast<Chord*>(parent());
      if (chord == 0)
            return;
      Note* anchor2   = chord->upNote();
      Segment* s = chord->segment();
      s = s->prev();
      while (s) {
            if (s->subtype() == Segment::SegChordRest && s->element(track()))
                  break;
            s = s->prev();
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
      QPointF p1    = anchor1->canvasPos();
      QPointF p2    = anchor2->canvasPos();

      double xo = _spatium * .5;
      double x1 = anchor1->headWidth() - (p2.x() - p1.x()) + xo;
      double y2 = anchor2->pos().y();
      double y1 = (p1.y() - p2.y()) + y2;
      double x2 = anchor2->pos().x() - xo;
      bool up   = p2.y() < p1.y();
      setPos(0.0, 0.0);

      double yo = up ? _spatium * .5 : -_spatium * .5;
      line = QLineF(x1, y1 - yo, x2, y2 + yo);
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
            QString tag(e.tagName());
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

void Glissando::draw(QPainter& p) const
      {
      p.save();
      QPen pen(p.pen());
      pen.setWidthF(_spatium * .15);
      pen.setCapStyle(Qt::RoundCap);
      p.setPen(pen);

      double w = line.dx();
      double h = line.dy();

      double l = sqrt(w * w + h * h);
      p.translate(line.p1());
      double wi = asin(-h / l) * 180.0 / M_PI;
      p.rotate(-wi);

      if (subtype() == 0) {
            p.drawLine(0.0, 0.0, l, 0.0);
            }
#if 0
      else if (subtype() == 1) {
            double x    = 0.0;
            double step = _spatium * .5;
            double h1   = step * .8;
            bool up     = true;
            for (;;) {
                  double step1 = step;
                  if (x + step1 > l)
                        step1 = l - x;
                  if (up)
                        p.drawArc(x, -h1*.5, step1, h1, 0, -180 * 16);
                  else
                        p.drawArc(x, h1*.5, step1, -h1, 0, 180 * 16);
                  x += step;
                  if (x >= l)
                        break;
                  up = !up;
                  }
            }
#endif
      else if (subtype() == 1) {
            QRectF b = symbols[trillelementSym].bbox(mag());
            qreal w  = symbols[trillelementSym].width(mag());
            int n    = lrint(l / w);
            symbols[trillelementSym].draw(p, mag(), 0.0, b.height()*.5, n);
            }
      if (_showText) {
            TextStyle* st = score()->textStyle(TEXT_STYLE_GLISSANDO);
            QRectF r      = st->bbox(_text);
            if (r.width() < l) {
                  QFont f = st->font();
                  p.setFont(f);
                  double x = (l - r.width()) * .5;
                  p.drawText(x, -_spatium * .5, _text);
                  }
            }
      p.restore();
      }

//---------------------------------------------------------
//   space
//---------------------------------------------------------

void Glissando::space(double& min, double& extra) const
      {
      min   = _spatium * 2;
      extra = 0.0;
      }

//---------------------------------------------------------
//   bbox
//---------------------------------------------------------

QRectF Glissando::bbox() const
      {
      return QRectF(line.p1(), line.p2()).normalized();
      }

//---------------------------------------------------------
//   setSize
//    used for palette
//---------------------------------------------------------

void Glissando::setSize(const QSizeF& s)
      {
      line = QLineF(0.0, s.height(), s.width(), 0.0);
      }

//---------------------------------------------------------
//   genPropertyMenu
//---------------------------------------------------------

bool Glissando::genPropertyMenu(QMenu* popup) const
      {
      Element::genPropertyMenu(popup);
      QAction* a = popup->addAction(tr("Glissando Properties..."));
      a->setData("props");
      return true;
      }

//---------------------------------------------------------
//   propertyAction
//---------------------------------------------------------

void Glissando::propertyAction(const QString& s)
      {
      if (s == "props") {
            GlissandoProperties vp(this);
            vp.exec();
            }
      else
            Element::propertyAction(s);
      }

//---------------------------------------------------------
//   GlissandoProperties
//---------------------------------------------------------

GlissandoProperties::GlissandoProperties(Glissando* g, QWidget* parent)
   : QDialog(parent)
      {
      setupUi(this);
      glissando = g;
      showText->setChecked(glissando->showText());
      text->setText(glissando->text());
      }

//---------------------------------------------------------
//   accept
//---------------------------------------------------------

void GlissandoProperties::accept()
      {
      glissando->setShowText(showText->isChecked());
      glissando->setText(text->text());
      QDialog::accept();
      }


