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

void writeReadElement(Element** pElement)
      {
      Element* element = *pElement;
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
            return;
            }
      docName = "--";
      QDomElement e = doc.documentElement();
      QString tag(e.tagName());
      element = Element::name2Element(e.tagName(), score);
      element->read(e);
      *pElement = element;
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
      writeReadElement((Element**)&note);
      TEST(note->pitch() == 33);

      //...

      return passed;
      }

