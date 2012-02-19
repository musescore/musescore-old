//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
//
//  Copyright (C) 2012 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "libmscore/score.h"
#include "libmscore/note.h"
#include "libmscore/chord.h"
#include "mtest.h"
#include "testutils.h"
#include "testutils.h"

extern Score* score;

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

