//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2007 Werner Schweer and others
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
//
// Lilypond export.
// For HISTORY, NEWS and TODOS: see end of file (Olav).
//
// Some revisions by olagunde@start.no As I have no prior knowledge of
// C or C++, I have to resort to techniques well known in standard
// Pascal as it was known in 1983, when I got my very short
// programming education in the "Programming for Poets" course.
// Olav.


#include "arpeggio.h"
#include "articulation.h"
#include "barline.h"
#include "beam.h"
#include "bracket.h"
#include "chord.h"
#include "clef.h"
#include "config.h"
#include "dynamics.h"
#include "globals.h"
#include "hairpin.h"
#include "key.h"
#include "keysig.h"
#include "measure.h"
#include "note.h"
#include "ottava.h"
#include "page.h"
#include "part.h"
#include "pedal.h"
#include "repeat.h"
#include "rest.h"
#include "score.h"
#include "segment.h"
#include "slur.h"
#include "staff.h"
#include <string.h>
#include <sstream>
#include "style.h"
#include "sym.h"
#include "tempotext.h"
#include "text.h"
#include "timesig.h"
#include "tuplet.h"
#include "volta.h"


static  const int MAX_SLURS = 8;
static  const int BRACKSTAVES=64;
static  const int MAXPARTGROUPS = 8;


//---------------------------------------------------------
//   ExportLy
//---------------------------------------------------------

class ExportLy {
  Score* score;
  QFile f;
  QTextStream os;
  int level;        // indent level
  int curTicks;
  Direction stemDirection;
  int indx;

  int  n, z1, z2, z3, z4; //timesignatures
  int barlen;
  bool slur;
  bool pickup;
  bool rehearsalnumbers;
  bool donefirst; //to prevent doing things in first ordinary bar which are already done in pickupbar
  bool graceswitch, gracebeam;
  int gracecount;
  int prevpitch, staffpitch, chordpitch, oktavdiff;
  int measurenumber, lastind, taktnr;
  bool repeatactive;
  bool firstalt,secondalt;
  enum voltatype {startending, endending, startrepeat, endrepeat, bothrepeat, doublebar, brokenbar, endbar, none};
  struct  voltareg { voltatype voltart; int barno; };
  struct voltareg  voltarray[255];
  int tupletcount;
  bool pianostaff;
  const Slur* slurre[MAX_SLURS];
  bool started[MAX_SLURS];
  int findSlur(const Slur* s) const;
  const char *relativ, *staffrelativ;
  bool voiceActive[VOICES];
  int prevElTick;

  struct lybrackets
  {
    bool piano;
    bool bracestart,brakstart, braceend, brakend;
    int braceno, brakno;
  };

  struct lybrackets lybracks[BRACKSTAVES];
  void bracktest();

  struct staffnameinfo
  {
    QString voicename[VOICES];
    QString  staffid, partname, partshort;
    bool simultaneousvoices;
  };

  struct staffnameinfo staffname[32];

  QString cleannote, prevnote;

  struct InstructionAnchor
  {
    Element* instruct;  // the element containing the instruction
    Element* anchor;    // the element it is attached to
    bool     start;     // whether it is attached to start or end
    int      tick;      // the timestamp
  };

  int nextAnchor;
  struct InstructionAnchor anker;
  struct InstructionAnchor anchors[512];

  QString voicebuffer;
  QTextStream out;

  void storeAnchor(struct InstructionAnchor);
  void initAnchors();
  void removeAnchor(int);
  void resetAnchor(struct InstructionAnchor &ank);
  bool findSpecificMatchInMeasure(int, Staff*, Measure*, int, int);
  bool findMatchInMeasure(int, Staff*, Measure*, int, int);
  bool findMatchInPart(int, Staff*, Score*, Part*, int, int);
  bool findSpecificMatchInPart(int, Staff*, Score*, int, int);


  void symbol(Symbol * sym);
  void tempoText(TempoText *);
  void words(Text *);
  void hairpin(Hairpin* hp, int tick);
  void ottava(Ottava* ot, int tick);
  void pedal(Pedal* pd, int tick);
  void dynamic(Dynamic* dyn);
  void textLine(TextLine* tl, int tick);
  //from xml's class directionhandler:
  void buildInstructionList(Part* p, int strack, int etrack);
  void buildInstructionList(Measure* m, int strack, int etrack);
  void handleElement(Element* el, bool start);
  void instructionJump(Jump*);
  void instructionMarker(Marker*);

  void indent(); //buffer-string
  void indentF(); //file
  int getLen(int ticks, int* dots);
  void writeLen(int);
  QString tpc2name(int tpc);
  QString tpc2purename(int tpc);

  void writeScore();
  void writeVoiceMeasure(Measure*, Staff*, int, int);
  void writeKeySig(int);
  void writeTimeSig(TimeSig*);
  void writeClef(int);
  void writeChord(Chord*);
  void writeRest(int, int);
  void findVolta();
  void writeBarline(Measure *);
  int  voltaCheckBar(Measure *, int);
  void writeVolta(int, int);
  void findTuplets(ChordRest*);
  void writeArticulation(Chord*);
  void writeScoreBlock();
  void checkSlur(Chord* chord);
  void doSlurStart(Chord* chord);
  void doSlurStop(Chord* chord);
  void initBrackets();
  void brackRegister(int, int, int, bool, bool);
  void findBrackets();

public:
  ExportLy(Score* s)
  {
    score  = s;
    level  = 0;
    curTicks = division;
    slur   = false;
    stemDirection = AUTO;
  }
  bool write(const QString& name);
};


//---------------------------------------------------------
// abs num value
//---------------------------------------------------------
int numval(int num)
{  if (num <0) return -num;
  return num;
}


//---------------------------------------------------------
// initBrackets -- init array of brackets and braces info
//---------------------------------------------------------

void ExportLy::initBrackets()
{
  for (int i = 0; i < BRACKSTAVES; ++i)     //init bracket-array
    {
      lybracks[i].piano=false;
      lybracks[i].bracestart=false;
      lybracks[i].brakstart=false;
      lybracks[i].braceend=false;
      lybracks[i].brakend=false;
      lybracks[i].braceno=0;
      lybracks[i].brakno=0;
    }
}



//----------------------------------------------------------------
//   brackRegister register where partGroup Start, and whether brace,
//   bracket or pianostaff
//----------------------------------------------------------------

void ExportLy::brackRegister(int brnumber, int bratype, int staffnr, bool start, bool end)

{
  QString br = "";
  switch(bratype)
    {
    case BRACKET_NORMAL:
      if (start) lybracks[staffnr].brakstart=true;
      //else lybracks[staffnr].brakstart=false;
      if (end) lybracks[staffnr].brakend=true;
      //      else lybracks[staffnr].brakend=false;
      lybracks[staffnr].brakno=brnumber;
      break;
    case BRACKET_AKKOLADE:
      if (start) lybracks[staffnr].bracestart=true;
      //      else lybracks[staffnr].bracestart=false;
      if (end) lybracks[staffnr].braceend=true;
      //      else lybracks[staffnr].braceend=false;
      lybracks[staffnr].braceno=brnumber;
      break;
    case -1: //piano-staff: lilypond makes rigid distance between
	     //staffs to allow cross-staff beaming.
      lybracks[staffnr].piano=true;
      if (start) lybracks[staffnr].bracestart=true;
      if (end) lybracks[staffnr].braceend=true;
      lybracks[staffnr].braceno=brnumber;
      break;
    default:
      printf("bracket subtype %d not understood\n", bratype);
    }
}


//-------------------------------------------------------------
// findBrackets
// run thru parts and staffs to find start and end of braces and brackets
//---------------------------------------------------------------

void ExportLy::findBrackets()
{
  initBrackets();
  char groupnumber;
  groupnumber=1;
  const QList<Part*>* il = score->parts();  //list of parts

  for (int partnumber = 0; partnumber < il->size(); ++partnumber)  //run thru list of parts
    {
      Part* part = il->at(partnumber);
      if (part->nstaves() == 2) pianostaff=true;
      for (int stavno = 0; stavno < part->nstaves(); stavno++) //run thru list of staves in part.
	{
	  if (pianostaff)
	    {
	      if (stavno==0)
		{
		  brackRegister(groupnumber, -1, partnumber+stavno, true, false);
		}
	      if (stavno==1)
		{
		  brackRegister(groupnumber, -1, partnumber+stavno, false, true);
		  pianostaff=false;
		}
	    }
	  else //not pianostaff
	    {
	      Staff* st = part->staff(stavno);
	      if (st)
		{
		  for (int braclev= 0; braclev < st->bracketLevels(); braclev++) //run thru bracketlevels of staff
		    {
		      if (st->bracket(braclev) != NO_BRACKET) //if bracket
			{
			  groupnumber++;
			  if (groupnumber < MAXPARTGROUPS)
			    { //brackRegister(bracketnumber, brackettype, staffnr, start, end)
			      brackRegister(groupnumber, st->bracket(braclev), partnumber, true, false);
			      brackRegister(groupnumber,st->bracket(braclev), partnumber-1+st->bracketSpan(braclev), false, true);
			    }
			}//end of if bracket.
		    }//end of bracket-levels of staff
		}//end if staff.
	    } // end of else:not pianostaff
	}//end of stafflist
    }//end of parts-list
}//end of findBrackets;



void ExportLy::bracktest()
{
  int i=0;
  for (i=0; i<10; i++)
    {
      cout << "stavnr: " << i << "   braceno: " << (int)lybracks[i].braceno << "   brackno: " << (int)lybracks[i].brakno;
      cout << "   bracestart: " << (int)lybracks[i].bracestart << "   brakstart: " << (int)lybracks[i].brakstart;
      cout << "   braceend: " << (int)lybracks[i].braceend << "   brackend: " << (int) lybracks[i].brakend << "\n";
    }
}



//-------------------------------------------------------
// instructionJump
//--------------------------------------------------------

void ExportLy::instructionJump(Jump* jp)
{
  int jtp = jp->jumpType();
  QString words = "";

  if (jtp == JUMP_DC)
    {
      if (jp->getText() == "")
	words = "D.C.";
      else
	words = jp->getText();
    }
  else if (jtp == JUMP_DC_AL_FINE)
    {
      if (jp->getText() == "")
	words = "D.C. al Fine";
      else
	words = jp->getText();
    }
  else if (jtp == JUMP_DC_AL_CODA)
    {
      if (jp->getText() == "")
	words = "D.C. al Coda";
      else
	words = jp->getText();
    }
  else if (jtp == JUMP_DS_AL_CODA)
    {
      if (jp->getText() == "")
	words = "D.S. al Coda";
      else
	words = jp->getText();
    }
  else if (jtp == JUMP_DS_AL_FINE)
    {
      if (jp->getText() == "")
	words = "D.S. al Fine";
      else
	words = jp->getText();
      if (jp->jumpTo() == "") {
             }
	}
  else if (jtp == JUMP_DS)
    {
      words = "D.S.";
    }
  else
    printf("jump type=%d not implemented\n", jtp);

  out << "\\mark " << "\" << words << \" ";
}



//---------------------------------------------------------
//   instructionMarker -- write marker
//---------------------------------------------------------

void ExportLy::instructionMarker(Marker* m)
{
  int mtp = m->markerType();
  QString words = "";
  QString type  = "";

  if (mtp == MARKER_CODA)
    {
      type = "coda";
    }
  else if (mtp == MARKER_SEGNO)
    {
      type = "segno";
    }
  else if (mtp == MARKER_FINE)
    {
      words = "Fine";
    }
  else if (mtp == MARKER_TOCODA)
    {
      if (m->getText() == "")
	words = "To Coda";
      else
	words = m->getText();
    }
  else
    printf("marker type=%d not implemented\n", mtp);
  if (words=="")
    out << "\\mark \\markup { \\musicglyph #\"scripts." << type<<"\"}";
  else
    out << "\\mark \"" << words << "\"";

}



//---------------------------------------------------------
//   symbol
//---------------------------------------------------------

void ExportLy::symbol(Symbol* sym)
      {
      QString name = symbols[sym->sym()].name();
      if (name == "pedal ped")
	out << " \\sustainOn ";
      else if (name == "pedalasterisk")
	out << " \\sustainOff ";
      else {
            printf("ExportLy::symbol(): %s not supported", name.toLatin1().data());
            return;
            }
      }


//---------------------------------------------------------
//   tempoText
//---------------------------------------------------------

void ExportLy::tempoText(TempoText* text)
      {
	out << "^\\markup {" << text->getText() << "}";
      }



//---------------------------------------------------------
//   words
//---------------------------------------------------------

void ExportLy::words(Text* text)
     {
       char* c;
       char a;
       //todo: find exact mscore-position of text and not only anchorpoint, and position accordingly in lily.
	if (text->subtypeName()== "RehearsalMark")
	  {
	    out << "\\mark\\default ";
	    //There must be an easier way to check if QString is number????
	    //is there a .toInteger()????
	    c=text->getText().toAscii().data();
	    a=(char)c[0];
	    if ((a>= 48) && (a<= 57))
	      {
		rehearsalnumbers=true;
	      }
	  }
	else
	  out << "^\\markup {" << text->getText() << "}";
      }



//---------------------------------------------------------
//   hairpin
//---------------------------------------------------------

void ExportLy::hairpin(Hairpin* hp, int tick)
      {
	int art=2;
	art=hp->subtype();
	if (hp->tick() == tick)
	  {
	    if (art == 0) //diminuendo
	      out << "\\< ";
	    if (art == 1) //crescendo
	      out << "\\> ";
	  }
	else   out << "\\! "; //end of hairpin
      }

//---------------------------------------------------------
//   ottava
// <octave-shift type="down" size="8" relative-y="14"/>
// <octave-shift type="stop" size="8"/>
//---------------------------------------------------------

void ExportLy::ottava(Ottava* ot, int tick)
      {
      int st = ot->subtype();
      if (ot->tick() == tick) {
            const char* sz = 0;
            switch(st) {
                  case 0:
                        sz = "-1";
                        break;
                  case 1:
                        sz = "-2";
                        break;
                  case 2:
                        sz = "1";
                        break;
                  case 3:
                        sz = "2";
                        break;
                  default:
                        printf("ottava subtype %d not understood\n", st);
                  }
            if (sz)
	      out << "#(set-octaviation " << sz << ") ";
            }
      else {
	if (st == 0)
	  out << "#(set-octaviation 0)";
            }
      }

//---------------------------------------------------------
//   pedal
//---------------------------------------------------------

void ExportLy::pedal(Pedal* pd, int tick)
      {
      if (pd->tick() == tick)
	{
	  out << "\\override Staff.pedalSustainStyle #\'bracket ";
	  out << "\\sustainDown ";
	}
      else
	out << "\\sustainUp ";
      }



//---------------------------------------------------------
//   dynamic
//---------------------------------------------------------
void ExportLy::dynamic(Dynamic* dyn)
{
  QString t = dyn->getText();
  if (t == "p" || t == "pp" || t == "ppp" || t == "pppp" || t == "ppppp" || t == "pppppp"
      || t == "f" || t == "ff" || t == "fff" || t == "ffff" || t == "fffff" || t == "ffffff"
      || t == "mp" || t == "mf" || t == "sf" || t == "sfp" || t == "sfpp" || t == "fp"
      || t == "rf" || t == "rfz" || t == "sfz" || t == "sffz" || t == "fz" || t == "sff")
    {
      out << "\\" << t.toLatin1().data() << " ";
    }
  else if (t == "m" || t == "z")
    {
      out << "\\"<< t.toLatin1().data() << " ";
    }
    else
      out << "_\\markup{"<< t.toLatin1().data() << "} ";
}//end dynamic



//---------------------------------------------------------
//   textLine
//---------------------------------------------------------

void ExportLy::textLine(TextLine* /*tl*/, int /*tick*/)
      {
	printf("textline\n");
//       QString rest;
//       QPointF p;

//       QString lineEnd = "none";
//       QString type;
//       int offs;
//       int n = 0;
//       if (tl->tick() == tick) {
//             QString lineType;
//             switch (tl->lineStyle()) {
//                   case Qt::SolidLine:
//                         lineType = "solid";
//                         break;
//                   case Qt::DashLine:
//                         lineType = "dashed";
//                         break;
//                   case Qt::DotLine:
//                         lineType = "dotted";
//                         break;
//                   default:
//                         lineType = "solid";
//                   }
//             rest += QString(" line-type=\"%1\"").arg(lineType);
//             p = tl->lineSegments().first()->userOff();
//             offs = tl->mxmlOff();
//             type = "start";
//             }
//       else {
//             if (tl->hook()) {
//                   lineEnd = tl->hookUp() ? "up" : "down";
//                   rest += QString(" end-length=\"%1\"").arg(tl->hookHeight().val() * 10);
//                   }
//             // userOff2 is relative to userOff in MuseScore
//             p = tl->lineSegments().last()->userOff2() + tl->lineSegments().first()->userOff();
//             offs = tl->mxmlOff2();
//             type = "stop";
//             }

//       n = findBracket(tl);
//       if (n >= 0)
//             bracket[n] = 0;
//       else {
//             n = findBracket(0);
//             bracket[n] = tl;
//             }

//       if (p.x() != 0)
//             rest += QString(" default-x=\"%1\"").arg(p.x() * 10);
//       if (p.y() != 0)
//             rest += QString(" default-y=\"%1\"").arg(p.y() * -10);

//       attr.doAttr(xml, false);
//       xml.stag(QString("direction placement=\"%1\"").arg((p.y() > 0.0) ? "below" : "above"));
//       if (tl->hasText()) {
//             xml.stag("direction-type");
//             xml.tag("words", tl->text());
//             xml.etag();
//             }
//       xml.stag("direction-type");
//       xml.tagE(QString("bracket type=\"%1\" number=\"%2\" line-end=\"%3\"%4").arg(type, QString::number(n + 1), lineEnd, rest));
//       xml.etag();
//       if (offs)
//             xml.tag("offset", offs);
//       if (staff)
//             xml.tag("staff", staff);
//       xml.etag();
      }


//--------------------------------------------------------
//  initAnchors
//--------------------------------------------------------
void ExportLy::initAnchors()
{
  int i;
  for (i=0; i<512; i++)
    resetAnchor(anchors[i]);
}



//--------------------------------------------------------
//   resetAnchor
//--------------------------------------------------------
void ExportLy::resetAnchor(struct InstructionAnchor &ank)
{
  ank.instruct=0;
  ank.anchor=0;
  ank.start=false;
  ank.tick=0;
}

//---------------------------------------------------------
//   deleteAnchor
//---------------------------------------------------------
void ExportLy::removeAnchor(int ankind)
{
  int i;
  resetAnchor(anchors[ankind]);
  for (i=ankind; i<=nextAnchor; i++)
    anchors[i]=anchors[i+1];
  resetAnchor(anchors[nextAnchor]);
  nextAnchor=nextAnchor-1;
}

//---------------------------------------------------------
//   storeAnchor
//---------------------------------------------------------

void ExportLy::storeAnchor(struct InstructionAnchor a)
      {
      if (nextAnchor < 512)
	{
	  anchors[nextAnchor++] = a;
	}
      else
            printf("InstructionHandler: too many instructions\n");
      resetAnchor(anker);
      }


//---------------------------------------------------------
//   handleElement -- handle all instructions attached to one specific element
//---------------------------------------------------------

void ExportLy::handleElement(Element* el, bool start)
{
  int i = 0;
  for (i = 0; i<=nextAnchor; i++)//run thru filled part of list
    {
      //anchored to start of this element:
      if (anchors[i].anchor != 0 and anchors[i].anchor==el && anchors[i].start == start)
       {
	  Element* instruction = anchors[i].instruct;
	  ElementType instructiontype = instruction->type();

	  switch(instructiontype)
	    {
	    case MARKER:
	      printf("MARKER\n");
	      instructionMarker((Marker*) instruction);
	      break;
	    case JUMP:
	      printf("JUMP\n");
	      instructionJump((Jump*) instruction);
	      break;
	    case SYMBOL:
	      printf("SYMBOL\n");
	      symbol((Symbol *) instruction);
	      break;
	    case TEMPO_TEXT:
	      printf("TEMPOTEXT MEASURE\n");
	      tempoText((TempoText*) instruction);
	      break;
	    case STAFF_TEXT:
	    case TEXT:
	      printf("TEXT\n");
	      words((Text*) instruction);
	      break;
	    case DYNAMIC:
	      printf("funnet DYNAMIC i ankerliste\n");
	      dynamic((Dynamic*) instruction);
	      break;
	    case HAIRPIN:
	      printf("funnet HÅRNÅL i ankerliste\n");
	      hairpin((Hairpin*) instruction, anchors[i].tick);
	      break;
	    case OTTAVA:
	      ottava((Ottava*) instruction, anchors[i].tick);
	      break;
	    case PEDAL:
	      pedal((Pedal*) instruction, anchors[i].tick);
	      break;
	    case TEXTLINE:
	      textLine((TextLine*) instruction, anchors[i].tick);
	      break;
	    default:
	      printf("InstructionHandler::handleElement: direction type %s at tick %d not implemented\n",
		     Element::name(instruction->type()), anchors[i].tick);
	      break;
	    }
	  removeAnchor(i);
	  //	  resetAnchor(anchors[i]); //discard used anchors to avoid unnecessary doubles.
	}
    } //foreach position i anchor-array.
}


// void ExportLy::anchorJumporMarker(Measure* m, int tick)
// {
//   int i=0, j=0;
//   measurenumber=m->no()+1;
//   if (tick==m-tick()) //start of measure: register last in previous
//     {
//       while (voltarray[i]<measurenumber-1) i++;
//       for (j=lastind, j>=i, j--)
// 	{
// 	  voltarray[j+1]=voltarray[j];
// 	}
//       voltarray[i].voltart=none;
//       voltarray[i].barno=0;
// }

//---------------------------------------------------------
//   findSpecificMatchInMeasure -- find chord or rest in measure
//     starting or ending at tick
//---------------------------------------------------------

bool ExportLy::findSpecificMatchInMeasure(int tick, Staff* stf, Measure* m, int strack, int etrack)
{
  bool found=false;
  for (int st = strack; st < etrack; ++st)
    {
      for (Segment* seg = m->first(); seg; seg = seg->next())
	{
	  Element* el = seg->element(st);
	  if (!el) continue;
	  if ((el->isChordRest()) && (el->staff() == stf) && ((el->tick() >= tick)))
	    {
	      if (el->tick() > tick) tick=prevElTick;
	      anker.anchor=el;
	      found=true;
	      anker.tick=tick;
	      anker.start=true;
	      goto fertig;
	    }
	    prevElTick=el->tick();
	 }
    }
 fertig:
 return found;
}


//---------------------------------------------------------
//   findMatchInMeasure -- find chord or rest in measure
//---------------------------------------------------------

bool ExportLy::findMatchInMeasure(int tick, Staff* st, Measure* m, int strack, int etrack)
      {
	bool found=false;
	found = findSpecificMatchInMeasure(tick, st, m, strack, etrack);
	return found;
      }

//---------------------------------------------------------
//   findSpecificMatchInPart -- find chord or rest in part
//     starting or ending at tick
//---------------------------------------------------------

bool ExportLy::findSpecificMatchInPart(int tick, Staff* st, Score* sc, int strack, int etrack)
{
  bool found=false;
  for (MeasureBase* mb = sc->measures()->first(); mb; mb = mb->next())
    {
      if (mb->type() != MEASURE)
	continue;
      Measure* m = (Measure*)mb;
      found=findSpecificMatchInMeasure(tick, st, m, strack, etrack);
      if (found) break;
     }
return found;
}

//---------------------------------------------------------
//   findMatchInPart -- find chord or rest in part
//---------------------------------------------------------

bool ExportLy::findMatchInPart(int tick, Staff* st, Score* sc, Part*, int strack, int etrack)
      {
	bool found=false;
	found = findSpecificMatchInPart(tick, st, sc, strack, etrack);
	return found;
      }

//---------------------------------------------------------
//   buildInstructionList -- associate instruction (measure relative elements)
//     with elements in segments to enable writing at the correct position
//     in the output stream. Called once for every part to handle all part-level elements.
//---------------------------------------------------------

void ExportLy::buildInstructionList(Part* p, int strack, int etrack)
{
  // part-level elements stored in the score layout

  prevElTick=0;
  foreach(Element* instruction, *(score->gel()))
    {
      bool found=false;
      switch(instruction->type())
	{
	case JUMP:
	  printf("score JUMP found at tick: %d\n", instruction->tick());
	case MARKER:
	  printf("score MARKER found at tick: %d\n", instruction->tick());
	case HAIRPIN:
	case OTTAVA:
	case PEDAL:
	case DYNAMIC:
	case TEXTLINE:
	  {
	    SLine* sl = (SLine*) instruction;
	    found=findMatchInPart(sl->tick(), sl->staff(), score, p, strack, etrack);
	    if (found)
	      {
		anker.instruct=instruction;
		storeAnchor(anker);
	      }
	    found=findMatchInPart(sl->tick2(), sl->staff(), score, p, strack, etrack);
	    if (found)
	      {
		anker.instruct=instruction;
		storeAnchor(anker);
	      }
	  break;
	  }
	  default:
	  // all others ignored
	  //printf(" instruction type %s not implemented\n", Element::name(instruction->type()));
	  break;
	}
    }// end foreach element....


  // part-level elements stored in measures
  for (MeasureBase* mb = score->measures()->first(); mb; mb = mb->next())
    {
      if (mb->type() != MEASURE)
	continue;
      Measure* m = (Measure*)mb;
      buildInstructionList(m, strack, etrack);
    }
}//end: buildInstructionList


//---------------------------------------------------------
//   buildInstructionList -- associate instruction (measure relative elements)
//     with elements in segments to enable writing at the correct position
//     in the output stream. Called once for every measure to handle either
//     part-level or measure-level elements.
//---------------------------------------------------------

void ExportLy::buildInstructionList(Measure* m, int strack, int etrack)
{
  bool found=false;
  // loop over all measure relative elements in this measure
  for (ciElement ci = m->el()->begin(); ci != m->el()->end(); ++ci)
    {
      Element* instruction = *ci;
      switch(instruction->type())
	{
	case JUMP:
	  printf("measure JUMP foundat tick: %d\n", instruction->tick());
	case MARKER:
	  printf("measure MARKER foundat tick: %d\n", instruction->tick());
	  //	  anchorJumpOrMarker(m,instruction);
	  break;
	case DYNAMIC:
	case SYMBOL:
	case TEMPO_TEXT:
	case TEXT:
	case HAIRPIN:
	case OTTAVA:
	case PEDAL:
	case STAFF_TEXT:
	  found = findMatchInMeasure(instruction->tick(), instruction->staff(), m, strack, etrack);
	  if (found)
	    {
	      anker.instruct=instruction;
	      storeAnchor(anker);
	      printf("put instruction type %d in anchorlist\n", instruction->type());
	    }
	  //else if (!found) printf("element not anchored?!: %d\n", instruction->type());
	  break;
	default:
	  printf("Intet measurerelevant anker\n");
	}
    }
}

//---------------------------------------------------------
//   indent  -- buffer
//---------------------------------------------------------

void ExportLy::indent()
{
  for (int i = 0; i < level; ++i)
    out << "    ";
}


//---------------------------------------------------------
//   indent  -- file
//---------------------------------------------------------

void ExportLy::indentF()
{
  for (int i = 0; i < level; ++i)
    os << "    ";
}


//-------------------------------------
// Find tuplets Note
//-------------------------------------

void ExportLy::findTuplets(ChordRest* cr)
      {
      Tuplet* t = cr->tuplet();

      // TODO: Tuplet() has changed; please check

      if (t) {
            if (tupletcount == 0) {
                  int actNotes   = t->actualNotes();
                  int nrmNotes   = t->normalNotes();
                  int baselength = t->ticks() / nrmNotes;
                  int thislength = cr->ticks();
                  tupletcount    = nrmNotes * baselength - thislength;
                  out << "\\times " <<  nrmNotes << "/" << actNotes << "{" ;
                  }
            else if (tupletcount > 1) {
                  int thislength = cr->ticks();
                  tupletcount    = tupletcount - thislength;
                  if (tupletcount == 0)
                        tupletcount = -1;
                  }
            }
      }

//-----------------------------------------------------
//  voltaCheckBar
//
// supplements findVolta and called from there: check barlinetypes in
// addition to endings
//------------------------------------------------------
int ExportLy::voltaCheckBar(Measure* meas, int i)
{

  int blt = meas->endBarLineType();

  switch(blt)
    {
    case START_REPEAT:
      i++;
      voltarray[i].voltart=startrepeat;
      voltarray[i].barno=taktnr;
      break;
    case END_REPEAT:
      i++;
      voltarray[i].voltart=endrepeat;
      voltarray[i].barno=taktnr;
      break;
    case END_START_REPEAT:
      i++;
      voltarray[i].voltart=bothrepeat;
      voltarray[i].barno=taktnr;
      break;
    case END_BAR:
      i++;
      voltarray[i].voltart=endbar;
      voltarray[i].barno=taktnr;
      break;
    case DOUBLE_BAR:
      i++;
      voltarray[i].voltart=doublebar;
      voltarray[i].barno=taktnr;
    case BROKEN_BAR:
      i++;
      voltarray[i].voltart=brokenbar;
      voltarray[i].barno=taktnr;
    default:
      break;
    }//switch

  bool rs = meas->repeatFlags() & RepeatStart;
  if (rs)
    {
      i++;
      voltarray[i].voltart=startrepeat;
      voltarray[i].barno=taktnr-1; //set as last element in previous measure.
    }
  return i;
}//end voltacheckbarline



//------------------------------------------------------------------
//   findVolta -- find and register volta and repeats in entire piece,
//   register them in voltarray for later use in writeVolta.
//------------------------------------------------------------------

void  ExportLy::findVolta()
{
  taktnr=0;
  lastind=0;
  int i=0;

  for (i=0; i<255; i++)
    {
      voltarray[i].voltart=none;
      voltarray[i].barno=0;
    }

  i=0;

  for (MeasureBase * m=score->first(); m; m=m->next())
    {// for all measures
      if (m->type() !=MEASURE )
	continue;
      ++taktnr;
      foreach(Element* el, *(m->score()->gel()))
	//walk thru all elements of measure
	{
	  if (el->type() == VOLTA)
	    {
	      Volta* v = (Volta*) el;

	      if (v->tick() == m->tick()) //If we are at the beginning of the measure
		{
		  i++;
		  //  if (v->subtype() == Volta::VOLTA_CLOSED)
		  // 		    {//lilypond developers have "never seen" last ending closed.
		  // 		      //So they are reluctant to implement it. Final ending is always "open" in lilypond.
		  // 		    }
		  // 		  else if (v->subtype() == Volta::VOLTA_OPEN)
		  // 		    {
		  // 		    }
		  voltarray[i].voltart = startending;
		  voltarray[i].barno=taktnr-1; //register as last element i previous measure
		}
	      if (v->tick2() == m->tick() + m->tickLen()) // if it is at the end of measure
		{
		  i++;
		  voltarray[i].voltart = endending;
		  voltarray[i].barno=taktnr;//last element of this measure
		  // 		  if (v->subtype() == Volta::VOLTA_CLOSED)
		  // 		    {// see comment above.
		  // 		    }
		  // 		  else if (v->subtype() == Volta::VOLTA_OPEN)
		  // 		    {// see comment above.
		  // 		    }

		}
	    }
	}// for all elements
      i=voltaCheckBar((Measure *) m, i);
    }//for all measures
  lastind=i;

  //  for (i=0; i<lastind; i++)
  //  cout << "voltatest: " << voltarray[i].barno << "  " << voltarray[i].voltart <<"\n";
}// end findvolta


//---------------------------------------------------------
//   exportLilypond
//---------------------------------------------------------

bool Score::saveLilypond(const QString& name)
{
  ExportLy em(this);
  return em.write(name);
}


//---------------------------------------------------------
//   writeClef
//---------------------------------------------------------

void ExportLy::writeClef(int clef)
{
  out << "\\clef ";
  switch(clef) {
  case CLEF_G:      out << "treble\n";         break;
  case CLEF_F:      out << "bass\n";           break;
  case CLEF_G1:     out << "\"treble^8\"\n";   break;
  case CLEF_G2:     out << "\"treble^15\"\n";  break;
  case CLEF_G3:     out << "\"treble_8\"\n";   break;
  case CLEF_F8:     out << "\"bass_8\"\n";     break;
  case CLEF_F15:    out << "\"bass_15\"\n";    break;
  case CLEF_F_B:    out << "bass\n";           break;
  case CLEF_F_C:    out << "bass\n";           break;
  case CLEF_C1:     out <<  "soprano\n";       break;
  case CLEF_C2:     out <<  "mezzo-soprano\n"; break;
  case CLEF_C3:     out <<  "alto\n";          break;
  case CLEF_C4:     out <<  "tenor\n";         break;
  case CLEF_TAB:    out <<  "tab\n";           break;
  case CLEF_PERC:   out <<  "percussion\n";    break;
  }

}

//---------------------------------------------------------
//   writeTimeSig
//---------------------------------------------------------

void ExportLy::writeTimeSig(TimeSig* sig)
{
  int st = sig->subtype();
  sig->getSig(&n, &z1, &z2, &z3, &z4);
  //lilypond writes 4/4 as C by default, so only check for cut.
  if (st == TSIG_ALLA_BREVE)
    {
      z1=2;
      n=2;
      // 2/2 automatically written as alla breve by lily.
    }
  indent();
  out << "\\time " << z1 << "/" << n << " ";
}

//---------------------------------------------------------
//   writeKeySig
//---------------------------------------------------------

void ExportLy::writeKeySig(int st)
{
  st = char(st & 0xff);
  indent();
  out << "\\key ";
  switch(st) {
  case 7:  out << "cis"; break;
  case 6:  out << "fis"; break;
  case 5:  out << "b";   break;
  case 4:  out << "e";   break;
  case 3:  out << "a";   break;
  case 2:  out << "d";   break;
  case 1:  out << "g";   break;
  case 0:  out << "c";   break;
  case -7: out << "ces"; break;
  case -6: out << "ges"; break;
  case -5: out << "des"; break;
  case -4: out << "as";  break;
  case -3: out << "es";  break;
  case -2: out << "bes"; break;
  case -1: out << "f";   break;
  default:
    printf("illegal key %d\n", st);
    break;
  }
  out << " \\major \n";
}

//---------------------------------------------------------
//   tpc2name
//---------------------------------------------------------

QString ExportLy::tpc2name(int tpc)
{
  const char names[] = "fcgdaeb";
  int acc   = ((tpc+1) / 7) - 2;
  QString s(names[(tpc + 1) % 7]);
  switch(acc) {
  case -2: s += "eses"; break;
  case -1: s += "es";  break;
  case  1: s += "is";  break;
  case  2: s += "isis"; break;
  case  0: break;
  default: s += "??"; break;
  }
  return s;
}


//---------------------------------------------------------
//   tpc2purename
//---------------------------------------------------------

QString ExportLy::tpc2purename(int tpc)
{
  const char names[] = "fcgdaeb";
  QString s(names[(tpc + 1) % 7]);
  return s;
}


//--------------------------------------------------------
//  Slur functions, stolen from exportxml.cpp:
//
//---------------------------------------------------------
//   findSlur -- get index of slur in slur table
//   return -1 if not found
//---------------------------------------------------------

int ExportLy::findSlur(const Slur* s) const
{
  for (int i = 0; i < 8; ++i)
    if (slurre[i] == s) return i;
  return -1;
}

//---------------------------------------------------------
//   doSlurStart
//---------------------------------------------------------

void ExportLy::doSlurStart(Chord* chord)
{
  foreach(const Slur* s, chord->slurFor())
    {
      int i = findSlur(s);
      if (i >= 0) {
	slurre[i] = 0;
	started[i] = false;
	if (s->slurDirection() == UP) out << "^";
	out << "(";
      }
      else {
	i = findSlur(0);
	if (i >= 0) {
	  slurre[i] = s;
	  started[i] = true;
	  out << "(";
	}
	else
	  printf("no free slur slot");
      }
    }
}


//---------------------------------------------------------
//   doSlurStop
//   From exportxml.cpp:
//-------------------------------------------
void ExportLy::doSlurStop(Chord* chord)
{
  foreach(const Slur* s, chord->slurBack())
    {
      // check if on slur list
      int i = findSlur(s);
      if (i < 0) {
	// if not, find free slot to store it
	i = findSlur(0);
	if (i >= 0) {
	  slurre[i] = s;
	  started[i] = false;
	  out << ")";
	}
	else
	  printf("no free slur slot");
      }
    }
  for (int i = 0; i < 8; ++i)
    {
      if (slurre[i])
	{
	  if  (slurre[i]->endElement() == chord)
	    {
	      if (started[i]) {
		slurre[i] = 0;
		started[i] = false;
		out << ")";
	      }
	    }
	}
    }
}

//-------------------------
// checkSlur
//-------------------------
void ExportLy::checkSlur(Chord* chord)
{
  //init array:
  for (int i = 0; i < 8; ++i)
    {
      slurre[i] = 0;
      started[i] = false;
    }
  doSlurStop(chord);
  doSlurStart(chord);
}


//-----------------------------------
// helper routine for writeScore
// -- called from there
//-----------------------------------

void ExportLy::writeArticulation(Chord* c)
{
  foreach(Articulation* a, *c->getArticulations())
    {
      switch(a->subtype())
	{
	case UfermataSym:
	  out << "\\fermata";
	  break;
	case DfermataSym:
	  out << "_\\fermata";
	  break;
	case ThumbSym:
	  out << "\\thumb";
	  break;
	case SforzatoaccentSym:
	  out << "->";
	  break;
	case EspressivoSym:
	  out << "\\espressivo";
	  break;
	case StaccatoSym:
	  out << "-.";
	  break;
	case UstaccatissimoSym:
	  out << "-|";
	  break;
	case DstaccatissimoSym:
	  out << "_|";
	  break;
	case TenutoSym:
	  out << "--";
	  break;
	case UportatoSym:
	  out << "-_";
	  break;
	case DportatoSym:
	  out << "__";
	  break;
	case UmarcatoSym:
	  out << "-^";
	  break;
	case DmarcatoSym:
	  out << "_^";
	  break;
	case OuvertSym:
	  out << "\\open";
	  break;
	case PlusstopSym:
	  out << "-+";
	  break;
	case UpbowSym:
	  out << "\\upbow";
	  break;
	case DownbowSym:
	  out << "\\downbow";
	  break;
	case ReverseturnSym:
	  out << "\\reverseturn";
	  break;
	case TurnSym:
	  out << "\\turn";
	  break;
	case TrillSym:
	  out << "\\trill";
	  break;
	case PrallSym:
	  out << "\\prall";
	  break;
	case MordentSym:
	  out << "\\mordent";
	  break;
	case PrallPrallSym:
	  out << "\\prallprall";
	  break;
	case PrallMordentSym:
	  out << "\\prallmordent";
	  break;
	case UpPrallSym:
	  out << "\\prallup";
	  break;
	case DownPrallSym:
	  out << "\\pralldown";
	  break;
	case UpMordentSym:
	  out << "\\upmordent";
	  break;
	case DownMordentSym:
	  out << "\\downmordent";
	  break;
	default:
	  printf("unsupported note attribute %d\n", a->subtype());
	  break;
	}// end switch
    }// end foreach
}// end writeArticulation();



//---------------------------------------------------------
//   writeChord
//---------------------------------------------------------

void ExportLy::writeChord(Chord* c)
{
  bool graceslur=false;
  int  purepitch;
  QString purename, chordnote;
  int pitchlist[12];
  int j=0;
  for (j=0; j<12; j++) pitchlist[j]=0;

  // We only export stem directions for gracenotes.

  if (c->beam() == 0 || c->beam()->elements().front() == c)
    {
      Direction d = c->stemDirection();
      if (d != stemDirection)
	{
	  stemDirection = d;
	  if ((d == UP) and (graceswitch == true))
	    out << "\\stemUp ";
	  else if ((d == DOWN)  and (graceswitch == true))
	    out << "\\stemDown ";
	//   else if (d == AUTO)
// 	    {
// 	      if (graceswitch == true)
// 		{
// 		  out << "\\stemNeutral "; // we set this at the end of graces anyway.
// 		}
// 	    }
	}
    }

  bool tie=false;
  NoteList* nl = c->noteList();

  if (nl->size() > 1)
    {
    out << "<";
    }

  j=0;

  for (iNote i = nl->begin();;)
    {
      Note* n = i->second;
      NoteType gracen;
      gracen = n->noteType();
      switch(gracen)
	{
	case NOTE_INVALID:
	case NOTE_NORMAL: if (graceswitch==true)
	    {
	      graceswitch=false;
	      graceslur=true;
	      gracebeam=false;
	      if (gracecount > 1) out << " ] "; //single graces are not beamed
	      out << " } \\stemNeutral "; //end of grace
	      gracecount=0;
	    }
	  break;
	case NOTE_ACCIACCATURA:
	case NOTE_APPOGGIATURA:
	case NOTE_GRACE4:
	case NOTE_GRACE16:
	case NOTE_GRACE32:
	  if (graceswitch==false)
	    {
	      out << "\\grace{\\stemUp "; //as long as general stemdirecton is unsolved: graces always stemUp.
	      graceswitch=true;
	      gracebeam=false;
	      gracecount=0;
	    }
	  gracecount++;
	  break;
	} //end of switch(gracen)


      findTuplets(n->chord()); //probably causes trouble here, Must be put outside of notelist?

      if (gracecount==2) out << " [ ";
      out << tpc2name(n->tpc());
      if (n->tieFor()) tie=true;
      purepitch = n->pitch();
      purename = tpc2name(n->tpc());  //with -es or -is
      prevnote=cleannote;             //without -es or -is
      cleannote=tpc2purename(n->tpc());//without -es or -is

      if (purename.contains("eses")==1)  purepitch=purepitch+2;
      else if (purename.contains("es")==1)  purepitch=purepitch+1;
      else if (purename.contains("isis")==1) purepitch=purepitch-2;
      else if (purename.contains("is")==1) purepitch=purepitch-1;

      oktavdiff=prevpitch - purepitch;
      int oktreit=numval(oktavdiff);


      while (oktreit > 0)
	{
	  if ((oktavdiff < -6) or ((prevnote=="b") and (oktavdiff < -5)))
	    { //up
		out << "'";
		oktavdiff=oktavdiff+12;
	    }
	    else if ((oktavdiff > 6)  or ((prevnote=="f") and (oktavdiff > 5)))
	      {//down
		out << ",";
		oktavdiff=oktavdiff-12;
	      }
	  oktreit=oktreit-12;
	}


      prevpitch=purepitch;
      pitchlist[j]=purepitch;
      j++;

      if (i == nl->begin())
	{
	  chordpitch=prevpitch;
	  chordnote=cleannote;
	}

      ++i; //number of notes in chord, we progress to next chordnote
      if (i == nl->end())
	break;
      out << " ";
    } //end of notelist = end of chord

  if (nl->size() > 1)
    {
    out << ">"; //endofchord sign
    cleannote=chordnote;
    //if this is a chord, use first note of chord as previous note
    //instead of actual previous note.
    }

  j=0;
  prevpitch=pitchlist[0];
   while (pitchlist[j] !=0)
     {
       if (pitchlist[j]<prevpitch) prevpitch=pitchlist[j];
       j++;
     }

  writeLen(c->tickLen());


  if (graceswitch)
    {
      out << " ( "; //gracenotes always slurred. Remove by hand in lilyfile otherwise.
    }

  if (tie)
    {
      out << "~";
      tie=false;
    }

  writeArticulation(c);
  checkSlur(c);

  if (tupletcount==-1)
    {
      out << " } ";
      tupletcount=0;
    }

  if (graceslur==true)
    {
      out << " ) ";
      graceslur=false;
    }

  out << " ";

}// end of writechord


//---------------------------------------------------------
//   getLen
//---------------------------------------------------------

int ExportLy::getLen(int l, int* dots)
{
  int len  = 4;
  if (l == 16 * division) //longa, whole measure of 4/2-time
    len=-2;
  else if (l == 12 * division) // "6/2" "dotted brevis" used for whole-measure rest in 6/2 time.
    len=-3;
  else if (l == 10 * division) // "5/2"- time, used for whole-measure rest.
    len=-4;
  else if (l == 8 * division) //brevis
    len = -1;
  else if      (l == 6 * division) //dotted whole
    {
      len  = 1;
      *dots = 1;
    }
  else if (l == 5 * division) //doubledotted whole
    {
      len = 1;
      *dots = 2;
    }
  else if (l == 4 * division) //whole
    len = 1;
  else if (l == 3 * division) // dotted half
    {
      len = 2;
      *dots = 1;
    }
  else if (l == ((division/2)*7)) // double-dotted half: 7/8 used for \partial bar.
    {
      len = 2;
      *dots=2;
    }
  else if (l == 2 * division)
    len = 2;
  else if (l == division)
    len = 4;
  else if (l == division *3 /2)
    {
      len=4;
      *dots=1;
    }
  else if (l == division / 2)
    len = 8;
  else if (l == division*3 /4) //dotted 8th
    {
      len = 8;
      *dots=1;
    }
  else if (l == division / 4)
    len = 16;
  else if (l == division / 8)
    len = 32;
  else if (l == division * 3 /8) //dotted 16th.
    {
      len = 16;
      *dots = 1;
    }
  else if (l == division / 16)
    len = 64;
  else if (l == division /32)
    len = 128;
  //triplets, lily uses nominal value surrounded by \times 2/3 {  }
  //so we set len equal to nominal value
  else if (l == division * 4 /3)
    len = 2;
  else if (l == (division * 2)/3)
    len = 4;
  else if (l == division /3)
    len = 8;
  else if (l == division /(3*2))
    len = 16;
  else if (l == division /3*4)
    len = 32;
  else printf("measure: %d, unsupported len %d (%d,%d)\n", measurenumber, l, l/division, l % division);
  return len;
}

//---------------------------------------------------------
//   writeLen
//---------------------------------------------------------

void ExportLy::writeLen(int ticks)
{
  int dots = 0;
  int len = getLen(ticks, &dots);

  if (ticks != curTicks)
    {
      switch (len)
	{
	case -4:
	  out << "2*5 ";
	  break;
	case -3:
	  out << "1.*2 ";
	  break;
	case -2://longa
	  out << "\\longa ";
	    break;
	case -1: //brevis
	  out << "\\breve";
	  break;
	default:
	  out << len;
	  for (int i = 0; i < dots; ++i)
	    out << ".";
	  break;
	}
      curTicks = ticks;
      if (dots>0) curTicks = 0; //first note after dotted: always explicit length
    }
}

//---------------------------------------------------------
//   writeRest
//    type = 0    normal rest
//    type = 1    whole measure rest
//    type = 2    spacer rest
//---------------------------------------------------------

void ExportLy::writeRest(int l, int type)
{
  if (type == 1) //whole measure rest
    {
      out << "R";
      writeLen(l);
     }
  else if (type == 2) //invisible rest
    {
      out << "s";
      writeLen(l);
    }
  else //normal rest
    {
      out << "r";
      writeLen(l);
    }
  out << " ";
}


//--------------------------------------------------------
//   writeVolta
//--------------------------------------------------------
void ExportLy::writeVolta(int measurenumber, int lastind)
{
  bool utgang=false;
  int i=0;

  while ((voltarray[i].barno < measurenumber) and (i<=lastind))
    {
      //find the present measure
      i++;
    }

  if (measurenumber==voltarray[i].barno)
    {
      while (utgang==false)
	{
	  switch(voltarray[i].voltart)
	    {
	    case startrepeat:
	      indent();
	      out << "\\repeat volta 2 {";
	      firstalt=false;
	      secondalt=false;
	      break;
	    case endrepeat:
	      if ((repeatactive==true) and (secondalt==false))
		{
		  out << "} % end of repeatactive\n";
		  // repeatactive=false;
		}
	      indent();
	      break;
	    case bothrepeat:
	      if (firstalt==false)
		{
		  out << "} % end of repeat\n";
		  indent();
		  out << "\\repeat volta 2 {";
		  firstalt=false;
		  secondalt=false;
		  repeatactive=true;
		}
	      break;
	    case doublebar:
	      indent();
	      out << "\\bar \"||\"";
	      break;
	      // 	    case brokenbar:
	      // 	      indent();
	      // 	      out << "\\bar \"| |:\"";
	      // 	      break;
	    case startending:
	      if (firstalt==false)
		{
		  out << "} % end of repeat except alternate endings\n";
		  indent();
		  out << "\\alternative{ {  ";
		  firstalt=true;
		}
	      else
		{
		  out << "{ ";
		  indent();
		  firstalt=false;
		  secondalt=true;
		}
	      break;
	    case endending:
	      if (firstalt)
		{
		  out << "} %close alt1\n";
		  secondalt=true;
		  repeatactive=true;
		}
	      else
		{
		  out << "} } %close alternatives\n";
		  secondalt=false;
		  firstalt=true;
		  repeatactive=false;
		}
	      break;
	    case endbar:
	      out << "\\bar \"|.\"";
	      break;
          default:
	    // case none: printf("strange voltarraycontents?\n");
	      break;
	    }//end switch

	  if (voltarray[i+1].barno==measurenumber)
	    {
	      i++;
	    }
	  else utgang=true;
	}// end of while utgang false;
    }// if barno=measurenumber
}// end writevolta



//---------------------------------------------------------
//   writeVoiceMeasure
//---------------------------------------------------------


void ExportLy::writeVoiceMeasure(Measure* m, Staff* staff, int staffInd, int voice)

{
  int i=0;
  char cvoicenum, cstaffnum;
  if (m->no() >0) out << " | % " << m->no() << "\n" ; //barchecksign and barnumber for previous measure
  measurenumber=m->no()+1;

   if (m->irregular())
     {
       printf("irregular measure, number: %d\n", measurenumber);
     }


  if ((measurenumber==1) and (donefirst==false))
    {
      donefirst=true;
      level=0;
      indent();
      cvoicenum=voice+65;
      cstaffnum= staffInd+65;
      //there must be more elegant ways to do this, but whatever...
      staffname[staffInd].voicename[voice] = staffname[staffInd].partshort;
      staffname[staffInd].voicename[voice].append("voice");
      staffname[staffInd].voicename[voice].append(cstaffnum);
      staffname[staffInd].voicename[voice].append(cvoicenum);
      staffname[staffInd].voicename[voice].prepend("A");
      staffname[staffInd].voicename[voice].remove(QRegExp("[0-9]"));
      staffname[staffInd].voicename[voice].remove(QChar('.'));
      staffname[staffInd].voicename[voice].remove(QChar(' '));

      out << staffname[staffInd].voicename[voice];
      out << " = \\relative c" << relativ;
      indent();
      out << "{\n";
      level++;
      indent();
      if (voice==0)
	{
	  out <<"\\set Staff.instrumentName = #\"" << staffname[staffInd].partname << "\"\n";
	  indent();
	  out << "\\set Staff.shortInstrumentName = #\"" <<staffname[staffInd].partshort << "\"\n";
	  indent();
	  writeClef(staff->clef(0));
	  indent();
	  out << "%staffkeysig\n";
	  //done in first measure anyway: ??
	  writeKeySig(staff->keymap()->key(0));
// 	  score->sigmap->timesig(0, z1, n);
// 	  out << "\\time " << z1<< "/" << n << " \n";
	}

      switch(voice)
	{
	case 0: break;
	case 1:
	  out <<"\\voiceTwo" <<"\n\n";
	  break;
	case 2:
	  out <<"\\voiceThree" <<"\n\n";
	  break;
	case 3:
	  out <<"\\voiceFour" <<"\n\n";
	  break;
	}
      //check for implicit startrepeat before first measure:
      i=0;
      while ((voltarray[i].voltart != startrepeat) and (voltarray[i].voltart != endrepeat)
	     and (voltarray[i].voltart !=bothrepeat) and (i<=lastind))
	{
	  i++;
	}

      if (i<=lastind)
	{
	  if ((voltarray[i].voltart==endrepeat) or (voltarray[i].voltart==bothrepeat))
	    {
	      indent();
	      out << "\\repeat volta 2 { \n";
	      repeatactive=true;
	    }
	}
    }// END if start of first measure

  indent();
  int tick = m->tick();
  int measuretick=0;
  Element* e;


  for(Segment* s = m->first(); s; s = s->next())
    {
      // for all segments in measure. Get element:
      e = s->element(staffInd * VOICES + voice);

      if (!(e == 0 || e->generated()))  voiceActive[voice] = true;
      else  continue;

      switch(e->type())
	{
	case CLEF:
	  writeClef(e->subtype());
	  break;
	case TIMESIG:
	  {
	    out << "%bartimesig: \n";
	    writeTimeSig((TimeSig*)e);
	    out << "\n";
	    barlen=m->tickLen();
	    int nombarlen=z1*division;
	    if (n==8) nombarlen=nombarlen/2;
	    if ((barlen<nombarlen) and (measurenumber==1))
	      {
		pickup=true;
		int punkt=0;
		int partial=getLen(barlen, &punkt);
		indent();
		switch(punkt)
		  {
		  case 0: out << "\\partial " << partial << "\n";
		    break;
		  case 1:
		    {
		      partial=(partial*2);
		      out << "\\partial " << partial << "*3 \n";
		    }
		    break;
		  case 2:
		    {
		      partial=partial*4;
		      out << "\\partial " << partial << "*7 \n";
		      break;
		    }
		  } //end switch (punkt)
	      }
	    curTicks=-1; //we always need explicit length after timesig.
	    indent();
	    break;
	  }
	case KEYSIG:
	  indent();
	  out << "%barkeysig: \n";
	  indent();
	  writeKeySig(e->subtype());
	  curTicks=-1; //feels safe to force explicit length after keysig
	  break;
	case CHORD:
	  {
	    int ntick = e->tick() - tick;
	    if (ntick > 0)
	      {
		writeRest(ntick, 2);//invisible rest: s
		curTicks=-1;
	      }
	    tick += ntick;
	    measuretick=measuretick+ntick;
	    writeChord((Chord*)e);
	    tick += ((Chord*)e)->ticks();
	    measuretick=measuretick+((Chord*)e)->ticks();
	  }
	  break;
	case REST:
	  {
	    findTuplets((ChordRest *) e);
	    int l = ((Rest*)e)->ticks();
	    int mlen=((Rest*)e)->segment()->measure()->tickLen();
	    //	    printf("pauselengde: %d, taktlengde %d \n",l, mlen );
	    if ((l==mlen) or (l==0))
	      {
		l = ((Rest*)e)->segment()->measure()->tickLen();
		writeRest(l, 1); //wholemeasure rest: R
	      }
	    else
	      writeRest(l, 0);//ordinary rest: r
	    tick += l;
	    measuretick=measuretick+l;

	    if (tupletcount==-1)
	      {
		out << " } ";
		tupletcount=0;
	      }
	  }
	  break;
	case MARKER:
	  printf("ordinary elements: Marker found\n");
	  break;
	case BREATH:
	  out << "\\breathe ";
	  break;
	case JUMP:
	  printf("ordinary elements: Jump found\n");
	  break;
	default:
	  //printf("Export Lilypond: unsupported element <%s>\n", e->name());
	  break;
	}
      handleElement(e, true); //check for instructions anchored to element e.
    } //end for all segments


  if (voiceActive[voice] == false)  //fill empty  bar with silent rest
    {
      if ((pickup) and (measurenumber==1))
	{
	  int punkt=0;
	  int partial=getLen(barlen, &punkt);

	  switch(punkt)
	    {
	    case 0: out << "s" << partial << " ";
	      break;
	    case 1:
	      {
		partial=(partial*2);
		out << "s" << partial << "*3 ";
	      }
	      break;
	    case 2:
	      {
		partial=partial*4;
		out << "s" << partial << "*7 ";
		break;
	      }
	    } //end switch (punkt)
	}//end if pickup
      else
	{
	  out << "s";
	  writeLen(barlen);
	  curTicks=-1;
	}
    }//end voice not active
  else  if ((measuretick < barlen) and (measurenumber>1))
      {
	int negative=barlen-measuretick;
	writeRest(negative, 2);
	curTicks=-1;
      }

  writeVolta(measurenumber, lastind);

} //end write VoiceMeasure


//---------------------------------------------------------
//   writeScore
//---------------------------------------------------------

void ExportLy::writeScore()
{
  firstalt=false;
  secondalt=false;
  tupletcount=0;
  char  cpartnum;
  chordpitch=41;
  repeatactive=false;
  int staffInd = 0;
  //int np = score->parts()->size();
  graceswitch=false;
  int voice=0;
  cleannote="c";
  prevnote="c";
  gracecount=0;
  donefirst=false;


  foreach(Part* part, *score->parts())
    {
      nextAnchor=0;
      initAnchors();
      resetAnchor(anker);
      int n = part->staves()->size();
      staffname[staffInd].partname  = part->longName()->getText();
      staffname[staffInd].partshort = part->shortName()->getText();
      curTicks=-1;
      pickup=false;

      //      int staves = part->nstaves();
      int strack = score->staffIdx(part) * VOICES;
      int etrack = strack + n* VOICES;
      buildInstructionList(part, strack, etrack);

      foreach(Staff* staff, *part->staves())
	{

	  out << "\n";

	  switch(staff->clef(0))
	    {
	    case CLEF_G:
	      relativ="'";
	      staffpitch=12*5;
	      break;
	    case CLEF_TAB:
	    case CLEF_PERC:
	    case CLEF_G3:
	    case CLEF_F:
	      relativ="";
	      staffpitch=12*4;
	      break;
	    case CLEF_G1:
	    case CLEF_G2:
	      relativ="''";
	      staffpitch=12*6;
	      break;
	    case CLEF_F_B:
	    case CLEF_F_C:
	    case CLEF_F8:
	      relativ=",";
	      staffpitch=12*3;
	      break;
	    case CLEF_F15:
	      relativ=",,";
	      staffpitch=12*2;
	      break;
	    case CLEF_C1:
	    case CLEF_C2:
	    case CLEF_C3:
	    case CLEF_C4:
	      relativ="'";
	      staffpitch=12*5;
	      break;
	    }

	  staffrelativ=relativ;

	  cpartnum = staffInd + 65;
	  staffname[staffInd].staffid = staffname[staffInd].partshort;
	  staffname[staffInd].staffid.append("part");
	  staffname[staffInd].staffid.append(cpartnum);
	  staffname[staffInd].staffid.prepend("A");
	  staffname[staffInd].staffid.remove(QRegExp("[0-9]"));
	  staffname[staffInd].staffid.remove(QChar('.'));
	  staffname[staffInd].staffid.remove(QChar(' '));

	  findVolta();

	  for (voice = 0; voice < VOICES; ++voice)  voiceActive[voice] = false;

	  /*Each voice is traversed from beginning to end
	    separately. If there was a way of checking whether the
	    entire voice is empty in a more immediate way, this would
	    not have been necessary. But I have not found it, so
	    please tell me if there is. This means that the voice is
	    not written to the outputstream (os) which is connected to
	    the output file, immediately, but buffered to an
	    outputstream (out) which is only connected to a string
	    ("voicebuffer") in memory. If there is no material in the
	    voice, this string is set to "". If there is material in
	    the voice, the voicebuffer is appended to the output
	    stream. The voicebuffer is thereafter set to "". (olav) */

	  for (voice = 0; voice < VOICES; ++voice)
	    {
	      prevpitch=staffpitch;
	      relativ=staffrelativ;
	      donefirst=false;

	      //for all measures in this voice:
	      for (MeasureBase* m = score->first(); m; m = m->next())
		{
		  if (m->type() != MEASURE)
		    continue;
		    writeVoiceMeasure((Measure*)m, staff, staffInd, voice);
		}
	      level--;
	      indent();
	      out << "\\bar \"|.\" \n"; //thin-thick barline as last.
	      level=0;
	      indent();
	      out << "}% end of last bar in partorvoice\n\n";
	      if (voiceActive[voice])
		{
		  os << voicebuffer;
		}
	      voicebuffer = " \n";
	    } // for voice 0 to VOICES

	  int voiceno=0;

	  for (voice = 0; voice < VOICES; ++voice)
	    if (voiceActive[voice]) voiceno++;

	  if (voiceno == 1) staffname[staffInd].simultaneousvoices=false;

	  if (voiceno>1) //if more than one voice must be combined into one staff.
	    {
	      level=0;
	      indent();
	      out << staffname[staffInd].staffid << " = \\simultaneous{\n";
	      staffname[staffInd].simultaneousvoices=true;
	      level++;
	      indent();
	      out << "\\override Staff.NoteCollision  #'merge-differently-headed = ##t\n";
	      indent();
              out << "\\override Staff.NoteCollision  #'merge-differently-dotted = ##t\n";
	      ++level;
	      for (voice = 0; voice < VOICES; ++voice)
		{
		  if (voiceActive[voice])
		    {
		      indent();
		      out << "\\context Voice = \"" << staffname[staffInd].voicename[voice];
		      out << "\" \\" << staffname[staffInd].voicename[voice]  << "\n";
		    }
		}
	      out << "} \n";
	      level=0;
	      indent();
	      os << voicebuffer;
	      voicebuffer = " \n";
	    }

	  ++staffInd;
	}// end of foreach staff
      staffname[staffInd].staffid="laststaff";
      if (n > 1) {
	--level;
	indent();
      }
    }
}// end of writeScore


//-------------------
// write score-block: combining parts and voices, drawing brackets and braces, at end of lilypond file
//-------------------
void ExportLy::writeScoreBlock()
{
  //  bracktest();
  level=0;
  os << "\n\\score { \n";
  level++;
  indentF();
  os << "\\relative << \n";


  indx=0;

  while (staffname[indx].staffid!="laststaff")
    {

      if (lybracks[indx].brakstart)
	{
	  ++level;
	  indentF();
	  os << "\\context StaffGroup = " << (char)(lybracks[indx].brakno + 64) << "<< \n";
	}

      if (lybracks[indx].bracestart)
	{
	  ++level;
	  indentF();
	  if (lybracks[indx].piano)
	    {
	      os << "\\context PianoStaff <<\n";
	      indentF();
	      os << "\\set PianoStaff.instrumentName=\"Piano\" \n";
	      pianostaff=true;
	    }
	  else
	  os << "\\context GrandStaff = " << (char)(lybracks[indx].braceno + 64) << "<< \n";
	}

      ++level;
      indentF();
      os << "\\context Staff = O" << staffname[indx].staffid << "G" << "  << \n";
      ++level;
      indentF();
      os << "\\context Voice = O" << staffname[indx].staffid << "G \\";

      if (staffname[indx].simultaneousvoices)
	os << staffname[indx].staffid << "\n";
      else
	os << staffname[indx].voicename[0] << "\n"; //voices are counted from 0.

      if (lybracks[indx].piano)
	{
	  indentF();
	  os << "\\set Staff.instrumentName = #\"\"\n";
	  indentF();
	  os << "\\set Staff.shortInstrumentName = #\"\"\n";
	}

      --level;
      indentF();
      os << ">>\n";

      if (((lybracks[indx].brakstart) and (lybracks[indx].brakend)) or ((lybracks[indx].bracestart) and (lybracks[indx].braceend)))
	{
	  //if bracket or brace starts and ends on same staff: one-staff brace/bracket.
	  indentF();
	  os << "\\override StaffGroup.SystemStartBracket #'collapse-height = #1 \n";
	  indentF();
	  os << "\\override Score.SystemStartBar #'collapse-height = #1 \n";
	}

      if (lybracks[indx].brakend)
	{  --level;
	  indentF();
	  os << ">> %end of StaffGroup" << (char)(lybracks[indx].brakno + 64) << "\n";
	}
      if (lybracks[indx].braceend)
	{
	  --level;
	  indentF();
	  if (lybracks[indx].piano)
	  os << ">> %end of PianoStaff" << (char)(lybracks[indx].braceno + 64) << "\n";
	  else
	  os << ">> %end of GrandStaff" << (char)(lybracks[indx].braceno + 64) << "\n";
	}

      ++indx;
    }


  indentF();
  os << "\\set Score.skipBars = ##t\n";
  indentF();
  os << "\\set Score.melismaBusyProperties = #'()\n";
  indentF();
  os << "\\override Score.BarNumber #'break-visibility = #end-of-line-invisible %%every bar is numbered.!!!\n";
  indentF();
  os << "%% remove previous line to get barnumbers only at beginning of system.\n";
  indentF();
  os << " #(set-accidental-style 'modern-cautionary)\n";
if (rehearsalnumbers)
    {
      indentF();
      os << "\\set Score.markFormatter = #format-mark-box-numbers %%boxed rehearsal-numbers \n";
    }
 else
   {
     indentF();
     os << "\\set Score.markFormatter = #format-mark-box-letters %%boxed rehearsal-marks\n";
   }
  indentF();
  os << "\\override Score.TimeSignature #'style = #'() %%makes timesigs always numerical\n";
  indentF();
  os << "%% remove previous line to get cut-time/alla breve or common time \n";
  --level;
  indentF();
  os << ">>\n";

  --level;
  --level;
  indentF();
  os <<"}\n\n";

  if (((pianostaff) and (indx==2)) or (indx < 2))
    os << "#(set-global-staff-size 20)\n";
  else if (indx > 2)
    os << "#(set-global-staff-size 14)\n";
}// end scoreblock


//---------------------------------------------------------
//   write
//---------------------------------------------------------

bool ExportLy::write(const QString& name)
{
  pianostaff=false;
  rehearsalnumbers=false;
  f.setFileName(name);
  if (!f.open(QIODevice::WriteOnly))
    return false;
  os.setDevice(&f);
  os.setCodec("utf8");
  out.setString(&voicebuffer);
  os << "%=============================================\n"
    "%   created by MuseScore Version: " << VERSION << "\n"
    "%          " << QDate::currentDate().toString(Qt::SystemLocaleLongDate);
  os << "\n";
  os <<"%=============================================\n"
    "\n"
    "\\version \"2.12.0\"\n\n";     // target lilypond version

  //---------------------------------------------------
  //    Page format
  //---------------------------------------------------


  PageFormat* pf = score->pageFormat();
  os << "#(set-default-paper-size ";
  switch(pf->size) {
  default:
  case  0: os << "\"a4\""; break;
  case  2: os << "\"letter\""; break;
  case  8: os << "\"a3\""; break;
  case  9: os << "\"a5\""; break;
  case 10: os << "\"a6\""; break;
  case 29: os << "\"11x17\""; break;
  }

  if (pf->landscape) os << " 'landscape";

  os << ")\n\n";

  // TODO/O.G.: better choose between standard formats and specified paper
  // dimensions. We normally don't need both.

  double lw = pf->width() - pf->evenLeftMargin - pf->evenRightMargin;
  os << "\\paper {\n";
  os <<  "  line-width    = " << lw * INCH << "\\mm\n";
  os <<  "  left-margin   = " << pf->evenLeftMargin * INCH << "\\mm\n";
  os <<  "  top-margin    = " << pf->evenTopMargin * INCH << "\\mm\n";
  os <<  "  bottom-margin = " << pf->evenBottomMargin * INCH << "\\mm\n";
  os <<  "  indent = 0 \\mm \n";
  os <<  "  %%set to ##t if your score is less than one page: \n";
  os <<  "  ragged-last-bottom = ##f \n";
  os <<  "  ragged-bottom = ##f  \n";
  os <<  "  }\n\n";


  //---------------------------------------------------
  //    score
  //---------------------------------------------------

  os << "\\header {\n";

  ++level;
  const MeasureBase* m = score->first();
  foreach(const Element* e, *m->el()) {
    if (e->type() != TEXT)
      continue;
    QString s = ((Text*)e)->getText();
    indentF();
    switch(e->subtype()) {
    case TEXT_TITLE:
      os << "title = ";
      break;
    case TEXT_SUBTITLE:
      os << "subtitle = ";
      break;
    case TEXT_COMPOSER:
      os << "composer = ";
      break;
    case TEXT_POET:
      os << "poet = ";
      break;
    default:
      printf("text-type %d not supported\n", e->subtype());
      os << "subtitle = ";
      break;
    }
    os << "\"" << s << "\"\n";
  }

  if (score->rights)
    {
      indentF();
      os << "copyright = \"" << score->rights->getText() << "\"\n";
    }

  indentF();
  os << "}\n";

  findBrackets();

  writeScore();

  writeScoreBlock();

  f.close();
  return f.error() == QFile::NoError;
}// end of function "write"





/*----------------------- NEWS and HISTORY:--------------------  */

/* mar. 2009 always explicit end-bar -> no need to declare last bar as
   incomplete, but writes to end-bars when last bar is complete. This
   doesn't show in print.

   12.feb.2009. - Removed bug: double instrument definitions in pieces
   with pickup-measure (prev'ly defined both in measure 0 and measure
   1). - Removed bug: nonrecognition of startrepeats. -Improved
   recognition of whole-measure rests.

   NEW 5.feb.2009: separated grandstaff (variable distance between staffs) from
   pianostaff: constant distance between staffs to prepare for cross-staff
   beaming (not implemented). Brackets/braces for single staffs.

   NEW 25.jan.2009: system brackets and braces for simple scores.
   Unsolved complications for multistaff instruments (piano, organ,
   harp), and for bracketing single staffs.

   NEW 22.jan. 2009
   -- fixed a problem with beames on grace-notes, and
   some faults produced by the previous revision of exportly.

   NEW 18. jan. 2009
   -- Now avoids export of empty voices.

   NEW 23.dec.2008
   -- export of note of lengths longa and brevis, and some rests longer than whole.

   NEW 9. dec. 2008:
   -- Some improvements to triplets and finding the right octave for single note after chord.
   -- started work on codas and segnos.

   NEW 24. nov. 2008:
   -- added dynamic signs and staff-text, by stealing from exportxml.cpp.

   NEW 1. nov. 2008
   --pickupbar (But not irregular bars in general.)
   --ties
   --management of incomplete bars in voices 2-4.

   NEW 26. oct. 2008
  - voice separation and recombination in score-block for easier editing of Lilypondfile.
    todo/unresolved: writes voices 2-4 in Lilypond file even if voice is empty.
  - better finding of correct octave when jumping intervals of fifths or more.

  NEW 10.oct.2008:
   - rudimentary handling of slurs.
   - voltas and endings
   - dotted 8ths and 4ths.
   - triplets, but not general tuplets.
   - PianoStaff reactivated.

   NEW:
   1. dotted 16th notes
   2. Relative pitches
   3. More grace-note types
   4. Slurs and beams in grace-notes
   5. Some adjustments in articulations
   6. separation of staffs/voices from score-block. Unfinished for pianostaffs/GrandStaffs and voices on one staff.
   7. completed the clef-secton.
   Points 2 and 6, and also in a smaller degree 5, enhances human readability of lilypond-code.
*/


/*----------------------TODOS------------------------------------*/

/* TODO: PROJECTS
   4. Lyrics (low priority in relation to music itself)
   5. Collisions in crowded multi-voice staffs (e.g. cello-suite).
   6. General tuplets
   7  Use linked list instead of static array for dynamics etcs.
   8. Determine whether text goes above or below staff.
   10. fingerings, chordname; rehearsal marks as \mark\default in all voices.
   12. correct indentation in score-block.
   13. cross-staff beaming in pianostaff
   14. Real multimeasure rests (for now only one-measure rests are exported).
   18. lacking fermata above/below rest.
   -  big problems with hairpins.
   ExportLy::symbol(): rcomma not supported
   - etc.etc.etc.etc........
*/

/*TODO: BUGS
  - massive failure on gollywog and Bilder
  - in Tarrega study: \stemNeutral failure, second voice, first part, from last bar before repeat.
  - metronome marks must be given as \tempo 4 = 60 and not as markups.
  - etc. etc. etc. ad nauseam.
 */
