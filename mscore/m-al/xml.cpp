//=============================================================================
//  MuseScore
//  Music Score Editor/Player
//  $Id:$
//
//  Copyright (C) 2011 Werner Schweer
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

#include <QtCore/QPointF>
#include <QtCore/QSizeF>
#include "xml.h"
#include "color.h"
#include "fraction.h"

/*
 * strtod.c --
 *
 *    Source code for the "strtod" library procedure.
 *
 * Copyright (c) 1988-1993 The Regents of the University of California.
 * Copyright (c) 1994 Sun Microsystems, Inc.
 *
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appear in all copies.  The University of California
 * makes no representations about the suitability of this
 * software for any purpose.  It is provided "as is" without
 * express or implied warranty.
 *
 */

static int maxExponent = 511; /* Largest possible base 10 exponent.  Any
                         * exponent larger than this will already
                         * produce underflow or overflow, so there's
                         * no need to worry about additional digits.
                         */
static double powersOf10[] = {      /* Table giving binary powers of 10.  Entry */
          10.,                /* is 10^2^i.  Used to convert decimal */
          100.,               /* exponents into floating-point numbers. */
          1.0e4,
          1.0e8,
          1.0e16,
          1.0e32,
          1.0e64,
          1.0e128,
          1.0e256
      };

/*----------------------------------------------------------------------
 *
 * strtod --
 *
 *    This procedure converts a floating-point number from an ASCII
 *    decimal representation to internal double-precision format.
 *
 * Results:
 *    The return value is the double-precision floating-point
 *    representation of the characters in string.  If endPtr isn't
 *    NULL, then *endPtr is filled in with the address of the
 *    next character after the last one that was part of the
 *    floating-point number.
 *
 * Side effects:
 *    None.
 *
 *   const char *string;  * A decimal ASCII floating-point number,
 *                        * optionally preceded by white space.
 *                        * Must have form "-I.FE-X", where I is the
 *                        * integer part of the mantissa, F is the
 *                        * fractional part of the mantissa, and X
 *                        * is the exponent.  Either of the signs
 *                        * may be "+", "-", or omitted.  Either I
 *                        * or F may be omitted, or both.  The decimal
 *                        * point isn't necessary unless F is present.
 *                        * The "E" may actually be an "e".  E and X
 *                        * may both be omitted (but not just one).
 *                        *
 *   char **endPtr;       * If non-NULL, store terminating character's
 *                        * address here.
 *----------------------------------------------------------------------*/

static double mstrtod(const char* string, char** endPtr)
      {
      int sign, expSign = FALSE;
      double fraction, dblExp, *d;
      const char *p;
      int c;
      int exp = 0;        /* Exponent read from "EX" field. */
      int fracExp = 0;          /* Exponent that derives from the fractional
                               * part.  Under normal circumstatnces, it is
                               * the negative of the number of digits in F.
                               * However, if I is very long, the last digits
                               * of I get dropped (otherwise a long I with a
                               * large negative exponent could cause an
                               * unnecessary overflow on I alone).  In this
                               * case, fracExp is incremented one for each
                               * dropped digit. */
      int mantSize;       /* Number of digits in mantissa. */
      int decPt;                /* Number of mantissa digits BEFORE decimal
                               * point. */
      const char *pExp;         /* Temporarily holds location of exponent
                               * in string. */

      /*
       * Strip off leading blanks and check for a sign.
       */

      p = string;
      while (isspace(*p)) {
            p += 1;
            }
      if (*p == '-') {
            sign = TRUE;
            p += 1;
            }
      else {
            if (*p == '+') {
                  p += 1;
                  }
            sign = FALSE;
            }

      /*
       * Count the number of digits in the mantissa (including the decimal
       * point), and also locate the decimal point.
       */

       decPt = -1;
       for (mantSize = 0; ; mantSize += 1) {
            c = *p;
            if (!isdigit(c)) {
                  if ((c != '.') || (decPt >= 0)) {
                        break;
                        }
                  decPt = mantSize;
                  }
            p += 1;
            }

      /*
       * Now suck up the digits in the mantissa.  Use two integers to
       * collect 9 digits each (this is faster than using floating-point).
       * If the mantissa has more than 18 digits, ignore the extras, since
       * they can't affect the value anyway.
       */

      pExp  = p;
      p -= mantSize;
      if (decPt < 0) {
            decPt = mantSize;
            }
      else {
            mantSize -= 1;                /* One of the digits was the point. */
            }
      if (mantSize > 18) {
            fracExp = decPt - 18;
            mantSize = 18;
            }
      else {
            fracExp = decPt - mantSize;
            }
      if (mantSize == 0) {
            fraction = 0.0;
            p = string;
            goto done;
            }
      else {
            int frac1, frac2;
            frac1 = 0;
            for ( ; mantSize > 9; mantSize -= 1) {
                  c = *p;
                  p += 1;
                  if (c == '.') {
                        c = *p;
                        p += 1;
                        }
                  frac1 = 10*frac1 + (c - '0');
                  }
            frac2 = 0;
            for (; mantSize > 0; mantSize -= 1) {
                  c = *p;
                  p += 1;
                  if (c == '.') {
                        c = *p;
                        p += 1;
                        }
                  frac2 = 10*frac2 + (c - '0');
                  }
            fraction = (1.0e9 * frac1) + frac2;
            }

      /*
      * Skim off the exponent.
      */

      p = pExp;
      if ((*p == 'E') || (*p == 'e')) {
            p += 1;
            if (*p == '-') {
                  expSign = TRUE;
                  p += 1;
                  }
            else {
                  if (*p == '+') {
                        p += 1;
                        }
                  expSign = FALSE;
                  }
            while (isdigit(*p)) {
                  exp = exp * 10 + (*p - '0');
                  p += 1;
                  }
            }
      if (expSign) {
            exp = fracExp - exp;
            }
      else {
            exp = fracExp + exp;
            }

      /*
       * Generate a floating-point number that represents the exponent.
       * Do this by processing the exponent one bit at a time to combine
       * many powers of 2 of 10. Then combine the exponent with the
       * fraction.
       */

      if (exp < 0) {
            expSign = TRUE;
            exp = -exp;
            }
      else {
            expSign = FALSE;
            }
      if (exp > maxExponent) {
            exp = maxExponent;
//            errno = ERANGE;
            }
      dblExp = 1.0;
      for (d = powersOf10; exp != 0; exp >>= 1, d += 1) {
            if (exp & 01) {
                  dblExp *= *d;
                  }
            }
      if (expSign) {
            fraction /= dblExp;
            }
      else {
            fraction *= dblExp;
            }
   done:
      if (endPtr != NULL) {
            *endPtr = (char *) p;
            }

      if (sign) {
            return -fraction;
            }
      return fraction;
      }

//---------------------------------------------------------
//   matof
//---------------------------------------------------------

qreal matof(const char* p)
      {
      return mstrtod(p, 0);
      }

//---------------------------------------------------------
//   isTag
//---------------------------------------------------------

inline bool isTag(const MString8 s, const char* tag)
      {
      return xmlStrEqual(s.s(), (const xmlChar*)tag);
      }

//---------------------------------------------------------
//   operator==
//---------------------------------------------------------

bool operator==(const MString8 s, const char* tag)
      {
      return xmlStrEqual(s.s(), (const xmlChar*)tag);
      }

//---------------------------------------------------------
//   operator!=
//---------------------------------------------------------

bool operator!=(const MString8 s, const char* tag)
      {
      return !xmlStrEqual(s.s(), (const xmlChar*)tag);
      }

//---------------------------------------------------------
//   unknown
//---------------------------------------------------------

void XmlReader::unknown()
      {
      printf("xmlReader: unknown <%s> at %d\n", name(), line());
      xmlChar* dname = (xmlChar*)strdup((const char*)name());
      skipElement(dname);
      free(dname);
      }

//---------------------------------------------------------
//   readElement
//---------------------------------------------------------

bool XmlReader::readElement(const char* e)
      {
      while (read()) {
            if ((nodeType() == XML_READER_TYPE_END_ELEMENT) && xmlStrEqual(name(), (const xmlChar*)e))
                  break;
            else if (nodeType() == XML_READER_TYPE_ELEMENT)
                  return true;
            else {
                  ; // printf("readElement:ignore %d <%s>\n", nodeType(), name());
                  }
            }
      return false;
      }

bool XmlReader::readElement()
      {
      while (read()) {
            if (nodeType() == XML_READER_TYPE_ELEMENT) {
                  _tag = MString8(name());
                  return true;
                  }
            else if (nodeType() == XML_READER_TYPE_END_ELEMENT)
                  break;
            }
      return false;
      }

//---------------------------------------------------------
//   readAttribute
//---------------------------------------------------------

bool XmlReader::readAttribute()
      {
      if (moveToNextAttribute()) {
            _tag = MString8(name());
            return true;
            }
      return false;
      }

//---------------------------------------------------------
//   skipElement
//---------------------------------------------------------

void XmlReader::skipElement(const xmlChar* e)
      {
      //      printf("skip <%s>\n", e);
      while (read()) {
            // printf("  name<%s>\n", name());
            if ((nodeType() == XML_READER_TYPE_END_ELEMENT) && xmlStrEqual(name(), e))
                  break;
            }
      }

//---------------------------------------------------------
//   readInt
//---------------------------------------------------------

bool XmlReader::readInt(const char* tag, int* val)
      {
      if (isTag(name(), tag)) {
            if (read()) {
                  if (nodeType() == XML_READER_TYPE_TEXT) {
                        *val = atoi((const char*)value());
                        skipElement((const xmlChar*)tag);
                        return true;
                        }
                  else if (nodeType() == XML_READER_TYPE_END_ELEMENT)
                        return false;
                  else
                        printf("?readInt: node type %d <%s>\n", nodeType(), name());
                  }
            }
      return false;
      }

int XmlReader::readInt(const char* tag)
      {
      if (read()) {
            if (nodeType() == XML_READER_TYPE_TEXT) {
                  int val = atoi((const char*)value());
                  skipElement((const xmlChar*)tag);
                  return val;
                  }
            else if (nodeType() == XML_READER_TYPE_END_ELEMENT)
                  return 0;
            else
                  printf("?readInt: node type %d <%s>\n", nodeType(), name());
            }
      return false;
      }

//---------------------------------------------------------
//   readChar
//---------------------------------------------------------

bool XmlReader::readChar(const char* tag, char* val)
      {
      if (isTag(name(), tag)) {
            if (read()) {
                  if (nodeType() == XML_READER_TYPE_TEXT) {
                        *val = atoi((const char*)value());
                        skipElement((const xmlChar*)tag);
                        return true;
                        }
                  else if (nodeType() == XML_READER_TYPE_END_ELEMENT)
                        return false;
                  else
                        printf("?readInt: node type %d <%s>\n", nodeType(), name());
                  }
            }
      return false;
      }

//---------------------------------------------------------
//   readBool
//---------------------------------------------------------

bool XmlReader::readBool(const char* tag, bool* val)
      {
      if (isTag(name(), tag)) {
            if (read()) {
                  if (nodeType() == XML_READER_TYPE_TEXT) {
                        *val = atoi((const char*)value());
                        skipElement((const xmlChar*)tag);
                        return true;
                        }
                  else if (nodeType() == XML_READER_TYPE_END_ELEMENT)
                        return false;
                  else
                        printf("?readInt: node type %d <%s>\n", nodeType(), name());
                  }
            }
      return false;
      }

//---------------------------------------------------------
//   readReal
//---------------------------------------------------------

bool XmlReader::readReal(const char* tag, qreal* val)
      {
      if (isTag(name(), tag)) {
            if (read()) {
                  if (nodeType() == XML_READER_TYPE_TEXT) {
                        *val = (qreal)matof((const char*)value());
                        skipElement((const xmlChar*)tag);
                        return true;
                        }
                  else if (nodeType() == XML_READER_TYPE_END_ELEMENT)
                        return false;
                  else
                        printf("?readInt: node type %d <%s>\n", nodeType(), name());
                  }
            }
      return false;
      }

qreal XmlReader::readReal(const char* tag)
      {
      if (read()) {
            if (nodeType() == XML_READER_TYPE_TEXT) {
                  qreal val = matof((const char*)value());
                  skipElement((const xmlChar*)tag);
                  return val;
                  }
            else if (nodeType() == XML_READER_TYPE_END_ELEMENT)
                  return 0;
            else
                  printf("?readInt: node type %d <%s>\n", nodeType(), name());
            }
      return false;
      }


//---------------------------------------------------------
//   readString
//---------------------------------------------------------

bool XmlReader::readString(const char* tag, QString* val)
      {
      if (isTag(name(), tag)) {
            while (read()) {
                  if (nodeType() == XML_READER_TYPE_TEXT) {
                        *val = QString::fromUtf8((const char*)value());
                        skipElement((const xmlChar*)tag);
                        return true;
                        }
                  else if ((nodeType() == XML_READER_TYPE_END_ELEMENT) && isTag(name(), tag)) {
                        *val = QString();
                        return true;
                        }
                  else
                        printf("?readInt: node type %d <%s>\n", nodeType(), name());
                  }
            }
      return false;
      }

//---------------------------------------------------------
//   readPlacement
//---------------------------------------------------------

bool XmlReader::readPlacement(const char* tag, Placement* val)
      {
      if (isTag(name(), tag)) {
            while (read()) {
                  if (nodeType() == XML_READER_TYPE_TEXT) {
                        MString8 t = value();
                        if (t == "auto")
                              *val = PLACE_AUTO;
                        else if (t == "above")
                              *val = PLACE_ABOVE;
                        else if (t == "below")
                              *val = PLACE_BELOW;
                        else if (t == "left")
                              *val = PLACE_LEFT;
                        skipElement((const xmlChar*)tag);
                        return true;
                        }
                  else if ((nodeType() == XML_READER_TYPE_END_ELEMENT) && isTag(name(), tag)) {
                        *val = PLACE_AUTO;
                        return true;
                        }
                  else
                        printf("?readInt: node type %d <%s>\n", nodeType(), name());
                  }
            }
      return false;
      }


//---------------------------------------------------------
//   readString
//---------------------------------------------------------

bool XmlReader::readColor(const char* t, Color* val)
      {
      if (isTag(name(), t)) {
            while (readAttribute()) {
                  if (tag() == "r")
                        val->setRed(intValue());
                  else if (tag() == "g")
                        val->setGreen(intValue());
                  else if (tag() == "b")
                        val->setBlue(intValue());
                  }
            read();
            return true;
            }
      return false;
      }

//---------------------------------------------------------
//   readPoint
//---------------------------------------------------------

bool XmlReader::readPoint(const char* t, QPointF* val)
      {
      if (isTag(name(), t)) {
            while (readAttribute()) {
                  if (tag() == "x")
                        val->setX(realValue());
                  else if (tag() == "y")
                        val->setY(realValue());
                  }
            read();
            return true;
            }
      return false;
      }

//---------------------------------------------------------
//   readSize
//---------------------------------------------------------

bool XmlReader::readSize(const char* t, QSizeF* val)
      {
      if (isTag(name(), t)) {
            while (readAttribute()) {
                  if (tag() == "w")
                        val->setWidth(realValue());
                  else if (tag() == "h")
                        val->setHeight(realValue());
                  }
            read();
            return true;
            }
      return false;
      }


//---------------------------------------------------------
//   readFraction
//---------------------------------------------------------

bool XmlReader::readFraction(const char* t, Fraction* val)
      {
      if (isTag(name(), t)) {
            while (readAttribute()) {
                  if (tag() == "z")
                        val->setNumerator(realValue());
                  else if (tag() == "n")
                        val->setDenominator(realValue());
                  }
            read();
            return true;
            }
      return false;
      }


