//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
//
//  Copyright (C) 2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "property.h"
#include "mscore.h"
#include "layoutbreak.h"

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

void setProperty(P_DATA_TYPE type, void* data, const QString& value)
      {
      switch(type) {
            case T_BOOL:
                  *(bool*)data = value.toInt();
                  break;
            case T_SUBTYPE:
            case T_INT:
                  *(int*)data = value.toInt();
                  break;
            case T_REAL:
                  *(qreal*)data = value.toDouble();
                  break;
            case T_COLOR:
                  *(QColor*)data = QColor();    // TODO value.toBool();
                  break;
            case T_DIRECTION:
                  {
                  if (value == "up")
                        *(Direction*)data = UP;
                  else if (value == "down")
                        *(Direction*)data = DOWN;
                  else if (value == "auto")
                        *(Direction*)data = AUTO;
                  }
                  break;
            case T_DIRECTION_H:
                  {
                  if (value == "left")
                        *(DirectionH*)data = DH_LEFT;
                  else if (value == "right")
                        *(DirectionH*)data = DH_RIGHT;
                  else if (value == "auto")
                        *(DirectionH*)data = DH_AUTO;
                  }
                  break;
            case T_LAYOUT_BREAK:
                  if (value == "line")
                        *(LayoutBreakType*)data = LAYOUT_BREAK_LINE;
                  else if (value == "page")
                        *(LayoutBreakType*)data = LAYOUT_BREAK_PAGE;
                  else if (value == "section")
                        *(LayoutBreakType*)data = LAYOUT_BREAK_SECTION;
                  break;
            case T_VALUE_TYPE:
                  if (value == "offset")
                        *(ValueType*)data = OFFSET_VAL;
                  else if (value == "user")
                        *(ValueType*)data = USER_VAL;
                  else if (value == "auto")
                        *(ValueType*)data = AUTO_VAL;
                  break;

            case T_POINT:
                  abort();
            }
      }

void setProperty(P_DATA_TYPE type, void* data, const QVariant& value)
      {
      switch(type) {
            case T_BOOL:
                  *(bool*)data = value.toBool();
                  break;
            case T_SUBTYPE:
            case T_INT:
                  *(int*)data = value.toInt();
                  break;
            case T_REAL:
                  *(qreal*)data = value.toDouble();
                  break;
            case T_POINT:
                  *(QPointF*)data = value.toPointF();
                  break;
            case T_COLOR:
                  *(QColor*)data = value.value<QColor>();
                  break;
            case T_DIRECTION:
            case T_DIRECTION_H:
            case T_LAYOUT_BREAK:
            case T_VALUE_TYPE:
                  *(int*)data = value.toInt();
                  break;
            }
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant getProperty(P_DATA_TYPE type, void* data)
      {
      switch(type) {
            case T_BOOL:
                  return QVariant(*(bool*)data);
            case T_SUBTYPE:
            case T_INT:
            case T_DIRECTION:
            case T_DIRECTION_H:
            case T_LAYOUT_BREAK:
            case T_VALUE_TYPE:
                  return QVariant(*(int*)data);
            case T_REAL:
                  return QVariant(*(qreal*)data);
            case T_COLOR:
                  return QVariant(*(QColor*)data);
            case T_POINT:
                  return QVariant(*(QPointF*)data);
            }
      return QVariant();
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant getProperty(P_DATA_TYPE type, const QDomElement& e)
      {
      const QString& value(e.text());
      switch(type) {
            case T_BOOL:
                  return QVariant(bool(value.toInt()));
            case T_SUBTYPE:
            case T_INT:
                  return QVariant(value.toInt());
            case T_REAL:
                  return QVariant(value.toDouble());
            case T_COLOR:
                  return QVariant(readColor(e));
            case T_POINT:
                  return QVariant(readPoint(e));
            case T_DIRECTION:
                  {
                  if (value == "up")
                        return QVariant(UP);
                  else if (value == "down")
                        return QVariant(DOWN);
                  else if (value == "auto")
                        return QVariant(AUTO);
                  }
                  break;
            case T_DIRECTION_H:
                  {
                  if (value == "left")
                        return QVariant(DH_LEFT);
                  else if (value == "right")
                        return QVariant(DH_RIGHT);
                  else if (value == "auto")
                        return QVariant(DH_AUTO);
                  }
                  break;
            case T_LAYOUT_BREAK:
                  if (value == "line")
                        return QVariant(int(LAYOUT_BREAK_LINE));
                  if (value == "page")
                        return QVariant(int(LAYOUT_BREAK_PAGE));
                  if (value == "section")
                        return QVariant(int(LAYOUT_BREAK_SECTION));
                  qDebug("getProperty: invalid T_LAYOUT_BREAK: <%s>", qPrintable(value));
                  break;
            case T_VALUE_TYPE:
                  if (value == "offset")
                        return QVariant(int(OFFSET_VAL));
                  else if (value == "user")
                        return QVariant(int(USER_VAL));
                  else if (value == "auto")
                        return QVariant(int(AUTO_VAL));
                  break;
            }
      return QVariant();
      }


