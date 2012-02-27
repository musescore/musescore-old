//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id$
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "figuredbass.h"
#include "score.h"
#include "note.h"
#include "measure.h"
#include "system.h"

// !! TO DO !! This table should come from a configuration file

#define FBIDigitNone    0

static QChar g_CombinedForm[2][2][10] =
{ //    0          1          2          3          4          5          6          7         8         9
  // modern forms
{ { L'\xE20B', L'\xE20C', L'\xE20D', L'\xE20E', L'\xE20F', L'\xE211', L'\xE212', L'\xE213', L'\xE214', L'\xE215' },   // plus & backsl.
  {    '?',       '?',       '?',       '?',       '?',    L'\xE210',    '?',       '?',      '?',      '?' },   // slash
},
  // historic forms
{ { L'\xE200', L'\xE201', L'\xE202', L'\xE203', L'\xE204', L'\xE206', L'\xE207', L'\xE208', L'\xE209', L'\xE20A' },   // plus & backsl.
  {    '?',       '?',       '?',       '?',       '?',    L'\xE205',    '?',       '?',      '?',      '?' },   // slash
}
};

//---------------------------------------------------------
//   FiguredBassItem
//---------------------------------------------------------

// used for formatted display (also should go into configuration files)
const QChar FiguredBassItem::accidToChar[FBINumOfAccid] =
{ 0, L'\xE114', L'\xE113', L'\xE10E', '+', '\\', '/', L'\xE11A', L'\xE11C'};
const QChar FiguredBassItem::parenthToChar[FBINumOfParenth] =
{ 0, '(', ')', '[', ']'};

// used for normalized display (in principle, might be different from formatted forms)
// (these is no normAccidToChar[], as accidentals may use mult. chars in normalized display):
const QChar FiguredBassItem::normParenthToChar[FBINumOfParenth] =
{ 0, '(', ')', '[', ']'};


FiguredBassItem::FiguredBassItem(Score* s, int l)
      : SimpleText(s), ord(l)
      {
      prefix      = suffix = FBIAccidNone;
      digit       = FBIDigitNone;
      parenth[0]  = parenth[1] = parenth[2] = parenth[3] = parenth[4] = FBIParenthNone;
      contLine    = false;
      setTextStyle(TEXT_STYLE_FIGURED_BASS);
      }

FiguredBassItem::FiguredBassItem(const FiguredBassItem& item)
   : SimpleText(item)
      {
      ord               = item.ord;
      prefix            = item.prefix;
      digit             = item.digit;
      suffix            = item.suffix;
      parenth[0]        = item.parenth[0];
      parenth[1]        = item.parenth[1];
      parenth[2]        = item.parenth[2];
      parenth[3]        = item.parenth[3];
      parenth[4]        = item.parenth[4];
      contLine          = item.contLine;
      }

FiguredBassItem::~FiguredBassItem()
      {
      }

//---------------------------------------------------------
//   FiguredBassItem parse()
//
// converts a string into a property-based representation, if possible;
// return true on success | false if the string is non-conformant
//---------------------------------------------------------

bool FiguredBassItem::parse(QString& str)
      {
      int               retVal;

      parseParenthesis(str, 0);
      retVal = parsePrefixSuffix(str, true);          // prefix
      if(retVal == -1)
            return false;
      parseParenthesis(str, 1);
      retVal = parseDigit(str);                       // digit
      if(retVal == -1)
            return false;
      parseParenthesis(str, 2);
      retVal = parsePrefixSuffix(str, false);         // suffix
      if(retVal == -1)
            return false;
      parseParenthesis(str, 3);
      // check for a possible cont. line symbol(s)
      contLine = false;                               // contLine
      while(str[0] == '-' || str[0] == '_') {
            contLine = true;
            str.remove(0, 1);
      }
      parseParenthesis(str, 4);

      // remove useless parentheses
      if(prefix == FBIAccidNone && parenth[1] == FBIParenthNone) {
            parenth[1] = parenth[0];
            parenth[0] = FBIParenthNone;
            }
      if(digit == FBIDigitNone && parenth[2] == FBIParenthNone) {
            parenth[2] = parenth[1];
            parenth[1] = FBIParenthNone;
            }
      if(!contLine && parenth[3] == FBIParenthNone) {
            parenth[3] = parenth[4];
            parenth[4] = FBIParenthNone;
            }
      if(suffix == FBIAccidNone && parenth[2] == FBIParenthNone) {
            parenth[2] = parenth[3];
            parenth[3] = FBIParenthNone;
            }

      // some checks:
      // if some extra input, str is not conformat
      if(str.size())
            return false;
      // prefix and suffix cannot be both non-empty
      // prefix, digit and suffix cannot be ALL empty
      // suffix cannot combine with empty digit
      if( (prefix != FBIAccidNone && suffix != FBIAccidNone)
            || (prefix == FBIAccidNone && digit == FBIDigitNone && suffix == FBIAccidNone)
            || ( (suffix == FBIAccidPlus || suffix == FBIAccidBackslash || suffix == FBIAccidSlash)
                  && digit == FBIDigitNone) )
            return false;
      return true;
}

//---------------------------------------------------------
//   FiguredBassItem parsePrefixSuffix()
//
//    scans str to extract prefix or suffix properties. Stops at the first char which cannot fit.
//    Fitting chars are removed from str. DOES NOT generate any display text
//
// returns the number of QChar's read from str or -1 if prefix / suffix has an illegal format
// (no prefix / suffix at all IS legal)
//---------------------------------------------------------

int FiguredBassItem::parsePrefixSuffix(QString& str, bool bPrefix)
      {
      FBIAccidental *   dest        = bPrefix ? &prefix : &suffix;
      bool              done        = false;
      int               size        = str.size();
      str = str.trimmed();

      *dest             = FBIAccidNone;

      while(str.size()) {
            switch(str.at(0).unicode())
            {
            case 'b':
                  if(*dest != FBIAccidNone) {
                        if(*dest == FBIAccidFlat)     // FLAT may double a previous FLAT
                              *dest = FBIAccidDoubleFlat;
                        else
                              return -1;              // but no other combination is acceptable
                        }
                  *dest = FBIAccidFlat;
                  break;
            case 'h':
                  if(*dest != FBIAccidNone)           // cannot combine with any other accidental
                        return -1;
                  *dest = FBIAccidNatural;
                  break;
            case '#':
                  if(*dest != FBIAccidNone) {
                        if(*dest == FBIAccidSharp)    // SHARP may double a preivous SHARP
                              *dest = FBIAccidDoubleSharp;
                        else
                              return -1;              // but no other combination is acceptable
                        }
                  *dest = FBIAccidSharp;
                  break;
            // '+', '\\' and '/' go into the suffix
            case '+':
                  if(suffix != FBIAccidNone)          // cannot combine with any other accidental
                        return -1;
                  suffix = FBIAccidPlus;
                  break;
            case '\\':
                  if(suffix != FBIAccidNone)          // cannot combine with any other accidental
                        return -1;
                  suffix = FBIAccidBackslash;
                  break;
            case '/':
                  if(suffix != FBIAccidNone)          // cannot combine with any other accidental
                        return -1;
                  suffix = FBIAccidSlash;
                  break;
            default:                                  // any other char: no longer in prefix/suffix
                  done = true;
                  break;
            }
            if(done)
                  break;
            str.remove(0,1);                         // 'eat' the char and continue
            }

      return size - str.size();                       // return how many chars we had read into prefix/suffix
      }

//---------------------------------------------------------
//   FiguredBassItem parseDigit()
//
//    scans str to extract digit properties. Stops at the first char which cannot belong to digit part.
//    Fitting chars are removed from str. DOES NOT generate any display text
//
// returns the number of QChar's read from str or -1 if no legal digit can be constructed
// (no digit at all IS legal)
//---------------------------------------------------------

int FiguredBassItem::parseDigit(QString& str)
      {
      int  size   = str.size();
      str         = str.trimmed();

      digit = FBIDigitNone;

      while(str.size()) {
            // any digit acceptable, if no previous digit
            if(str[0] >= '1' && str[0] <= '9') {
                  if(digit == FBIDigitNone) {
                        digit = str[0].unicode() - '0';
                        str.remove(0, 1);
                        }
                  else
                        return -1;
                  }
            // anything else: no longer in digit part
            else
                  break;
            }

      return size  - str.size();
      }

//---------------------------------------------------------
//   FiguredBassItem parseParenthesis()
//
//    scans str to extract a (possible) parenthesis, stores its code into parenth[parenthIdx]
//    and removes it from str. Only looks at first str char.
//
// returns the number of QChar's read from str (actually 0 or 1).
//---------------------------------------------------------

int FiguredBassItem::parseParenthesis(QString& str, int parenthIdx)
      {
      int c = str[0].unicode();
      FBIParenthesis code = FBIParenthNone;
      switch(c)
      {
      case '(':
            code =FBIParenthRoundOpen;
            break;
      case ')':
            code =FBIParenthRoundClosed;
            break;
      case '[':
            code =FBIParenthSquaredOpen;
            break;
      case ']':
            code =FBIParenthSquaredClosed;
            break;
      default:
            break;
            }
      parenth[parenthIdx] = code;
      if(code != FBIParenthNone) {
            str.remove(0, 1);
            return 1;
            }
      return 0;
      }

//---------------------------------------------------------
//   FiguredBassItem normalizedText()
//
// returns a string with the normalized text, i.e. the text displayed while editing;
// this is a standard textual representation of the item properties
//---------------------------------------------------------

QString FiguredBassItem::normalizedText() const
      {
      QString str = QString();
      if(parenth[0] != FBIParenthNone)
            str.append(normParenthToChar[parenth[0]]);

      if(prefix != FBIAccidNone) {
            switch(prefix)
            {
            case FBIAccidFlat:
                  str.append('b');
                  break;
            case FBIAccidNatural:
                  str.append('h');
                  break;
            case FBIAccidSharp:
                  str.append('#');
                  break;
            case FBIAccidDoubleFlat:
                  str.append("bb");
                  break;
            case FBIAccidDoubleSharp:
                  str.append("##");
                  break;
            default:
                  break;
            }
            }

      if(parenth[1] != FBIParenthNone)
            str.append(normParenthToChar[parenth[1]]);

      // digit
      if(digit != FBIDigitNone)
            str.append(QChar('0' + digit));

      if(parenth[2] != FBIParenthNone)
            str.append(normParenthToChar[parenth[2]]);

      // suffix
      if(suffix != FBIAccidNone) {
            switch(suffix)
            {
            case FBIAccidFlat:
                  str.append('b');
                  break;
            case FBIAccidNatural:
                  str.append('h');
                  break;
            case FBIAccidSharp:
                  str.append('#');
                  break;
            case FBIAccidPlus:
                  str.append('+');
                  break;
            case FBIAccidBackslash:
                  str.append('\\');
                  break;
            case FBIAccidSlash:
                  str.append('/');
                  break;
            case FBIAccidDoubleFlat:
                  str.append("bb");
                  break;
            case FBIAccidDoubleSharp:
                  str.append("##");
                  break;
            default:
                  break;
            }
            }

      if(parenth[3] != FBIParenthNone)
            str.append(normParenthToChar[parenth[3]]);
      if(contLine)
            str.append('_');
      if(parenth[4] != FBIParenthNone)
            str.append(normParenthToChar[parenth[4]]);

      return str;
      }

//---------------------------------------------------------
//   FiguredBassItem write()
//---------------------------------------------------------

void FiguredBassItem::write(Xml& xml) const
{
      xml.stag("FiguredBassItem");
      xml.tagE(QString("brackets b0=\"%1\" b1=\"%2\" b2=\"%3\" b3=\"%4\" b4=\"%5\"")
                    .arg(parenth[0]) .arg(parenth[1]) .arg(parenth[2]) .arg(parenth[3]) .arg(parenth[4]) );
      if(prefix != FBIAccidNone)
            xml.tag(QString("prefix"), prefix);
      if(digit != FBIDigitNone)
            xml.tag(QString("digit"), digit);
      if(suffix != FBIAccidNone)
            xml.tag(QString("suffix"), suffix);
      if(contLine)
            xml.tag("continuationLine", contLine);
      xml.etag();
}

//---------------------------------------------------------
//   FiguredBassItem read()
//---------------------------------------------------------

void FiguredBassItem::read(const QDomElement& de)
{
      for (QDomElement e = de.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            const QString& tag(e.tagName());
            const QString& val(e.text());
            int   iVal = val.toInt();

            if(tag == "brackets") {
                  parenth[0] = (FBIParenthesis)e.attribute("b0").toInt();
                  parenth[1] = (FBIParenthesis)e.attribute("b1").toInt();
                  parenth[2] = (FBIParenthesis)e.attribute("b2").toInt();
                  parenth[3] = (FBIParenthesis)e.attribute("b3").toInt();
                  parenth[4] = (FBIParenthesis)e.attribute("b4").toInt();
                  }
            if(tag == "prefix")
                  prefix = (FBIAccidental)iVal;
            else if(tag == "digit")
                  digit = iVal;
            else if(tag == "suffix")
                  suffix = (FBIAccidental)iVal;
            else if(tag == "continuationLine")
                  contLine = iVal;
            else if(!Element::readProperties(e))
                  domError(e);
            }
}

//---------------------------------------------------------
//   FiguredBassItem layout()
//    creates the display text (set as element text) and computes
//    the horiz. offset needed to align the right part as well as the vert. offset
//---------------------------------------------------------

void FiguredBassItem::layout()
      {
      qreal             h, w, x, x1, x2, y;

      if (textStyle() == TEXT_STYLE_INVALID)
            setTextStyle(TEXT_STYLE_FIGURED_BASS);
      QFontMetricsF     fm(style().font(spatium()));
      QString           str = QString();
      x = symbols[score()->symIdx()][quartheadSym].width(magS()) * .5;
      x1 = x2 = 0.0;

      // create display text

      if(parenth[0] != FBIParenthNone)
            str.append(parenthToChar[parenth[0]]);

      // prefix
      if(prefix != FBIAccidNone) {
            // if no digit, the string created so far 'hangs' to the left of the note
            if(digit == FBIDigitNone)
                  x1 = fm.width(str);
            str.append(accidToChar[prefix]);
            // if no digit, the string from here onward 'hangs' to the right of the note
            if(digit == FBIDigitNone)
                  x2 = fm.width(str);
            }

      if(parenth[1] != FBIParenthNone)
            str.append(parenthToChar[parenth[1]]);

      // digit
      if(digit != FBIDigitNone) {
            // if some digit, the string created so far 'hangs' to the left of the note
            x1 = fm.width(str);
            // if suffix is a combining shape, combine it with digit
            // unless there is a parenthesis in between
            if( (suffix == FBIAccidPlus || suffix == FBIAccidBackslash || suffix == FBIAccidSlash)
                        && parenth[2] == FBIParenthNone) {
                  int sel = (suffix == FBIAccidPlus || suffix == FBIAccidBackslash) ? 0 : 1;
                  str.append(g_CombinedForm[0][sel][digit]);
            }
            else
                  str.append('0' + digit);
            // if some digit, the string from here onward 'hangs' to the right of the note
            x2 = fm.width(str);
            }

      if(parenth[2] != FBIParenthNone)
            str.append(parenthToChar[parenth[2]]);

      // suffix
      // append only if non-combining shape or cannot combine (no digit or parenthesis in between)
      if( (suffix != FBIAccidNone && suffix != FBIAccidPlus
                        && suffix != FBIAccidBackslash && suffix != FBIAccidSlash)
                  || digit == FBIDigitNone
                  || parenth[2] != FBIParenthNone)
            str.append(accidToChar[suffix]);

      if(parenth[3] != FBIParenthNone)
            str.append(parenthToChar[parenth[3]]);

      // !! TO DO !! INSERT HERE PROPER CONT. LINE FORMATTING
      if(contLine)                              // currently, only a token representation is provided
            str.append("___");
      if(parenth[4] != FBIParenthNone)
            str.append(parenthToChar[parenth[4]]);

      setText(str);                             // this text will be displayed

      // position the text so that [x1...x2] is centered below the note
      x = x - (x1+x2) * 0.5;
      h = fm.lineSpacing();
      h *= score()->styleD(ST_figuredBassLineHeight);
      w = fm.width(str);
      y = h * ord;
      setPos(x, y);
      setbbox(QRect(0, 0, w, h));
}

//---------------------------------------------------------
//   FiguredBass
//---------------------------------------------------------

#include "chord.h"

FiguredBass::FiguredBass(Score* s)
   : Text(s)
      {
      setTextStyle(TEXT_STYLE_FIGURED_BASS);
      setTicks(0);
      items.clear();
      }

FiguredBass::FiguredBass(const FiguredBass& fb)
   : Text(fb)
      {
      setTicks(fb.ticks());
      items = fb.items;
      }

FiguredBass::~FiguredBass()
      {
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void FiguredBass::write(Xml& xml) const
      {
      xml.stag("FiguredBass");
      if (ticks() > 0) {
            xml.tag("ticks", ticks());
            }
      foreach(FiguredBassItem item, items)
            item.write(xml);
      Element::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void FiguredBass::read(const QDomElement& de)
      {
      QString normalizedText = QString();
      int idx = 0;
      for (QDomElement e = de.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            const QString& tag(e.tagName());
            const QString& val(e.text());
            if(tag == "ticks")
                  setTicks(val.toInt());
            else if (tag == "FiguredBassItem") {
                  FiguredBassItem * pItem = new FiguredBassItem(score(), idx++);
                  pItem->setTrack(track());
                  pItem->setParent(this);
                  pItem->read(e);
                  items.append(*pItem);
                  // add item normalized text
                  if(!normalizedText.isEmpty())
                        normalizedText.append('\n');
                  normalizedText.append(pItem->normalizedText());
                  }
            else if(!Element::readProperties(e))
                  domError(e);
            }
      setText(normalizedText);                  // this is the text to show while editing
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

// uncomment for using built-in edit Text layout
#define _USE_EDIT_TEXT_LAYOUT_

void FiguredBass::layout()
      {
      if (!styled())
            setTextStyle(TEXT_STYLE_FIGURED_BASS);
      qreal             y;

#ifdef _USE_EDIT_TEXT_LAYOUT_
      if (_editMode) {
            Text::layout();
            return;
            }
#endif

      // vertical position
      y = 0;                                          // default vert. pos.
      qreal staffHeight = 4 * spatium();              // assume a standard staff height
      if(parent() && track() >= 0) {
            System* sys = ((Segment*)parent())->measure()->system();
            if (sys == 0)
                  qDebug("FiguredBass layout: no system!");
            else {
                  SysStaff* staff = sys->staff(staffIdx());
                  staffHeight = staff->bbox().height();
                  y = staff->y();
                  }
            }
      y += point(score()->styleS(ST_figuredBassDistance));
      y += staffHeight;

      // bounding box
#ifndef _USE_EDIT_TEXT_LAYOUT_
      qreal             h, w, w1;

      if(editMode()) {
            QFontMetricsF     fm(style().font(spatium()));
            // box width
            w = 0;
            QStringList list = getText().split('\n');
            foreach(QString str, list) {
                  w1 = fm.width(str);
                  if(w1 > w)
                        w = w1;
                  }
            // bbox height
            h = fm.lineSpacing();
            h *= score()->styleD(ST_figuredBassLineHeight);
            h *= (list.size() > 1 ? list.size() : 1);      // at least 1 line
            // ready to set position and bbox
            setPos(0, y);
            setbbox(QRectF(0, 0, w, h));
            }
      else
#endif
            {
            setPos(0, y);
            setbbox(QRect());
            // layout each item and enlarge bbox to include items bboxes
            for(int i=0; i < items.size(); i++) {
                  items[i].layout();
                  addbbox(items[i].bbox().translated(items[i].pos()));
                  }
            }
      adjustReadPos();
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void FiguredBass::draw(QPainter* painter) const
      {
      if(editMode())
            Text::draw(painter);
      else {
            foreach(FiguredBassItem item, items) {
                  painter->translate(item.pos());
                  item.draw(painter);
                  painter->translate(-item.pos());
                  }
            }
      }

//---------------------------------------------------------
//   endEdit
//---------------------------------------------------------

void FiguredBass::endEdit()
      {
      int         idx;

      Text::endEdit();
      QString txt = getText();
      if(txt.isEmpty())
            return;

      // split text into lines and create an item for each line
      QStringList list = txt.split('\n', QString::SkipEmptyParts);
      items.clear();
      QString normalizedText = QString();
      idx = 0;
      foreach(QString str, list) {
            FiguredBassItem* pItem = new FiguredBassItem(score(), idx++);
            if(!pItem->parse(str))
                  return;
            pItem->setTrack(track());
            pItem->setParent(this);
            items.append(*pItem);

            // add item normalized text
            if(!normalizedText.isEmpty())
                  normalizedText.append('\n');
            normalizedText.append(pItem->normalizedText());
            }
      setText(normalizedText);
      layout();
      }

//---------------------------------------------------------
//   setSelected /setVisible
//
//    forward flags to items
//---------------------------------------------------------

void FiguredBass::setSelected(bool flag)
      {
      Element::setSelected(flag);
      for(int i=0; i < items.size(); i++) {
            items[i].setSelected(flag);
            }
      }

void FiguredBass::setVisible(bool flag)
      {
      Element::setVisible(flag);
      for(int i=0; i < items.size(); i++) {
            items[i].setVisible(flag);
            }
      }

//---------------------------------------------------------
//   STATIC FUNCTION
//    adding a new FiguredBass to a Segment;
//    the main purpose of this function is to ensure that ONLY ONE F.b. element exists for each Segment/track;
//    they either re-use an existing FiguredBass or create a new one if none if found;
//    they return the FiguredBass and set pNew to true if it has been newly created.
//
//    As the F.b. very concept requires the underlying chord to have ONLY ONE note, 
//    it might make sense to drop the checking / setting of Track and unconditionally
//    always use track 0 for FiguredBass elements.
//---------------------------------------------------------

FiguredBass * FiguredBass::addFiguredBassToSegment(Segment * seg, int track, int ticks, bool * pNew)
      {
      // scan segment annotations for an existing FB element
      const QList<Element*>& annot = seg->annotations();
      int i;
      int count = annot.size();
      FiguredBass* fb;
      for(i = 0; i < count; i++) {
            if(annot.at(i)->type() == FIGURED_BASS && annot.at(i)->track() == track) {
                  // an FB already exists in segment: re-use it
                  fb = static_cast<FiguredBass*>(annot.at(i));
                  *pNew = false;
                  break;
                  }
            }
      if(i >= count) {                          // no FB at segment: create new
            fb = new FiguredBass(seg->score());
            fb->setTrack(track);
            fb->setParent(seg);
            fb->setTicks(ticks);
            *pNew = true;
            }
      return fb;
      }

//---------------------------------------------------------
//
// METHODS BELONGING TO OTHER CLASSES
//
//    Work In Progress: kept here until the FiguredBass framwork is reasonably set up;
//    To be finally moved to their respective class implementation files.
//
//---------------------------------------------------------

//---------------------------------------------------------
//   Score::addFiguredBass
//    called from Keyboard Accelerator & menus
//---------------------------------------------------------

#include "score.h"

FiguredBass* Score::addFiguredBass()
      {
      Element* el = selection().element();
      if (el == 0 || (el->type() != NOTE && el->type() != FIGURED_BASS)) {
            QMessageBox::information(0,
               QMessageBox::tr("MuseScore:"),
               QMessageBox::tr("No note or figured bass selected:\n"
                  "Please select a single note or figured bass and retry.\n"),
               QMessageBox::Ok, QMessageBox::NoButton);
            return 0;
            }

      FiguredBass * fb;
      bool bNew;
      if (el->type() == NOTE) {
            ChordRest * cr = static_cast<Note*>(el)->chord();
            fb = FiguredBass::addFiguredBassToSegment(cr->segment(),
                        cr->track(), cr->duration().ticks(), &bNew);
            }
      else if (el->type() == FIGURED_BASS) {
            fb = static_cast<FiguredBass*>(el);
            bNew = false;
            }
      else
            return 0;

      if(fb == 0)
            return 0;

      if(bNew)
            undoAddElement(fb);
      select(fb, SELECT_SINGLE, 0);
      return fb;
      }

#include "mscore/scoreview.h"
#include "segment.h"

//---------------------------------------------------------
//   ScoreView::figuredEndEdit
//    derived from harmonyEndEdit()
//    remove the FB if empty
//---------------------------------------------------------

void ScoreView::figuredBassEndEdit()
      {
      FiguredBass* fb         = static_cast<FiguredBass*>(editObject);
      FiguredBass* origFb     = static_cast<FiguredBass*>(origEditObject);

      if (fb->isEmpty() && origFb->isEmpty())
            fb->parent()->remove(fb);
      }

//---------------------------------------------------------
//   ScoreView::figuredBassTab
//    derived from chordTab() (for Harmony)
//    manages [Space] / [Shift][Space] keys, moving editing to FB of next/prev ChordRest
//---------------------------------------------------------

void ScoreView::figuredBassTab(bool back)
      {
      FiguredBass* fb  = (FiguredBass*)editObject;
      Segment* segment = fb->segment();
      int track        = fb->track();
      if (segment == 0) {
            qDebug("figuredBassTab: no segment");
            return;
            }

      // search next chord
      Segment * nextSeg;
      if (back)
            nextSeg = segment->prev1(SegChordRest);
      else
            nextSeg = segment->next1(SegChordRest);
      if (nextSeg == 0) {
            qDebug("figuredBassTab: no prev/next segment");
            return;
            }
      endEdit();
      _score->startCmd();

      // !! TODO !! Detect correct ticks for new fb
      bool bNew;
      FiguredBass * fbNew = FiguredBass::addFiguredBassToSegment(nextSeg, track, 0, &bNew);
      // if the next segment is at a tick distance from the prev. other than prev. fb duration
      // update prev. fb duration
//      if( (nextSeg->tick() - segment->tick()) != fb->ticks())   NOT YET!
//            fb->setTicks(nextSeg->tick() - segment->tick());

//      _score->startCmd();                     // used by lyricsTab() but not by chordTab(): needed or not?
      if(bNew)
            _score->undoAddElement(fbNew);
      _score->select(fbNew, SELECT_SINGLE, 0);
      startEdit(fbNew, -1);
      adjustCanvasPosition(fbNew, false);
      ((FiguredBass*)editObject)->moveCursorToEnd();
      _score->setLayoutAll(true);
//      _score->end2();                         // used by lyricsTab() but not by chordTab(): needed or not?
//      _score->end1();                         //          "           "
      }
/*
//---------------------------------------------------------
//   ScoreView::figuredBassTab
//    derived from LyricsTab(): currently, here only for reference
//---------------------------------------------------------

void ScoreView::figuredBassTab(bool back)
      {
      FiguredBass* fb  = (FiguredBass*)editObject;
      int track        = fb->track();
//      int staffIdx     = fb->staffIdx();
      Segment* segment = fb->segment();

      Segment* nextSegment = segment;
      if (back) {
            // search prev chord
            while ((nextSegment = nextSegment->prev1(SegChordRest))) {
                  Element* el = nextSegment->element(track);
                  if (el &&  el->type() == CHORD)
                        break;
                  }
            }
      else {
            // search next chord
            while ((nextSegment = nextSegment->next1(SegChordRest))) {
                  Element* el = nextSegment->element(track);
                  if (el &&  el->type() == CHORD)
                        break;
                  }
            }
      if (nextSegment == 0)
            return;

//      fb->ticks = score()->inputState().duration().ticks();
      endEdit();

      bool bNew;
      fb = FiguredBass::addFiguredBassToSegment(nextSegment, track, &bNew);
      if(fb == 0)
            return;

      _score->startCmd();
      if(bNew)
          _score->undoAddElement(fb);
      _score->select(fb, SELECT_SINGLE, 0);
      startEdit(fb, -1);
      adjustCanvasPosition(fb, false);
      ((Lyrics*)editObject)->moveCursorToEnd();
      _score->setLayoutAll(true);
      _score->end2();
      _score->end1();
      }
*/
