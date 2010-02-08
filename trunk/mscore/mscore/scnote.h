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

#ifndef __SCNOTE_H__
#define __SCNOTE_H__

class Note;
class Score;
typedef Note* NotePtr;

//---------------------------------------------------------
//   ScNotePrototype
//---------------------------------------------------------

class ScNotePrototype : public QObject, public QScriptable
      {
      Q_OBJECT
      Q_PROPERTY(QString name   READ getName)
      Q_PROPERTY(int     pitch  READ getPitch  WRITE setPitch  SCRIPTABLE true)
      Q_PROPERTY(double  tuning READ getTuning WRITE setTuning SCRIPTABLE true)
      Q_PROPERTY(QColor  color  READ getColor  WRITE setColor  SCRIPTABLE true)

      Note* thisNote() const;

   public slots:
      void setColor(const QColor& c);
      void setTuning(double v);
      void setPitch(int v);

      QString getName() const;
      int     getPitch() const;
      double  getTuning() const;
      QColor  getColor() const;

   public:
      ScNotePrototype(QObject *parent = 0) : QObject(parent) {}
      ~ScNotePrototype() {}
      };

Q_DECLARE_METATYPE(NotePtr)
Q_DECLARE_METATYPE(NotePtr*)

#endif


