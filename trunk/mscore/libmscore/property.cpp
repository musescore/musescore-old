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
//   PropertyData
//---------------------------------------------------------

struct PropertyData {
      P_ID id;
      const char* name;       // xml name of property
      P_TYPE type;
      };

//
// always: property[subtype].id == subtype
//
static const PropertyData propertyList[] = {
      { P_SUBTYPE,             "subtype",       T_INT   },
      { P_SELECTED,            "selected",      T_BOOL  },
      { P_COLOR,               "color",         T_COLOR },
      { P_VISIBLE,             "visible",       T_BOOL  },
      { P_SMALL,               "small",         T_BOOL  },
      { P_SHOW_COURTESY,       "",              T_INT   },
      { P_LINE_TYPE,           "",              T_INT   },
      { P_PITCH,               "pitch",         T_INT },
      { P_TPC,                 "tpc",           T_INT },
      { P_HEAD_TYPE,           "headType",      T_INT },
      { P_HEAD_GROUP,          "head",          T_INT },
      { P_VELO_TYPE,           "veloType",      T_VALUE_TYPE },
      { P_VELO_OFFSET,         "velocity",      T_INT },
      { P_ARTICULATION_ANCHOR, "",              T_INT },
      { P_DIRECTION,           "direction",     T_DIRECTION },
      { P_STEM_DIRECTION,      "StemDirection", T_DIRECTION },
      { P_NO_STEM,             "",              T_INT },
      { P_SLUR_DIRECTION,      "",              T_INT },
      { P_LEADING_SPACE,       "",              T_INT },
      { P_TRAILING_SPACE,      "",              T_INT },
      { P_DISTRIBUTE,          "distribute",    T_BOOL },
      { P_MIRROR_HEAD,         "mirror",        T_DIRECTION_H },
      { P_DOT_POSITION,        "dotPosition",   T_DIRECTION },
      { P_ONTIME_OFFSET,       "onTimeOffset",  T_INT },
      { P_OFFTIME_OFFSET,      "offTimeOffset", T_INT },
      { P_TUNING,              "tuning",        T_REAL },
      { P_PAUSE,               "pause",         T_REAL },
      { P_BARLINE_SPAN,        "",              T_INT },
      { P_USER_OFF,            0,               T_POINT },
      { P_FRET,                "fret",          T_INT   },
      { P_STRING,              "string",        T_INT   },
      { P_GHOST,               "ghost",         T_BOOL  },
      { P_TIMESIG_NOMINAL,     0,               T_FRACTION },
      { P_TIMESIG_ACTUAL,      0,               T_FRACTION },
      { P_NUMBER_TYPE,         "numberType",    T_INT   },
      { P_BRACKET_TYPE,        "bracketType",   T_INT   },
      { P_NORMAL_NOTES,        "normalNotes",   T_INT   },
      { P_ACTUAL_NOTES,        "actualNotes",   T_INT   },
      { P_P1,                  "p1",            T_POINT },
      { P_P2,                  "p2",            T_POINT },
      { P_GROW_LEFT,           "growLeft",      T_REAL  },
      { P_GROW_RIGHT,          "growRight",     T_REAL  },
      { P_BOX_HEIGHT,          "",              T_REAL  },
      { P_BOX_WIDTH,           "",              T_REAL  },
      { P_TOP_GAP,             "",              T_SREAL },
      { P_BOTTOM_GAP,          "",              T_SREAL },
      { P_LEFT_MARGIN,         "",              T_REAL  },
      { P_RIGHT_MARGIN,        "",              T_REAL  },
      { P_TOP_MARGIN,          "",              T_REAL  },
      { P_BOTTOM_MARGIN,       "",              T_REAL  },
      { P_LAYOUT_BREAK,        "subtype",       T_LAYOUT_BREAK },
      { P_END,                 "",              T_INT   }
      };

//---------------------------------------------------------
//   propertyType
//---------------------------------------------------------

P_TYPE propertyType(P_ID id)
      {
      return propertyList[id].type;
      }

//---------------------------------------------------------
//   propertyName
//---------------------------------------------------------

const char* propertyName(P_ID id)
      {
      return propertyList[id].name;
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

void setProperty(P_ID id, void* data, const QString& value)
      {
      switch(propertyType(id)) {
            case T_BOOL:
                  *(bool*)data = value.toInt();
                  break;
            case T_SUBTYPE:
            case T_INT:
                  *(int*)data = value.toInt();
                  break;
            case T_REAL:
            case T_SREAL:
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
                  break;

            case T_FRACTION:
            case T_POINT:
                  abort();
            }
      }

void setProperty(P_ID id, void* data, const QVariant& value)
      {
      switch(propertyType(id)) {
            case T_BOOL:
                  *(bool*)data = value.toBool();
                  break;
            case T_SUBTYPE:
            case T_INT:
                  *(int*)data = value.toInt();
                  break;
            case T_SREAL:
            case T_REAL:
                  *(qreal*)data = value.toDouble();
                  break;
            case T_FRACTION:
                  *(Fraction*)data = value.value<Fraction>();
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
// getProperty
//---------------------------------------------------------

QVariant getProperty(P_ID id, const QDomElement& e)
      {
      const QString& value(e.text());
      switch(propertyType(id)) {
            case T_BOOL:
                  return QVariant(bool(value.toInt()));
            case T_SUBTYPE:
            case T_INT:
                  return QVariant(value.toInt());
            case T_REAL:
            case T_SREAL:
                  return QVariant(value.toDouble());
            case T_FRACTION:
                  return QVariant::fromValue(readFraction(e));
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
                  break;
            }
      return QVariant();
      }

