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

#include "score.h"
#include "tempotext.h"
#include "al/tempo.h"
#include "system.h"
#include "measure.h"

//---------------------------------------------------------
//   TempoText
//---------------------------------------------------------

TempoText::TempoText(Score* s)
   : Text(s)
      {
      _tempo = 2.0;
      _followText = false;
      setSubtype(TEXT_TEMPO);
      setTextStyle(TEXT_STYLE_TEMPO);
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void TempoText::write(Xml& xml) const
      {
      xml.stag("Tempo");
      xml.tag("tempo", _tempo);
      if (_followText)
            xml.tag("followText", _followText);
      Text::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void TempoText::read(QDomElement e)
      {
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            if (tag == "tempo"){
                  _tempo = e.text().toDouble();
                  //Don't need to add to tempo since tempo map is read at the beginning of the file.
                  }
            else if (tag == "followText")
                  _followText = e.text().toInt();
            else if (!Text::readProperties(e))
                  domError(e);
            }
      if (score()->mscVersion() < 119) {
            //
            // Reset text in old version to
            // style.
            //
            if (_textStyle != TEXT_STYLE_INVALID) {
                  setStyled(true);
                  styleChanged();
                  }
            }
      }

//---------------------------------------------------------
//   TempoPattern
//---------------------------------------------------------

struct TempoPattern {
      const char* pattern;
      double f;
      TempoPattern(const char* s, double v) : pattern(s), f(v) {}
      };

//---------------------------------------------------------
//   textChanged
//    text may have changed
//---------------------------------------------------------

void TempoText::textChanged()
      {
      if (!_followText)
            return;
      QString s = getText();

      static const TempoPattern tp[] = {
            TempoPattern("\\xd834\\xdd5f = (\\d+)", 1.0/60.0),      // 1/4
            TempoPattern("\\xd834\\xdd5e = (\\d+)", 1.0/30.0),      // 1/2
            TempoPattern("\\xd834\\xdd60 = (\\d+)", 1.0/120.0),     // 1/8
            TempoPattern("\\xd834\\xdd5f\\xd834\\xdd6d = (\\d+)", 1.5/60.0),   // dotted 1/4
            TempoPattern("\\xd834\\xdd5e\\xd834\\xdd6d = (\\d+)", 1.5/30.0),   // dotted 1/2
            TempoPattern("\\xd834\\xdd60\\xd834\\xdd6d = (\\d+)", 1.5/120.0),  // dotted 1/8
            };

      for (unsigned i = 0; i < sizeof(tp)/sizeof(*tp); ++i) {
            QRegExp re(tp[i].pattern);      // 1/4
            if (re.indexIn(s) != -1) {
                  QStringList sl = re.capturedTexts();
                  if (sl.size() == 2) {
                        _tempo = double(sl[1].toInt()) * tp[i].f;
                        break;
                        }
                  }
            }
      }

