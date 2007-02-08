//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: note.h,v 1.45 2006/03/03 16:20:42 wschweer Exp $
//
//  Copyright (C) 2002-2006 Werner Schweer (ws@seh.de)
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
#include "accidental.h"

class Tie;
class Chord;
class Text;
class Score;
class Painter;
class Sym;

//---------------------------------------------------------
//   Note
//---------------------------------------------------------

/**
 Graphic representation of a note.
*/

class Note : public Element {
      int _pitch;             ///< Note pitch as midi value (0 - 127).
      int _userAccidental;    ///< -1 - automatic accidental.
      int _line;              ///< y-Position; 0 - top line.
      int _move;              ///< -1, 0, +1.
      Accidental* _accidental;

      int _head;              ///< Note head.
      DurationType _durationType;
      bool _grace;
      bool _mirror;           ///< True if note is mirrored at stem.
      int _dots;
      QList<Text*> _fingering;

      Tie* _tieFor;
      Tie* _tieBack;

      int _lineOffset;  ///< Used during mouse dragging.

      virtual bool isMovable() const { return true; }
      virtual QRectF drag(const QPointF& s);
      virtual void endDrag();

   public:
      Note(Score*);
      Note(Score*, int pitch, bool grace = false);
      Note &operator=(const Note&);
      ~Note();
      virtual Note* clone() const { return new Note(*this); }
      virtual ElementType type() const { return NOTE; }

      virtual QRectF bbox() const;
      bool grace() const            { return _grace; }
      void setGrace(bool val)       { _grace = val;  }

      void setHead(int);
      int totalTicks() const;
      void setType(DurationType);
      void setDots(int n)           { _dots = n; }
      int dots() const              { return _dots; }

      double headWidth() const;
      int noteHead()                { return _head;  }

      int pitch() const             { return _pitch; }
      void setPitch(int val)        { _pitch = val; }
      void changePitch(int val);
      int move() const              { return _move; }
      void setMove(int val)         { _move = val; }

      int userAccidental() const      { return _userAccidental; }
      void setUserAccidental(int i)   { _userAccidental = i; }
      int accidentalIdx() const       { return _accidental ? _accidental->subtype() : ACC_NONE; }
      Accidental* accidental() const  { return _accidental; }
      void setAccidental(int);
      void changeAccidental(int);

      int line() const                { return _line + _lineOffset;   }
      void setLine(int n)             { _line = n;      }

      QList<Text*>& fingering()       { return _fingering; }

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

      virtual void draw1(Painter&);
      virtual void read(QDomNode);
      virtual void write(Xml& xml) const;
      QPointF stemPos(bool upFlag) const;    ///< Point to connect stem.
      Element* findSelectableElement(QPointF p) const;
      virtual bool acceptDrop(const QPointF&, int, const QDomNode&) const;
      virtual void drop(const QPointF&, int, const QDomNode&);

      virtual bool startEdit(QMatrix&, const QPointF&);
      virtual bool edit(QKeyEvent*);
      virtual void endEdit();
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

   public:
      ShadowNote(Score*);
      virtual ShadowNote* clone() const { return new ShadowNote(*this); }
      virtual ElementType type() const  { return SHADOW_NOTE; }
      int line() const                  { return _line;   }
      void setLine(int n)               { _line = n;      }
      virtual QRectF bbox() const;
      virtual void draw(Painter& p);
      };

#endif

