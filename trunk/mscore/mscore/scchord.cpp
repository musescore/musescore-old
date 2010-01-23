//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#include "mscore.h"
#include "scchord.h"
#include "chord.h"
#include "scscore.h"
#include "scnote.h"
#include "note.h"

//---------------------------------------------------------
//   thisChord
//---------------------------------------------------------

Chord* ScChordPrototype::thisChord() const
      {
      Chord** cp = qscriptvalue_cast<ChordPtr*>(thisObject().data());
      if (cp)
            return *cp;
      return 0;
      }
