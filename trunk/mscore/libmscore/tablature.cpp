//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "tablature.h"
#include "chord.h"
#include "note.h"
#include "score.h"
#include "undo.h"

static int guitarStrings[6] = { 40, 45, 50, 55, 59, 64 };

Tablature guitarTablature(13, 6, guitarStrings);

//---------------------------------------------------------
//   Tablature
//---------------------------------------------------------

bool Tablature::bFretting = false;

Tablature::Tablature(int numFrets, int numStrings, int strings[])
      {
      _frets = numFrets;

      for (int i = 0; i < numStrings; i++)
            stringTable.append(strings[i]);
      }

Tablature::Tablature(int numFrets, QList<int>& strings)
      {
      _frets = numFrets;

      foreach(int i, strings)
            stringTable.append( i);
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Tablature::read(const QDomElement& de)
      {

      for (QDomElement e = de.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            const QString& tag(e.tagName());
            if (tag == "frets")
                  _frets = e.text().toInt();
            else if (tag == "string")
                  stringTable.append(e.text().toInt());
            else
                  domError(e);
            }
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Tablature::write(Xml& xml) const
      {
      xml.stag("Tablature");
      xml.tag("frets", _frets);
      foreach(int pitch, stringTable)
            xml.tag("string", pitch);
      xml.etag();
      }

//---------------------------------------------------------
//   convertPitch
//   Finds string and fret for a note.
//
//   Fills *string and *fret with suitable values for pitch
//   using the highest possible string.
//   If note cannot be fretted, uses fret 0 on nearest string and returns false
//
//    Note: Strings are stored internally from lowest (0) to highest (strings()-1),
//          but the returned *string value references strings in reversed, 'visual', order:
//          from highest (0) to lowest (strings()-1)
//---------------------------------------------------------

bool Tablature::convertPitch(int pitch, int* string, int* fret) const
      {
      int strings = stringTable.size();

      // if above max fret on highest string, fret on first string, but return failure
      if(pitch > stringTable.at(strings-1) + _frets) {
            *string = 0;
            *fret   = 0;
            return false;
            }

      // look for a suitable string, starting from the highest
      for (int i = strings-1; i >=0; i--) {
            if(pitch >= stringTable.at(i)) {
                  *string = strings - i - 1;
                  *fret   = pitch - stringTable.at(i);
                  return true;
                  }
            }

      // if no string found, pitch is below lowest string:
      // fret on last string, but return failure
      *string = strings-1;
      *fret   = 0;
      return false;
      }

//---------------------------------------------------------
//   getPitch
//   Returns the pitch corresponding to the string / fret combination
//---------------------------------------------------------

int Tablature::getPitch(int string, int fret) const
      {
      int strings = stringTable.size();
      return stringTable[strings - string - 1] + fret;
      }

//---------------------------------------------------------
//   fret
//    Returns the fret corresponding to the pitch / string combination
//    returns -1 if not possible
//---------------------------------------------------------

int Tablature::fret(int pitch, int string) const
      {
      int strings = stringTable.size();

      if (string < 0 || string >= strings)
            return -1;
      int fret = pitch - stringTable[strings - string - 1];
      if (fret < 0 || fret >= _frets)
            return -1;
      return fret;
      }

//---------------------------------------------------------
//   fretChord
//    Assigns fretting to all the notes of the chord,
//    re-using existing fretting wherever possible
//
//    Minimizes fret conflicts (multiple notes on the same string)
//    but marks as fretConflict notes which cannot be fretted
//    (outside tablature range) or which cannot be assigned
//    a separate string
//---------------------------------------------------------

void Tablature::fretChord(Chord * chord) const
      {
      int nFret, nNewFret, nTempFret;
      int nString, nNewString, nTempString;

      if(bFretting)
            return;
      bFretting = true;

      // we need to keep track of each string we allocate ourselves within this algorithm
      bool bUsed[strings()];                    // initially all strings are available
      for(nString=0; nString<strings(); nString++)
            bUsed[nString] = false;
      // we also need the notes sorted in order of string (from highest to lowest) and then pitch
      QMap<int, Note *> sortedNotes;
      foreach(Note * note, chord->notes())
            sortedNotes.insert(note->string()*1000 - note->pitch(), note);

      // scan chord notes from highest, matching with strings from the highest
      foreach(Note * note, sortedNotes) {
            nString = nNewString    = note->string();
            nFret   = nNewFret      = note->fret();
            note->setFretConflict(false);       // assume no conflicts on this note
            // if no fretting yet or current fretting is no longer valid
            if (nString == -1 || nFret == -1 || getPitch(nString, nFret) != note->pitch()) {
                  // get a new fretting
                  if(!convertPitch(note->pitch(), &nNewString, &nNewFret) ) {
                        // no way to fit this note in this tab:
                        // mark as fretting conflict
                        note->setFretConflict(true);
//                        // store pitch change without affecting chord context
//                        chord->score()->undo(new ChangePitch(note, note->pitch(), note->tpc(),
//                           note->line(), nNewFret, nNewString));
                        continue;
                        }

                  // check this note is not using the same string of another note of this chord
                  foreach(Note * note2, sortedNotes) {
                        // if same string...
                        if(note2 != note && note2->string() == nNewString) {
                              // ...attempt to fret this note on its old string
                              if( (nTempFret=fret(note->pitch(), nString)) != -1) {
                                    nNewFret   = nTempFret;
                                    nNewString = nString;
                                    }
                              break;
                              }
                        }
                  }

            // check we are not reusing a string we already used
            if(bUsed[nNewString]) {
                  // ...try with each other string, from the highest
                  for(nTempString=0; nTempString < strings(); nTempString++) {
                        if(bUsed[nTempString])
                              continue;
                        if( (nTempFret=fret(note->pitch(), nTempString)) != -1) {
                              // suitable string found
                              nNewFret    = nTempFret;
                              nNewString  = nTempString;
                              break;
                              }
                        }
                  // if we run out of strings
                  if(nTempString >= strings()) {
                        // no way to fit this chord in this tab:
                        // mark as fretting conflict this note...
                        note->setFretConflict(true);
//                        // and any note already scanned and set on the same string
//                        for(nCount2=chord->notes().size()-1; nCount2 > nCount; nCount2--) {
//                              note2 = chord->notes().at(nCount2);
//                              if(note2->string() == nNewString)
//                                    note2->setFretConflict(true);
//                              }
                        continue;
                        }
                  }

            // if fretting did change, store as a fret change
            if(nString != nNewString || nFret != nNewFret)
                  chord->score()->undo(new ChangeFret(note, nNewFret, nNewString));

            bUsed[nNewString] = true;           // string is used
            }
      bFretting = false;
      }

