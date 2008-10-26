//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: exportxml.cpp,v 1.71 2006/03/28 14:58:58 wschweer Exp $
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
// For HISTORY, NEWS and TODOS: see end of file 
// Still too many problems for exportly to be usable.

// Works on inv1.msc and the tarrega-study in demos. Provided you
// delete a quarter-rest in the last measure before repeatsign, in
// staff 1, voice 2. Seems like this voice/bar has one quarter too many?

// 
// Some revisions by olagunde@start.no
//
// In my code I use emacs c++ mode for formatting. In addition I hit
// the return key before every "begin"-brace. In that way begin- and
// end-braces are vertically aligned. Otherwise I get lost, so you
// will have to endure it. I will eventually clean up redundant begin-end braces
// Olav.

#include "barline.h"
#include "beam.h"
#include "config.h"
#include "dynamics.h"
#include "score.h"
#include "globals.h"
#include "part.h"
#include "hairpin.h"
#include "segment.h"
#include "measure.h"
#include "staff.h"
#include "layout.h"
#include "chord.h"
#include "note.h"
#include "rest.h"
#include "timesig.h"
#include "clef.h"
#include "keysig.h"
#include "key.h"
#include "page.h"
#include "text.h"
#include "slur.h"
#include "style.h"
#include "tuplet.h"
#include "articulation.h"
#include "volta.h"
#include <string.h>

const int MAX_SLURS = 8;

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
  QString  staffid[32];
  int indx;
  int timeabove, timebelow;
  bool slur;
  bool graceswitch;
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
  QString voicename[VOICES], partshort[32];
  char *relativ, *staffrelativ;
  bool voiceActive[VOICES];
  QString  partname[32];
  QString cleannote, prevnote;

  
  void indent();
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
  void writeRest(int, int, int);
  void findVolta();
  void writeBarline(Measure* m);
  int  voltaCheckBar(Measure * meas, int i);
  void writeVolta(int measurenumber, int lastind);
  void findTuplets(Note* note);
  void writeArticulation(Chord*);
  void writeScoreBlock();
  void checkSlur(Chord* chord);
  void doSlurStart(Chord* chord);
  void doSlurStop(Chord* chord);

  
public:
  ExportLy(Score* s) {
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
//   indent
//---------------------------------------------------------

void ExportLy::indent()
{
  for (int i = 0; i < level; ++i)
    os << "    ";
}


//-------------------------------------
// Find tuplets
//-------------------------------------

void ExportLy::findTuplets(Note* note)

{
  
  Tuplet* t = note->chord()->tuplet();
  int actNotes = 1;
  int nrmNotes = 1;

  if (t) 
    {
      if (tupletcount == 0) 
	{
	  actNotes = t->actualNotes();
	  nrmNotes = t->normalNotes();
	  tupletcount=actNotes;
	  os << "\\times " <<  nrmNotes << "/" << actNotes << "{" ; 
	}
      else if (tupletcount>1)
	{
	  tupletcount--;
	} 
    }
}//end of check-tuplets


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
  return i;
}//end voltacheckbarline



//------------------------------------------------------------------
//   findVolta -- find and register volta and repeats in entire piece
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

  for (MeasureBase * m=score->layout()->first(); m; m=m->next())
    {// for all measures
      if (m->type() !=MEASURE )
	continue;
      
      //hvis det derimot er en takt:
      
      ++taktnr;
      
      foreach(Element* el, *(m->score()->gel())) 
	//walk thru all elements of measure
	{
	  if (el->type() == VOLTA) 
	    {
	      Volta* v = (Volta*) el;
              
	      if (v->tick() == m->tick()) //hvis det er først i takten
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
		  // 		    {// se comment above.
		  // 		    }
		  // 		  else if (v->subtype() == Volta::VOLTA_OPEN) 
		  // 		    {// se comment above.
		  // 		    }
		  
		}
	    }
	}// for all elements

      i=voltaCheckBar((Measure *) m, i);

    }//for all measures

  lastind=i;

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
  os << "\\clef ";
  switch(clef) {
  case CLEF_G:      os << "treble";       break;
  case CLEF_F:      os << "bass";         break;
  case CLEF_G1:     os << "\"treble^8\""; break;
  case CLEF_G2:     os << "\"treble^15\"";break;
  case CLEF_G3:     os << "\"treble_8\""; break;
  case CLEF_F8:     os << "\"bass_8\"";   break;
  case CLEF_F15:    os << "\"bass_15\"";  break;
  case CLEF_F_B:    os << "bass";         break;
  case CLEF_F_C:    os << "bass";         break;
  case CLEF_C1:     os <<  "soprano";     break;
  case CLEF_C2:     os <<  "mezzo-soprano";break;
  case CLEF_C3:     os <<  "alto";        break;
  case CLEF_C4:     os <<  "tenor";       break;
  case CLEF_TAB:    os <<  "tab";         break;
  case CLEF_PERC:   os <<  "percussion";  break;
  }
  os << " ";
}

//---------------------------------------------------------
//   writeTimeSig
//---------------------------------------------------------

void ExportLy::writeTimeSig(TimeSig* sig)
{
  int z, n;
  sig->getSig(&n, &z);
  timeabove=z;
  timebelow=n;
  os << "\\time " << z << "/" << n << " ";
}

//---------------------------------------------------------
//   writeKeySig
//---------------------------------------------------------

void ExportLy::writeKeySig(int st)
{
  st = char(st & 0xff);
  if (st == 0)
    return;
  os << "\\key ";
  switch(st) {
  case 6:  os << "fis"; break;
  case 5:  os << "h";   break;
  case 4:  os << "e";   break;
  case 3:  os << "a";   break;
  case 2:  os << "d";   break;
  case 1:  os << "g";   break;
  case 0:  os << "c";   break;
  case -7: os << "ces"; break;
  case -6: os << "ges"; break;
  case -5: os << "des"; break;
  case -4: os << "as";  break;
  case -3: os << "es";  break;
  case -2: os << "bes"; break;
  case -1: os << "f";   break;
  default:
    printf("illegal key %d\n", st);
    break;
  }
  os << " \\major ";
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




//---------------------------------------------------------
//   findSlur -- get index of slur in slur table
//   return -1 if not found
//   I really don't understand this. I have only stolen it
//   from exportxml.cpp and adapted it.
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
  // search for slurre(s) starting at this chord
  foreach(const Slur* s, chord->slurFor()) 
    {
      // check if on slur list (i.e. stop already seen)
      int i = findSlur(s);
      if (i >= 0) {
	// remove from list and print start
	slurre[i] = 0;
	started[i] = false;
	if (s->slurDirection() == UP) os << "^";
	os << "(";
	//xml.tagE("slur type=\"start\"%s number=\"%d\"",....
      }
      else {
	// find free slot to store it
	i = findSlur(0);
	if (i >= 0) {
	  slurre[i] = s;
	  started[i] = true;
	  os << "(";
	  //xml.tagE("slur type=\"start\" number=\"%d\"", i + 1);
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
  // search for slurre(s) stopping at this chord but not on slur list yet
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
	  os << ")";
	  //xml.tagE("slur type=\"stop\" number=\"%d\"", i + 1);
	}
	else
	  printf("no free slur slot");
      }
    }
  // search slur list for already started slur(s) stopping at this chord
  for (int i = 0; i < 8; ++i) 
    {
      if (slurre[i])
	{
	  if  (slurre[i]->endElement() == chord) 
	    {
	      if (started[i]) {
		slurre[i] = 0;
		started[i] = false;
		os << ")";
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
	  os << "\\fermata";
	  break;
	case DfermataSym:
	  os << "_\\fermata";
	  break;
	case ThumbSym:
	  os << "\\thumb";
	  break;
	case SforzatoaccentSym:
	  os << "->";
	  break;
	case EspressivoSym:
	  os << "\\espressivo";
	  break;
	case StaccatoSym:
	  os << "-.";
	  break;
	case UstaccatissimoSym:
	  os << "-|";
	  break;
	case DstaccatissimoSym:
	  os << "_|";
	  break;
	case TenutoSym:
	  os << "--";
	  break;
	case UportatoSym:
	  os << "-_";
	  break;
	case DportatoSym:
	  os << "__";
	  break;
	case UmarcatoSym:
	  os << "-^";
	  break;
	case DmarcatoSym:
	  os << "_^";
	  break;
	case OuvertSym:
	  os << "\\open";
	  break;
	case PlusstopSym:
	  os << "-+";
	  break;
	case UpbowSym:
	  os << "\\upbow";
	  break;
	case DownbowSym:
	  os << "\\downbow";
	  break;
	case ReverseturnSym:
	  os << "\\reverseturn";
	  break;
	case TurnSym:
	  os << "\\turn";
	  break;
	case TrillSym:
	  os << "\\trill";
	  break;
	case PrallSym:
	  os << "\\prall";
	  break;
	case MordentSym:
	  os << "\\mordent";
	  break;
	case PrallPrallSym:
	  os << "\\prallprall";
	  break;
	case PrallMordentSym:
	  os << "\\prallmordent";
	  break;
	case UpPrallSym:
	  os << "\\prallup";
	  break;
	case DownPrallSym:
	  os << "\\pralldown";
	  break;
	case UpMordentSym:
	  os << "\\upmordent";
	  break;
	case DownMordentSym:
	  os << "\\downmordent";
	  break;
#if 0 // TODO: this are now Repeat() elements
	case SegnoSym:
	  os << "\\segno";
	  break;
	case CodaSym:
	  os << "\\coda";
	  break;
	case VarcodaSym:
	  os << "\\varcoda";
	  break;
#endif
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
  QString purename;

  // only the stem direction of the first chord in a beamed chord
  // group is relevant OG: -- for Mscore that is, causes trouble for
  // lily when tested on inv1.msc
  // 
  if (c->beam() == 0 || c->beam()->getElements().front() == c) 
    {
      Direction d = c->stemDirection();
      if (d != stemDirection) 
	{
	  stemDirection = d;
	  if ((d == UP) and (graceswitch == true))
	    os << "\\stemUp ";
	  else if ((d == DOWN)  and (graceswitch == true))
	    os << "\\stemDown ";
	  else if (d == AUTO)
	    {
	      if (graceswitch == true) os << "] ";
	      os << "\\stemNeutral ";
	    }
	}
    }


  NoteList* nl = c->noteList();

  if (nl->size() > 1)
    os << "<"; //start of chord

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
	      os << " } "; //end of grace
	    }
	  break;
	case NOTE_ACCIACCATURA:
	case NOTE_APPOGGIATURA:
	case NOTE_GRACE4:
	case NOTE_GRACE16:
	case NOTE_GRACE32: 	
	  if (graceswitch==false)  
	    { 
	      os << "\\grace{\\stemUp "; //as long as general stemdirecton is unsolved: graces always stemUp. 
	      graceswitch=true; 
	    }
	  else 
	    if (graceswitch==true)
	      {
		os << "[( "; //grace always beamed and slurred
	      }
	  break;                   
	} //end of switch(gracen)


      findTuplets(n);

      os << tpc2name(n->tpc());
 
      purepitch = n->pitch();
      purename = tpc2name(n->tpc());  //with -es or -is
      prevnote=cleannote;             //without -es or -is
      cleannote=tpc2purename(n->tpc());//without -es or -is 
      
      if (purename.contains("eses")==1)  purepitch=purepitch+2;
      else if (purename.contains("es")==1)  purepitch=purepitch+1;
      else if (purename.contains("isis")==1) purepitch=purepitch-2; 
      else if (purename.contains("is")==1) purepitch=purepitch-1;
      
      oktavdiff=prevpitch - purepitch;
      //      int oktreit = (numval(oktavdiff) / 12);
      int oktreit=numval(oktavdiff);
      
      while (oktreit > 0) 
	{ 
	  if ((oktavdiff < -6) or ((prevnote=="b") and (oktavdiff < -5)))
	    { //up
		os << "'"; 
		oktavdiff=oktavdiff+12;
	    }
	    else if ((oktavdiff > 6)  or ((prevnote=="f") and (oktavdiff > 5)))
	      {//down
		os << ",";
		oktavdiff=oktavdiff-12;
	      }
	  oktreit=oktreit-12;
	}

      prevpitch=purepitch;

      if (i == nl->begin()) chordpitch=prevpitch; 
      //^^^^^^remember pitch of first chordnote to write next chord-or-note relative to.
      ++i; //number of notes in chord, we progress to next chordnote
      if (i == nl->end())
	break;
      os << " ";
    } //end of notelist = end of chord

  if (nl->size() > 1)
    os << ">"; //end of chord sign

  if (tupletcount==1) {os << " } "; tupletcount=0; }

  prevpitch=chordpitch;

  writeLen(c->tickLen());

  if (graceslur==true)
    {
      os << " ) "; //slur skulle vært avslutta etter hovednoten.
      graceslur=false;
    }

  writeArticulation(c);      
  checkSlur(c);

  os << " ";
}// end of writechord


//---------------------------------------------------------
//   getLen
//---------------------------------------------------------

int ExportLy::getLen(int l, int* dots)
{
  int len  = 4;

  if      (l == 6 * division) 
    {
      len  = 1;
      *dots = 1;
    }
  else if (l == 5 * division) 
    {
      len = 1;
      *dots = 2;
    }
  else if (l == 4 * division)
    len = 1;
  else if (l == 3 * division) // dotted half
    {
      len = 2;
      *dots = 1;
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
  else if (l == division * 2 /3) 
    len = 4;
  else if (l == division /3) 
    len = 8;
  else if (l == division /3*2)
    len = 16;
  else if (l == division /3*4)
    len = 32;
  else printf("unsupported len %d (%d,%d)\n", l, l/division, l % division);
  return len;
}

//---------------------------------------------------------
//   writeLen
//---------------------------------------------------------

void ExportLy::writeLen(int ticks)
{
  int dots = 0;
  int len = getLen(ticks, &dots);
  if (ticks != curTicks) {
    os << len;
    for (int i = 0; i < dots; ++i)
      os << ".";
    curTicks = ticks;
  }
}

//---------------------------------------------------------
//   writeRest
//    type = 0    normal rest
//    type = 1    whole measure rest
//    type = 2    spacer rest
//---------------------------------------------------------

void ExportLy::writeRest(int tick, int l, int type)
{
  if (type == 1) {
    // write whole measure rest
    int z, n;
    score->sigmap->timesig(tick, z, n);
    os << "R1*" << z << "/" << n;
    curTicks = -1;
  }
  else if (type == 2) {
    os << "s";
    writeLen(l);
  }
  else {
    os << "r";
    writeLen(l);
  }
  os << " ";
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
	      os << "\\repeat volta 2 {";
	      firstalt=false;
	      secondalt=false;
	      break;
	    case endrepeat: 
	      if ((repeatactive==true) and (secondalt==false))
		{
		  os << "} % end of repeatactive\n";		
		  // repeatactive=false;
		} 
	      indent();
	      break;
	    case bothrepeat:
	      if (firstalt==false)
		{
		  os << "} % end of repeat\n";
		  indent();
		  os << "\\repeat volta 2 {";
		  firstalt=false;
		  secondalt=false;
		  repeatactive=true;
		}
	      break;
	    case doublebar:
	      indent();
	      os << "\\bar \"||\"";
	      break;
	      // 	    case brokenbar:      
	      // 	      indent(); 
	      // 	      os << "\\bar \"| |:\"";
	      // 	      break;
	    case startending:
	      if (firstalt==false)
		{
		  os << "} % end of repeat except alternate endings\n";
		  indent();
		  os << "\\alternative{ {  ";
		  firstalt=true;
		}
	      else 
		{ 
		  os << "{ ";
		  indent();
		  firstalt=false;
		  secondalt=true;
		}      
	      break;
	    case endending:
	      if (firstalt) 
		{
		  os << "} %close alt1\n";
		  secondalt=true;
		  repeatactive=true;
		}
	      else 
		{
		  os << "} } %close alternatives\n";
		  secondalt=false;
		  firstalt=true;
		  repeatactive=false;
		} 
	      break;
	    case endbar:
	      os << "\\bar \"|.\"";
	      break;
	    case none: printf("strange voltarraycontents?\n");
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
void ExportLy::writeVoiceMeasure(Measure* m, Staff* staff, int staffIdx, int voice) 
{
  int i=0;
  char cvoicenum;
  measurenumber=m->no()+1;

  if (measurenumber==1)
    {
      level=0;
      indent();
      cvoicenum=voice+65;
      //there must be more elegant ways to do this, but whatever...
      voicename[voice] = partshort[staffIdx];
      voicename[voice].append("voice");
      voicename[voice].append(cvoicenum);
      voicename[voice].prepend("A");
      voicename[voice].remove(QRegExp("[0-9]"));
      voicename[voice].remove(QChar('.'));
      voicename[voice].remove(QChar(' '));

      os << voicename[voice];
      os << " = \\relative c" << relativ <<" \n";
      indent();
      os << "{\n";
      level++;
      indent();
      if (voice==0)
	{
	  os <<"\\set Staff.instrumentName = #\"" << partname[staffIdx] << "\"\n";
	  indent();
	  os << "\\set Staff.shortInstrumentName = #\"" <<partshort[staffIdx] << "\"\n";
	  indent();
	  writeClef(staff->clef(0));
	  writeKeySig(staff->keymap()->key(0));
	  os << "\n";
	  indent();
	  //write time signature:
	  os << "\\time " << timeabove << "/" << timebelow << "\n";
	  indent();
	  os << "\n\n";
	}

      switch(voice)
	{
	case 0: break;
	case 1: 
	  os <<"\\voiceTwo" <<"\n\n";
	  break;
	case 2:
	  os <<"\\voiceThree" <<"\n\n";
	  break;
	case 3:
	  os <<"\\voiceFour" <<"\n\n";
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
	      os << "\\repeat volta 2 { \n";
	      repeatactive=true;
	    }
	}
    }// if start of first measure
  indent();
  
  int tick = m->tick(); 
  
  for(Segment* s = m->first(); s; s = s->next()) 
    {
      // for all segments in measure. What is a segment??
      Element* e = s->element(staffIdx * VOICES + voice);
      
      if (!(e == 0 || e->generated()))  voiceActive[voice] = true; 
      else  continue;

      switch(e->type()) 
	{
	case CLEF:
	  writeClef(e->subtype());
	  break;
	case TIMESIG:
	  {
	    writeTimeSig((TimeSig*)e);
	    os << "\n\n";
	    indent();
	    break;
	  }
	case KEYSIG:
	  writeKeySig(e->subtype());
	  break;
	case CHORD:
	  {
	    if (voice) //only write rests as part of chord for second etc. voice.
	      {
		int ntick = e->tick() - tick;
		if (ntick > 0)
		  writeRest(tick, ntick, 2);
		tick += ntick;
	      }
	    writeChord((Chord*)e);
	    tick += e->tickLen();
	  }
	  break;
	case REST:
	  {
	    int l = e->tickLen();
	    if (l == 0) {
	      l = ((Rest*)e)->segment()->measure()->tickLen();
	      writeRest(e->tick(), l, 1);
	    }
	    else
	      writeRest(e->tick(), l, 0);
	    tick += l;
	  }
	  break;
	case BAR_LINE: //We never arrive here!!??
	  break;
	case BREATH:
	  os << "\\breathe ";
	  break;
	case DYNAMIC:
	  printf("DYNAMIC!\n");
	  break;
	default:
	  printf("Export Lilypond: unsupported element <%s>\n", e->name());
	  break;
	}
    } //end for all segments

  if (voiceActive[voice] == false)   os << "s1";
  writeVolta(measurenumber, lastind); 
  //  writeBarline(m);
  os << " | % " << m->no()+1 << "\n" ; //barcheck and barnumber
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
  int staffIdx = 0;
  int np = score->parts()->size();
  graceswitch=false;
  int voice=0;
  cleannote="c";
  prevnote="c";
    
  if (np > 1) 
    {
      //Number of parts, to be used when we implement braces and brackets.
    }

  //get start time signature:
  score->sigmap->timesig(0, timeabove, timebelow);

  foreach(Part* part, *score->parts()) 
    {
      int n = part->staves()->size();
      partname[staffIdx]  = part->longName();	   
      partshort[staffIdx] = part->shortName();
      if (n > 1) 
	{//number of staffs. At the moment any collection of staffs,
	  //e.g. a symphonic score, is treated as one pianostaff.
	  pianostaff=true;
	}
      
      foreach(Staff* staff, *part->staves())
	{
	  
	  os << "\n";
	  
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
	
	  cpartnum = staffIdx + 65;
	  staffid[staffIdx] = partshort[staffIdx];
	  staffid[staffIdx].append("part");
	  staffid[staffIdx].append(cpartnum);
	  staffid[staffIdx].prepend("A");
	  staffid[staffIdx].remove(QRegExp("[0-9]"));
	  staffid[staffIdx].remove(QChar('.'));
	  staffid[staffIdx].remove(QChar(' '));
	  
	  findVolta();

	  for (voice = 0; voice < VOICES; ++voice)  voiceActive[voice] = false; 
	
	  for (voice = 0; voice < VOICES; ++voice) 
	    { 
	      prevpitch=staffpitch;
	      relativ=staffrelativ;
	      for (MeasureBase* m = score->layout()->first(); m; m = m->next())
		{ 
		  if (m->type() != MEASURE)
		    continue;
		  writeVoiceMeasure((Measure*)m, staff, staffIdx, voice);
		}
	      level=0;
	      indent();
	      os << "}% end of last bar in partorvoice\n\n";
	    }

	  int voiceno=0;
	  
	  for (voice = 0; voice < VOICES; ++voice)
	    if (voiceActive) voiceno++;
	  
	  if (voiceno>1) 
	    {     
	      level=0;
	      indent();
	      os << staffid[staffIdx] << " = \\simultaneous{\n";
	      level++;
	      indent();
	      os << "\\override Staff.NoteCollision  #'merge-differently-headed = ##t\n";
	      indent();
              os << "\\override Staff.NoteCollision  #'merge-differently-dotted = ##t\n";    
	      ++level;
	      for (voice = 0; voice < VOICES; ++voice)
		{
		  if (voiceActive[voice])
		    {		      
		      indent();
		      os << "\\context Voice = \"" << voicename[voice] << "\" \\" << voicename[voice]  << "\n";
		    }
		}
	      os << "} \n";
	      level=0;
	      indent();
	    }
	  
	  ++staffIdx;
	}// end of foreach staff
      staffid[staffIdx]="laststaff";
      if (n > 1) {
	--level;	      
	indent();
      }
    }      
}// end of writeScore


//------------------- 
// score-block: combining parts and voices, drawing brackets and braces, at end of lilypond file
//-------------------
void ExportLy::writeScoreBlock()
{
  level=0;
  os << "\n\\score { \n";
  level++;
  indent();
  os << "\\relative << \n";

  if (pianostaff)
    {
      ++level;
      indent();
      os << "\\context PianoStaff <<\n";
    }

  indx=0;

  while (staffid[indx]!="laststaff")
    {
      ++level;
      indent();
      os << "\\context Staff = O" << staffid[indx] << "G" << "  << \n"; 
      ++level;
      indent();
      os << "\\context Voice = O" << staffid[indx] << "G \\" << staffid[indx] << "\n";
      --level;
      indent();
      os << ">>\n";
      ++indx;
    }

  if (pianostaff)
    { 
      --level;
      indent();
      os << ">>\n";
      pianostaff=false;
    }

  --level;
  indent();
  os << ">>\n";
  --level;
  indent();
  os <<"}\n";

}// end scoreblock



//---------------------------------------------------------
//   write
//---------------------------------------------------------

bool ExportLy::write(const QString& name)
{
  pianostaff=false;
  f.setFileName(name);
  if (!f.open(QIODevice::WriteOnly))
    return false;
  os.setDevice(&f);
  os << "%=============================================\n"
    "%   created by MuseScore Version: " << VERSION << "\n"
    "%=============================================\n"
    "\n"
    "\\version \"2.10.5\"\n\n";     // target lilypond version
	
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

  // better choose between standard formats^^^ and specified paper
  // dimensions. We normally don't need both.

  double lw = pf->width() - pf->evenLeftMargin - pf->evenRightMargin;
  os << "\\paper {\n"
    "  line-width    = " << lw * INCH << "\\mm\n"
    "  left-margin   = " << pf->evenLeftMargin * INCH << "\\mm\n"
    "  top-margin    = " << pf->evenTopMargin * INCH << "\\mm\n"
    "  bottom-margin = " << pf->evenBottomMargin * INCH << "\\mm\n"
    "  }\n\n";

  //---------------------------------------------------
  //    score
  //---------------------------------------------------

  os << "\\header {\n";

  ++level;
  const MeasureBase* m = score->layout()->first();
  foreach(const Element* e, *m->el()) {
    if (e->type() != TEXT)
      continue;
    QString s = ((Text*)e)->getText();
    indent();
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
  indent();
  os << "}\n";
    

  writeScore();
      
  writeScoreBlock();

  f.close();
  return f.error() == QFile::NoError;
}// end of function "write"


/*----------------------- NEWS and HISTORY:--------------------  */

/* NEW 26. oct. 2008
  - voice separation and recombination in score-block for easier editing of Lilypondfile.
    todo/unresolved: writes voices 2-4 in Lilypond file even if voice is empty. 
  - better finding of correct octave when jumping intervals of fifths or more.
*/


/* NEW 10.oct.2008:
   - rudimentary handling of slurs.
   - voltas and endings
   - dotted 8ths and 4ths. Problem: Do I have to calculate each and every notelength: 
   is it not possible to find a general algorithm?
   - triplets, but not general tuplets. Same problem as in previous point.
   - PianoStaff reactivated.*/


/*NEW:
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
   - Pickup bar
   - avoid empty output in voices 2-4. Does not affect visual endresult.
   1. Dynamics
   2. Segno etc.                -----"-------
   3. Piano staffs/GrandStaffs, system brackets and braces.
   - Lyrics
   4. General tuplets 
   - etc.etc.etc.etc........
*/

/*TODO: BUGS
  - \stemUp \stemDown : sometimes correct sometimes not??? Maybe I
  have not understood Lily's rules for the use of these commands?
  Lily's own stem direction algorithms are good enough. Until a better
  idea comes along: drop export of stem direction and leave it to
  LilyPond.
 */
