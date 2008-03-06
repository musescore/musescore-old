//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: dynamics.cpp,v 1.23 2006/03/28 14:58:58 wschweer Exp $
//
//  Copyright (C) 2002-2007 Werner Schweer and others
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

#include "dynamics.h"
#include "xml.h"
#include "score.h"
#include "measure.h"
#include "system.h"
#include "segment.h"
#include "utils.h"
#include "style.h"
#include "layout.h"

static const QChar pp[]     = { 0x70, 0x70, 0x70, 0x70, 0x70, 0x70};
static const QChar mp[]     = { 0x6d, 0x70 };               // mp
static const QChar mf[]     = { 0x6d, 0x66 };               // mf
static const QChar ff[]     = { 0x66, 0x66, 0x66, 0x66, 0x66, 0x66};
static const QChar fp[]     = { 0x66, 0x70 };               // fp
static const QChar sfz[]    = { 0x73, 0x66, 0x7a };         // sfz
static const QChar sffz[]   = { 0x73, 0x66, 0x66, 0x7a };   // sffz
static const QChar sfp[]    = { 0x73, 0x66, 0x70, 0x70};    // sfpp
static const QChar rfz[]    = { 0x72, 0x66, 0x7a };         // rfz
static const QChar fz[]     = { 0x66, 0x7a };               // fz
static const QChar z[]      = { 0x7a };                     // z

Dyn dynList[] = {
      Dyn(TEXT_STYLE_DYNAMICS,   -1,  "other-dynamics", QString("")),
      Dyn(TEXT_STYLE_DYNAMICS1,   1,  "pppppp", QString(pp, 6)),
      Dyn(TEXT_STYLE_DYNAMICS1,   5,  "ppppp",  QString(pp, 5)),
      Dyn(TEXT_STYLE_DYNAMICS1,  10,  "pppp",   QString(pp, 4)),
      Dyn(TEXT_STYLE_DYNAMICS1,  30,  "ppp",    QString(pp, 3)),
      Dyn(TEXT_STYLE_DYNAMICS1,  50,  "pp",     QString(pp, 2)),
      Dyn(TEXT_STYLE_DYNAMICS1,  60,  "p",      QString(pp, 1)),
      Dyn(TEXT_STYLE_DYNAMICS1,  70,  "mp",     QString(mp,  2)),
      Dyn(TEXT_STYLE_DYNAMICS1,  80,  "mf",     QString(mf,  2)),
      Dyn(TEXT_STYLE_DYNAMICS1,  90,  "f",      QString(ff,  1)),
      Dyn(TEXT_STYLE_DYNAMICS1, 100,  "ff",     QString(ff,  2)),
      Dyn(TEXT_STYLE_DYNAMICS1, 110,  "fff",    QString(ff,  3)),
      Dyn(TEXT_STYLE_DYNAMICS1, 120,  "ffff",   QString(ff,  4)),
      Dyn(TEXT_STYLE_DYNAMICS1, 125,  "fffff",  QString(ff,  5)),
      Dyn(TEXT_STYLE_DYNAMICS1, 127,  "ffffff", QString(ff,  6)),
      Dyn(TEXT_STYLE_DYNAMICS1,  -1,  "fp",     QString(fp,  2)),
      Dyn(TEXT_STYLE_DYNAMICS1,  -1,  "sf",     QString(sfz, 2)),
      Dyn(TEXT_STYLE_DYNAMICS1,  -1,  "sfz",    QString(sfz, 3)),
      Dyn(TEXT_STYLE_DYNAMICS1,  -1,  "sffz",   QString(sffz, 4)),
      Dyn(TEXT_STYLE_DYNAMICS1,  -1,  "sfp",    QString(sfp, 3)),
      Dyn(TEXT_STYLE_DYNAMICS1,  -1,  "sfpp",   QString(sfp, 4)),
      Dyn(TEXT_STYLE_DYNAMICS1,  -1,  "rfz",    QString(rfz, 3)),
      Dyn(TEXT_STYLE_DYNAMICS1,  -1,  "rf",     QString(rfz, 2)),
      Dyn(TEXT_STYLE_DYNAMICS1,  -1,  "fz",     QString(fz,  2)),
      Dyn(TEXT_STYLE_DYNAMICS1,  -1,  "m",      QString(mp,  1)),
      Dyn(TEXT_STYLE_DYNAMICS1,  -1,  "r",      QString(rfz, 1)),
      Dyn(TEXT_STYLE_DYNAMICS1,  -1,  "s",      QString(sfz, 1)),
      Dyn(TEXT_STYLE_DYNAMICS1,  -1,  "z",      QString(z,   1)),
      };

//---------------------------------------------------------
//   Dynamic
//---------------------------------------------------------

Dynamic::Dynamic(Score* s)
   : Text(s)
      {
      }

Dynamic::Dynamic(const Dynamic& d)
   : Text(d)
      {
      setSubtype(subtype());
      }

//---------------------------------------------------------
//   setSubtype
//---------------------------------------------------------

void Dynamic::setSubtype(int idx)
      {
      Element::setSubtype(idx);
      setStyle(score()->textStyle(dynList[idx].textStyle));
      if (idx != 0)
            setText(dynList[idx].str);
      }

//---------------------------------------------------------
//   isMovable
//---------------------------------------------------------

bool Dynamic::isMovable() const
      {
      dragOffset = QPointF();
      return true;
      }

//---------------------------------------------------------
//   drag
//---------------------------------------------------------

QRectF Dynamic::drag(const QPointF& pos)
      {
      QRectF r(abbox());
      setUserOff((pos - dragOffset) / _spatium);
#if 0
      int ntick;
      int stfi = staffIdx();
      int line;
      Segment* seg;
//      MeasureBase* mb = _score->pos2measure(canvasPos(), &ntick, &stfi, 0, 0, 0);
      MeasureBase* mb = _score->pos2measure2(canvasPos(), &ntick, &stfi, &line, &seg);
      if (mb && mb->type() == MEASURE) {
            Measure* measure = (Measure*)mb;
            QPointF op = canvasPos();
            setTick(ntick);
            setTrack(stfi * VOICES);
            if (measure != parent()) {
                  ((Measure*)parent())->remove(this);
                  measure->add(this);
                  }
            QPointF uo(op - layoutPos());
            setUserOff(uo / _spatium);
            dragOffset = pos - uo;
            }
      else
            printf("Dynamic::drag(): measure not found\n");
#endif
      return abbox() | r;
      }

//---------------------------------------------------------
//   endDrag
//    bind to nearest tick position
//---------------------------------------------------------

void Dynamic::endDrag()
      {
      int ntick;
      int stfi = staffIdx();
      MeasureBase* mb = _score->pos2measure(canvasPos(), &ntick, &stfi, 0, 0, 0);
      if (mb && mb->type() == MEASURE) {
            Measure* measure = (Measure*)mb;
            QPointF op = canvasPos();
            setTick(ntick);
            setTrack(stfi * VOICES);
            if (measure != parent()) {
                  ((Measure*)parent())->remove(this);
                  measure->add(this);
                  }
            setUserOff((op - layoutPos()) / _spatium);
            }
      else
            printf("Dynamic::endDrag(): measure not found\n");
      }

//---------------------------------------------------------
//   layoutPos
//    return layout position relative to canvas
//---------------------------------------------------------

QPointF Dynamic::layoutPos()
      {
      QPointF o(QPointF(_xoff, _yoff));
      if (_offsetType == OFFSET_SPATIUM)
            o *= _spatium;
      else
            o *= DPI;
      o += QPointF(_rxoff * parent()->width() * 0.01, _ryoff * parent()->height() * 0.01);

      double th = height();
      double tw = width();
      QPointF p;
      if (_align & ALIGN_BOTTOM)
            p.setY(-th);
      else if (_align & ALIGN_VCENTER)
            p.setY(-(th * .5));
      else if (_align & ALIGN_BASELINE)
            p.setY(-basePosition());
      if (_align & ALIGN_RIGHT)
            p.setX(-tw);
      else if (_align & ALIGN_HCENTER)
            p.setX(-(tw * .5));
      p += o;

      Measure* m = (Measure*) parent();
      Segment* seg = m->tick2segment(tick());
      double xp = seg->x();
      for (Element* e = parent(); e; e = e->parent())
            xp += e->x();
      System* system = measure()->system();
      double yp = system->staff(staffIdx())->y() + system->y();
      return p + QPointF(xp, yp);
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Dynamic::write(Xml& xml) const
      {
      xml.stag("Dynamic");
      if (subtype() == 0)
            xml.tag("data", QVariant(doc()->toHtml("utf8")));
      Element::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Dynamic::read(QDomElement e)
      {
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            if (e.tagName() == "data")
                  doc()->setHtml(e.text());
            else if (!Element::readProperties(e))
                  domError(e);
            }
      setSubtype(subtype());
      }

//---------------------------------------------------------
//   toSubtype
//---------------------------------------------------------

void Dynamic::setSubtype(const QString& tag)
      {
      int n = sizeof(dynList)/sizeof(*dynList);
      for (int i = 0; i < n; ++i) {
            if (dynList[i].tag == tag) {
                  setSubtype(i);
                  return;
                  }
            }
      setSubtype(0);
      setText(tag);
      }

//---------------------------------------------------------
//   subtypeName
//---------------------------------------------------------

const QString Dynamic::subtypeName() const
      {
      return dynList[subtype()].tag;
      }

//---------------------------------------------------------
//   canvasPos
//---------------------------------------------------------

QPointF Dynamic::canvasPos() const
      {
      if (!parent())
            return pos();
      double xp = x();
      for (Element* e = parent(); e; e = e->parent())
            xp += e->x();
      System* system = measure()->system();
      double yp = y() + system->staff(staffIdx())->y() + system->y();
      return QPointF(xp, yp);
      }

//---------------------------------------------------------
//   dragAnchor
//---------------------------------------------------------

QLineF Dynamic::dragAnchor() const
      {
      QPointF cp = canvasPos();
      QPointF anchor = cp - (userOff() * _spatium);
      return QLineF(cp, anchor);
      }

