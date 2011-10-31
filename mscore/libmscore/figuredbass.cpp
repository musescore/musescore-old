//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: lyrics.cpp 4904 2011-10-28 10:30:30Z wschweer $
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "figuredbass.h"
#include "score.h"
#include "note.h"
#include "measure.h"
#include "system.h"

//---------------------------------------------------------
//   FiguredBass
//---------------------------------------------------------

FiguredBass::FiguredBass(Score* s)
   : Lyrics(s)
      {
      setSubtype(TEXT_FIGURED_BASS);
      setTextStyle(TEXT_STYLE_FIGURED_BASS);
      }

FiguredBass::FiguredBass(const FiguredBass& l)
   : Lyrics(l)
      {
      }

//---------------------------------------------------------
//   FiguredBass
//---------------------------------------------------------

FiguredBass::~FiguredBass()
      {
      }

//---------------------------------------------------------
//   addFiguredBass
//    called from Keyboard Accelerator & menue
//---------------------------------------------------------

FiguredBass* Score::addFiguredBass()
      {
      Element* el = selection().element();
      if (el == 0 || (el->type() != NOTE && el->type() != FIGURED_BASS)) {
            QMessageBox::information(0,
               QMessageBox::tr("MuseScore:"),
               QMessageBox::tr("No note or figured bass selected:\n"
                  "Please select a single note or figured bass and retry operation\n"),
               QMessageBox::Ok, QMessageBox::NoButton);
            return 0;
            }
      ChordRest* cr;
      if (el->type() == NOTE)
            cr = static_cast<Note*>(el)->chord();
      else if (el->type() == FIGURED_BASS)
            cr = static_cast<FiguredBass*>(el)->chordRest();
      else
            return 0;

      QList<Lyrics*> ll = cr->lyricsList();
      int no = ll.size();
      FiguredBass* fb = new FiguredBass(this);
      fb->setTrack(cr->track());
      fb->setParent(cr);
      fb->setNo(no);
      undoAddElement(fb);

      select(fb, SELECT_SINGLE, 0);
      return fb;
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void FiguredBass::write(Xml& xml) const
      {
      xml.stag("FiguredBass");
      if (_no)
            xml.tag("no", _no);
      Text::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void FiguredBass::read(QDomElement e)
      {
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            const QString& tag(e.tagName());
            if (tag == "no")
                  _no = e.text().toInt();
            else if (!Text::readProperties(e))
                  domError(e);
            }
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void FiguredBass::layout()
      {
      if (!styled())
            _textStyle = TEXT_STYLE_FIGURED_BASS;
      Text::layout();
      qreal lh = lineSpacing() * score()->styleD(ST_figuredBassLineHeight);

      System* sys = measure()->system();
      if (sys == 0) {
            qDebug("lyrics layout: no system!");
            abort();
            }
      const QList<Lyrics*>* ll = &(chordRest()->lyricsList());

      int line = ll->indexOf(this);
      qreal y  = lh * line + point(score()->styleS(ST_figuredBassDistance))
                 + sys->staff(staffIdx())->bbox().height();
      QString s = getText();
      qreal x = 0.0;
      if (s.size() > 1) {
            for (int i = 0; i < s.size(); ++i) {
                  if (s[i].isNumber()) {
                        x = -QFontMetricsF(font()).width(s.left(i));
                        break;
                        }
                  }
            }
printf("layout x %f\n", x);
      setPos(x, y);
      }


