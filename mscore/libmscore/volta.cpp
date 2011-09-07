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

#include "volta.h"
#include "style.h"
#include "xml.h"
#include "score.h"
#include "text.h"

//---------------------------------------------------------
//   Volta
//---------------------------------------------------------

Volta::Volta(Score* s)
   : TextLine(s)
      {
      setBeginText("1.", TEXT_STYLE_VOLTA);

      setBeginTextPlace(PLACE_BELOW);
      setContinueTextPlace(PLACE_BELOW);

      setBeginHook(true);
      Spatium hook(s->styleS(ST_voltaHook));
      setBeginHookHeight(hook);
      setYoff(s->styleS(ST_voltaY).val());
      setEndHookHeight(hook);
      setAnchor(ANCHOR_MEASURE);
      }

//---------------------------------------------------------
//   setSubtype
//---------------------------------------------------------

void Volta::setSubtype(int val)
      {
      Element::setSubtype(val);
      switch(val) {
            case Volta::VOLTA_OPEN:
                  setEndHook(false);
                  break;
            case Volta::VOLTA_CLOSED:
                  setEndHook(true);
                  break;
            }
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Volta::layout()
      {
      setLineWidth(score()->styleS(ST_voltaLineWidth));
      Spatium hook(score()->styleS(ST_voltaHook));
      setBeginHookHeight(hook);
      setYoff(score()->styleS(ST_voltaY).val());
      setEndHookHeight(hook);
      TextLine::layout();
      }

//---------------------------------------------------------
//   setText
//---------------------------------------------------------

void Volta::setText(const QString& s)
      {
      setBeginText(s, TEXT_STYLE_VOLTA);
      foreach(SpannerSegment* seg, spannerSegments())
            static_cast<VoltaSegment*>(seg)->clearText();
      }

//---------------------------------------------------------
//   text
//---------------------------------------------------------

QString Volta::text() const
      {
      return beginText()->getText();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Volta::read(QDomElement e)
      {
      foreach(SpannerSegment* seg, spannerSegments())
            delete seg;
      spannerSegments().clear();
      setId(e.attribute("id", "-1").toInt());
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            if (tag == "text")            // obsolete
                  setText(e.text());
            else if (tag == "endings") {
                  QString s = e.text();
                  QStringList sl = s.split(",", QString::SkipEmptyParts);
                  _endings.clear();
                  foreach(const QString& l, sl) {
                        int i = l.simplified().toInt();
                        _endings.append(i);
                        }
                  }
            else if (!TextLine::readProperties(e))
                  domError(e);
            }
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Volta::write(Xml& xml) const
      {
      Volta proto(score());
      proto.setSubtype(subtype());

      xml.stag(QString("%1 id=\"%2\"").arg(name()).arg(id()));
      TextLine::writeProperties(xml, &proto);
      QString s;
      foreach(int i, _endings) {
            if (!s.isEmpty())
                  s += ", ";
            s += QString("%1").arg(i);
            }
      xml.tag("endings", s);
      xml.etag();
      }

//---------------------------------------------------------
//   createLineSegment
//---------------------------------------------------------

LineSegment* Volta::createLineSegment()
      {
      return new VoltaSegment(score());
      }

//---------------------------------------------------------
//   hasEnding
//---------------------------------------------------------

bool Volta::hasEnding(int repeat) const
      {
      foreach(int ending, endings()) {
            if (ending == repeat)
                  return true;
            }
      return false;
      }

