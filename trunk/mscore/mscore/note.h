//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: note.h,v 1.45 2006/03/03 16:20:42 wschweer Exp $
//
//  Copyright (C) 2002-2008 Werner Schweer and others
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

#ifndef __NOTE_H__
#define __NOTE_H__

/**
 \file
 Definition of classes Note and ShadowNote.
*/

#include "globals.h"
#include "element.h"
#include "symbol.h"
#include "accidental.h"
#include "durationtype.h"

class Tie;
class Chord;
class Text;
class Score;
class Sym;
class Viewer;

//
//    note head groups
//
enum {
      HEAD_NORMAL, HEAD_CROSS, HEAD_DIAMOND, HEAD_TRIANGLE, HEAD_DIAMOND2,
      HEAD_SLASH, HEAD_XCIRCLE,
      HEAD_GROUPS
      };

//---------------------------------------------------------
//   NoteHead
//---------------------------------------------------------

class NoteHead : public Symbol {

   public:
      NoteHead(Score*);
      NoteHead &operator=(const NoteHead&);
      virtual NoteHead* clone() const  { return new NoteHead(*this); }
      virtual ElementType type() const { return NOTEHEAD; }
      virtual void write(Xml& xml) const;
      };

//---------------------------------------------------------
//   Note
//---------------------------------------------------------

/**
 Graphic representation of a note.
*/

class Note : public Element {
      Q_DECLARE_TR_FUNCTIONS(Note)

//      static int noteHeads[HEAD_GROUPS][4];
      static int smallNoteHeads[HEAD_GROUPS][4];

      int _subchannel;        ///< articulation

      int _pitch;             ///< Note pitch as midi value (0 - 127).
      int _tpc;               ///< tonal pitch class

      int _line;              ///< y-Position; 0 - top line.
      int _staffMove;         ///< -1, 0, +1, used for crossbeaming

      int _userAccidental;    ///< editorial accidental type (0-15)
      Accidental* _accidental;

      int _head;              ///< note head symbol number
      int _headGroup;
      bool _mirror;           ///< True if note is mirrored at stem.
      ElementList _el;        ///< fingering, other text, symbols or images

      Tie* _tieFor;
      Tie* _tieBack;

      int _lineOffset;        ///< Used during mouse dragging.
      bool _hidden;           ///< markes this note as the hidden one if there are
                              ///< overlapping notes; hidden notes are not played
                              ///< and heads are not shown

      virtual bool isMovable() const { return true; }
      virtual QRectF drag(const QPointF& s);
      virtual void endDrag();

   public:
      Note(Score*);
      Note(const Note&);
      Note &operator=(const Note&);
      ~Note();
      virtual Note* clone() const      { return new Note(*this); }
      virtual ElementType type() const { return NOTE; }
      virtual QRectF bbox() const;
      virtual QPointF canvasPos() const;      ///< position in canvas coordinates
      virtual void layout(ScoreLayout*);
      virtual void collectElements(QList<const Element*>& el) const;
      virtual void setTrack(int val);

      int totalTicks() const;
      void setType(Duration);

      double headWidth() const;
      double headHeight() const;
      int noteHead() const            { return _head;  }
      int headGroup() const           { return _headGroup; }
      void setHeadGroup(int val);

      int pitch() const               { return _pitch; }
      void setPitch(int val);
      int tpc() const                 { return _tpc; }
      void setTpc(int v);
      void changePitch(int val);

      int userAccidental() const      { return _userAccidental; }

      int accidentalSubtype() const   { return _accidental ? _accidental->subtype() : ACC_NONE; }
      Accidental* accidental() const  { return _accidental; }
      void setAccidentalSubtype(int);
      void changeAccidental(int);

      int line() const                { return _line + _lineOffset;   }
      void setLine(int n)             { _line = n;      }

      virtual void add(Element*);
      virtual void remove(Element*);

      bool mirror() const             { return _mirror; }
      void setMirror(bool val)        { _mirror = val;  }

      Tie* tieFor() const             { return _tieFor;     }
      Tie* tieBack() const            { return _tieBack;    }
      void setTieFor(Tie* t)          { _tieFor = t;        }
      void setTieBack(Tie* t);

      Chord* chord() const            { return (Chord*)parent(); }
      void setChord(Chord* a)         { setParent((Element*)a);    }

      virtual void draw(QPainter&) const;
      virtual void read(QDomElement);
      void write(Xml& xml, bool clipboardmode, int start, int end) const;
      virtual void write(Xml& xml) const { write(xml, false, 0, 0); }
      QPointF stemPos(bool upFlag) const;    ///< Point to connect stem.

      virtual bool acceptDrop(Viewer*, const QPointF&, int, int) const;
      virtual Element* drop(const QPointF&, const QPointF&, Element*);

      virtual bool genPropertyMenu(QMenu*) const;
      virtual void propertyAction(const QString&);

      bool isSimple(Xml&) const;

      bool hidden() const           { return _hidden; }
      void setHidden(bool val)      { _hidden = val;  }

      NoteType noteType() const;
      int staffMove() const         { return _staffMove; }
      void setStaffMove(int val)    { _staffMove = val; }

      ElementList* el()             { return &_el; }
      const ElementList* el() const { return &_el; }

      int subchannel() const        { return _subchannel; }
      void setSubchannel(int val)   { _subchannel = val;  }

      virtual void setTickLen(int n) { printf("Note: setTickLen %d\n", n); }
      };

//---------------------------------------------------------
//   ShadowNote
//---------------------------------------------------------

/**
 Graphic representation of a shadow note,
 which shows the note insert position in note entry mode.
*/

class ShadowNote : public Element {
      int _line;
      int _headGroup;

   public:
      ShadowNote(Score*);
      virtual ShadowNote* clone() const { return new ShadowNote(*this); }
      virtual ElementType type() const  { return SHADOW_NOTE; }
      int line() const                  { return _line;   }
      void setLine(int n)               { _line = n;      }
      virtual QRectF bbox() const;
      virtual void draw(QPainter& p) const;
      int headGroup() const           { return _headGroup; }
      void setHeadGroup(int val)      { _headGroup = val;  }
      };

extern const int noteHeads[HEAD_GROUPS][4];

#endif

