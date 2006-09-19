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

#include "globals.h"
#include "plist.h"
#include "element.h"

class Tie;
class Chord;
class Fingering;
class Score;
class Painter;

//---------------------------------------------------------
//   Note
//    changePitch()         computes _line and _accidental
//                          resets _userAccidental
//    changeAccidental()    computes _pitch
//
//    setPitch()/setAccidental() do what the name says
//---------------------------------------------------------

class Note : public Element {
      int _pitch;             // note pitch as midi value
      int _userAccidental;    // -1 - automatic accidental
      int _line;              // y-Position; 0 - top line
      int _move;              // -1, 0, +1
      Accidental* _accidental;

      Sym _sym;         // note head
      DurationType _durationType;
      bool _grace;
      bool _mirror;     // note is mirrored at stem
      int _dots;
      Fingering* _fingering;

      Tie* _tieFor;
      Tie* _tieBack;

      int _lineOffset;  // used during mouse dragging

      virtual bool startDrag(const QPointF&);
      virtual QRectF drag(const QPointF& s);
      virtual void endDrag();

   public:
      Note(Score*);
      Note(Score*, int pitch, bool grace = false);
      ~Note();
      virtual ElementType type() const { return NOTE; }

      virtual void bboxUpdate();
      bool grace() const            { return _grace; }
      void setGrace(bool val)       { _grace = val;  }

      void setHead(int);
      int totalTicks() const;
      void setType(DurationType);
      void setDots(int n)           { _dots = n; }
      int dots() const              { return _dots; }

      double headWidth() const      { return _sym.width(); }
      Sym* noteHead()               { return &_sym;  }

      int pitch() const             { return _pitch; }
      void setPitch(int val)        { _pitch = val; }
      void changePitch(int val);
      int move() const              { return _move; }
      void setMove(int val)         { _move = val; }

      int userAccidental() const      { return _userAccidental; }
      void setUserAccidental(int i)   { _userAccidental = i; }
      int accidentalIdx() const       { return _accidental ? _accidental->idx() : ACC_NONE; }
      Accidental* accidental() const  { return _accidental; }
      void setAccidental(int);
      void changeAccidental(Score*, int);

      int line() const                { return _line + _lineOffset;   }
      void setLine(int n)             { _line = n;      }

      Fingering* fingering() const    { return _fingering; }

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

      virtual void draw1(Painter&) const;
      virtual void read(QDomNode);
      virtual void write(Xml& xml) const;
      QPointF stemPos(bool upFlag) const;    // point to connect stem
      Element* findSelectableElement(QPointF p) const;
      virtual bool acceptDrop(int, int) const;
      virtual void drop(const QPointF&, int, int);
      };

//---------------------------------------------------------
//   ShadowNote
//    shows the note insert position in note entry mode
//---------------------------------------------------------

class ShadowNote : public Element {
      Sym _sym;         // note head
      int _line;

   public:
      ShadowNote(Score*);
      virtual ElementType type() const  { return SHADOW_NOTE; }
      int line() const                  { return _line;   }
      void setLine(int n)               { _line = n;      }
      virtual void layout();
      virtual void draw(Painter& p) const;
      };

#endif

