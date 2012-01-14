//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: score.cpp 5149 2011-12-29 08:38:43Z wschweer $
//
//  Copyright (C) 2011-2012 Werner Schweer
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

extern Score* score;

//---------------------------------------------------------
//   testNote
//    read write test of note
//---------------------------------------------------------

bool testNote()
      {
      printf("====test note\n");

      bool passed = true;
      Chord* chord = new Chord(score);
      Note* note = new Note(score);
      chord->add(note);

   // pitch
      printf("  -pitch\n");
      note->setPitch(33);
      note->setTpcFromPitch();
      Note* n = static_cast<Note*>(writeReadElement(note));
      TEST(n->pitch() == 33);
      delete n;

   // tpc
      printf("  -tpc\n");
      note->setTpc(22);
      n = static_cast<Note*>(writeReadElement(note));
      TEST(n->tpc() == 22);
      delete n;

   // small
      printf("  -small\n");
      note->setSmall(true);
      n = static_cast<Note*>(writeReadElement(note));
      TEST(n->small());
      delete n;

   // mirror
      printf("  -mirror\n");
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
      printf("  -dotPosition\n");
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
      printf("  -onTimeUserOffset\n");
      note->setOnTimeUserOffset(12);
      n = static_cast<Note*>(writeReadElement(note));
      TEST(n->onTimeUserOffset() == 12);
      delete n;

  // offTimeUserOffset
      printf("  -offTimeUserOffset\n");
      note->setOffTimeUserOffset(21);
      n = static_cast<Note*>(writeReadElement(note));
      TEST(n->offTimeUserOffset() == 21);
      delete n;

  // headGroup
      printf("  -headGroup\n");
      for (int i = 0; i < HEAD_GROUPS; ++i) {
            note->setHeadGroup(i);
            n = static_cast<Note*>(writeReadElement(note));
            TEST(n->headGroup() == i);
            delete n;
            }

  // headType
      printf("  -headType\n");
      for (int i = 0; i < 5; ++i) {
            note->setHeadType(NoteHeadType(i));
            n = static_cast<Note*>(writeReadElement(note));
            TEST(n->headType() == i);
            delete n;
            }

   // velo offset
      printf("  -velo offset\n");
      note->setVeloOffset(71);
      n = static_cast<Note*>(writeReadElement(note));
      TEST(n->veloOffset() == 71);
      delete n;

   // tuning
      printf("  -tuning\n");
      note->setTuning(1.3);
      n = static_cast<Note*>(writeReadElement(note));
      TEST(n->tuning() == 1.3);
      delete n;

   // fret
      printf("  -fret\n");
      note->setFret(9);
      n = static_cast<Note*>(writeReadElement(note));
      TEST(n->fret() == 9);
      delete n;

   // string
      printf("  -string\n");
      note->setString(3);
      n = static_cast<Note*>(writeReadElement(note));
      TEST(n->string() == 3);
      delete n;

   // ghost
      printf("  -ghost\n");
      note->setGhost(true);
      n = static_cast<Note*>(writeReadElement(note));
      TEST(n->ghost());
      delete n;

   // velo type
      printf("  -veloType\n");
      note->setVeloType(USER_VAL);
      n = static_cast<Note*>(writeReadElement(note));
      TEST(n->veloType() == USER_VAL);
      delete n;

      note->setVeloType(OFFSET_VAL);
      n = static_cast<Note*>(writeReadElement(note));
      TEST(n->veloType() == OFFSET_VAL);
      delete n;

      //================================================
      //   test setProperty(int, QVariant)
      //================================================

      printf("  setProperty\n");
   // pitch
      note->setProperty(P_PITCH, 32);
      n = static_cast<Note*>(writeReadElement(note));
      TEST(n->pitch() == 32);
      delete n;

   // tpc
      note->setProperty(P_TPC, 21);
      n = static_cast<Note*>(writeReadElement(note));
      TEST(n->tpc() == 21);
      delete n;

   // small
      note->setProperty(P_SMALL, false);
      n = static_cast<Note*>(writeReadElement(note));
      TEST(!n->small());
      delete n;

      note->setProperty(P_SMALL, true);
      n = static_cast<Note*>(writeReadElement(note));
      TEST(n->small());
      delete n;

   // mirror
      note->setProperty(P_MIRROR_HEAD, int(DH_LEFT));
      n = static_cast<Note*>(writeReadElement(note));
      TEST(n->userMirror() == DH_LEFT);
      delete n;

      note->setProperty(P_MIRROR_HEAD, int(DH_RIGHT));
      n = static_cast<Note*>(writeReadElement(note));
      TEST(n->userMirror() == DH_RIGHT);
      delete n;

      note->setProperty(P_MIRROR_HEAD, DH_AUTO);
      n = static_cast<Note*>(writeReadElement(note));
      TEST(n->userMirror() == DH_AUTO);
      delete n;

   // dot position
      note->setProperty(P_DOT_POSITION, UP);
      n = static_cast<Note*>(writeReadElement(note));
      TEST(n->dotPosition() == UP);
      delete n;

      note->setProperty(P_DOT_POSITION, DOWN);
      n = static_cast<Note*>(writeReadElement(note));
      TEST(n->dotPosition() == DOWN);
      delete n;

      note->setProperty(P_DOT_POSITION, AUTO);
      n = static_cast<Note*>(writeReadElement(note));
      TEST(n->dotPosition() == AUTO);
      delete n;

  // onTimeUserOffset
      note->setProperty(P_ONTIME_OFFSET, 9);
      n = static_cast<Note*>(writeReadElement(note));
      TEST(n->onTimeUserOffset() == 9);
      delete n;

  // offTimeUserOffset
      note->setProperty(P_OFFTIME_OFFSET, 19);
      n = static_cast<Note*>(writeReadElement(note));
      TEST(n->offTimeUserOffset() == 19);
      delete n;

  // headGroup
      for (int i = 0; i < HEAD_GROUPS; ++i) {
            note->setProperty(P_HEAD_GROUP, int(i));
            n = static_cast<Note*>(writeReadElement(note));
            TEST(n->headGroup() == i);
            delete n;
            }

  // headType
      for (int i = 0; i < 5; ++i) {
            note->setProperty(P_HEAD_TYPE, i);
            n = static_cast<Note*>(writeReadElement(note));
            TEST(n->headType() == i);
            delete n;
            }

   // velo offset
      note->setProperty(P_VELO_OFFSET, 38);
      n = static_cast<Note*>(writeReadElement(note));
      TEST(n->veloOffset() == 38);
      delete n;

   // tuning
      note->setProperty(P_TUNING, 2.4);
      n = static_cast<Note*>(writeReadElement(note));
      TEST(n->tuning() == 2.4);
      delete n;

   // fret
      note->setProperty(P_FRET, 7);
      n = static_cast<Note*>(writeReadElement(note));
      TEST(n->fret() == 7);
      delete n;

   // string
      note->setProperty(P_STRING, 4);
      n = static_cast<Note*>(writeReadElement(note));
      TEST(n->string() == 4);
      delete n;

   // ghost
      note->setProperty(P_GHOST, false);
      n = static_cast<Note*>(writeReadElement(note));
      TEST(!n->ghost());
      delete n;

      note->setProperty(P_GHOST, true);
      n = static_cast<Note*>(writeReadElement(note));
      TEST(n->ghost());
      delete n;

   // velo type
      note->setProperty(P_VELO_TYPE, USER_VAL);
      n = static_cast<Note*>(writeReadElement(note));
      TEST(n->veloType() == USER_VAL);
      delete n;

      note->setProperty(P_VELO_TYPE, OFFSET_VAL);
      n = static_cast<Note*>(writeReadElement(note));
      TEST(n->veloType() == OFFSET_VAL);
      delete n;

      return passed;
      }

