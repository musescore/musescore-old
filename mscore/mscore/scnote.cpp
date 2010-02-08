//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2009 Werner Schweer and others
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

#include "mscore.h"
#include "scnote.h"
#include "note.h"
#include "utils.h"
#include "scscore.h"
#include "undo.h"

//---------------------------------------------------------
//   thisNote
//---------------------------------------------------------

Note* ScNotePrototype::thisNote() const
      {
      NotePtr* np = qscriptvalue_cast<NotePtr*>(thisObject());
printf("===========ScNotePrototype::thisNote %p\n", np);
      if (np)
            return *np;
      return 0;
      }

//---------------------------------------------------------
//   noteName
//---------------------------------------------------------

QString ScNotePrototype::getName() const
      {
      return pitch2string(thisNote()->pitch());
      }

//---------------------------------------------------------
//   getPitch
//---------------------------------------------------------

int ScNotePrototype::getPitch() const
      {
      return thisNote()->pitch();
      }

//---------------------------------------------------------
//   setPitch
//---------------------------------------------------------

void ScNotePrototype::setPitch(int v)
      {
printf("ScNotePrototype::setPitch %d\n", v);

      thisNote()->setPitch(v);
      thisNote()->setTpcFromPitch();
      }

//---------------------------------------------------------
//   getTuning
//---------------------------------------------------------

double ScNotePrototype::getTuning() const
      {
      return thisNote()->tuning();
      }

//---------------------------------------------------------
//   setTuning
//---------------------------------------------------------

void ScNotePrototype::setTuning(double v)
      {
      thisNote()->setTuning(v);
      }

//---------------------------------------------------------
//   getColor
//---------------------------------------------------------

QColor ScNotePrototype::getColor() const
      {
      return thisNote()->color();
      }

//---------------------------------------------------------
//   setColor
//---------------------------------------------------------

void ScNotePrototype::setColor(const QColor& c)
      {
      Note* note = thisNote();
      Score* score = note->score();
printf("Note: setColor score %p %d %d %d\n", score, c.red(), c.green(), c.blue());
      if (score)
            score->undo()->push(new ChangeColor(note, c));
      else
            note->setColor(c);
      }




