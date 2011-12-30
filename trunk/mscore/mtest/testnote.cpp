//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: score.cpp 5149 2011-12-29 08:38:43Z wschweer $
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "libmscore/score.h"
#include "libmscore/note.h"
#include "libmscore/chord.h"

extern Score* score;

#define TEST(a) if (!(a)) \
      printf("Test failed in <%s>: " #a "\n", __func__), passed = false;

//---------------------------------------------------------
//   writeReadElement
//    writes and element and reads it back
//---------------------------------------------------------

Element* writeReadElement(Element* element)
      {
      //
      // write element
      //
      QBuffer buffer;
      buffer.open(QIODevice::WriteOnly);
      Xml xml(&buffer);
      xml.header();
      element->write(xml);
      buffer.close();

      //
      // read element
      //
//      printf("%s\n", buffer.buffer().data());

      QDomDocument doc;
      int line, column;
      QString err;
      if (!doc.setContent(buffer.buffer(), &err, &line, &column)) {
            printf("writeReadElement: error reading paste data at line %d column %d: %s\n",
               line, column, qPrintable(err));
            printf("%s\n", buffer.buffer().data());
            return 0;
            }
      docName = "--";
      QDomElement e = doc.documentElement();
      QString tag(e.tagName());
      element = Element::name2Element(e.tagName(), score);
      element->read(e);
      return element;
      }

//---------------------------------------------------------
//   testNote
//    read write test of note
//---------------------------------------------------------

bool testNote()
      {
      bool passed = true;
      Chord* chord = new Chord(score);
      Note* note = new Note(score);
      chord->add(note);

   // pitch
      note->setPitch(33);
      Note* n = static_cast<Note*>(writeReadElement(note));
      TEST(n->pitch() == 33);
      delete n;

   // tpc
      note->setTpc(22);
      n = static_cast<Note*>(writeReadElement(note));
      TEST(n->tpc() == 22);
      delete n;

   // small
      note->setSmall(true);
      n = static_cast<Note*>(writeReadElement(note));
      TEST(n->small());
      delete n;

   // mirror
      note->setUserMirror(DH_LEFT);
      n = static_cast<Note*>(writeReadElement(note));
      TEST(n->userMirror() == DH_LEFT);
      delete n;

      note->setUserMirror(DH_RIGHT);
      n = static_cast<Note*>(writeReadElement(note));
      TEST(n->userMirror() == DH_RIGHT);
      delete n;

      note->setUserMirror(DH_AUTO);
      n = static_cast<Note*>(writeReadElement(note));
      TEST(n->userMirror() == DH_AUTO);
      delete n;

   // dot position
      note->setDotPosition(UP);
      n = static_cast<Note*>(writeReadElement(note));
      TEST(n->dotPosition() == UP);
      delete n;

      note->setDotPosition(DOWN);
      n = static_cast<Note*>(writeReadElement(note));
      TEST(n->dotPosition() == DOWN);
      delete n;

      note->setDotPosition(AUTO);
      n = static_cast<Note*>(writeReadElement(note));
      TEST(n->dotPosition() == AUTO);
      delete n;

  // onTimeUserOffset
      note->setOnTimeUserOffset(12);
      n = static_cast<Note*>(writeReadElement(note));
      TEST(n->onTimeUserOffset() == 12);
      delete n;

  // offTimeUserOffset
      note->setOffTimeUserOffset(21);
      n = static_cast<Note*>(writeReadElement(note));
      TEST(n->offTimeUserOffset() == 21);
      delete n;

  // headGroup
      for (int i = 0; i < HEAD_GROUPS; ++i) {
            note->setHeadGroup(i);
            n = static_cast<Note*>(writeReadElement(note));
            TEST(n->headGroup() == i);
            delete n;
            }

  // headType
      for (int i = 0; i < 5; ++i) {
            note->setHeadType(NoteHeadType(i));
            n = static_cast<Note*>(writeReadElement(note));
            TEST(n->headType() == i);
            delete n;
            }

   // velo offset
      note->setVeloOffset(71);
      n = static_cast<Note*>(writeReadElement(note));
      TEST(n->veloOffset() == 71);
      delete n;

   // tuning
      note->setTuning(1.3);
      n = static_cast<Note*>(writeReadElement(note));
      TEST(n->tuning() == 1.3);
      delete n;

   // fret
      note->setFret(9);
      n = static_cast<Note*>(writeReadElement(note));
      TEST(n->fret() == 9);
      delete n;

   // string
      note->setString(3);
      n = static_cast<Note*>(writeReadElement(note));
      TEST(n->string() == 3);
      delete n;

   // ghost
      note->setGhost(true);
      n = static_cast<Note*>(writeReadElement(note));
      TEST(n->ghost());
      delete n;

   // velo type
      note->setVeloType(USER_VAL);
      n = static_cast<Note*>(writeReadElement(note));
      TEST(n->veloType() == USER_VAL);
      delete n;

      note->setVeloType(OFFSET_VAL);
      n = static_cast<Note*>(writeReadElement(note));
      TEST(n->veloType() == OFFSET_VAL);
      delete n;

      note->setVeloType(AUTO_VAL);
      n = static_cast<Note*>(writeReadElement(note));
      TEST(n->veloType() == AUTO_VAL);
      delete n;

      return passed;
      }

