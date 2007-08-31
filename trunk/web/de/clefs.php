<?php
  $lang="de";
  $file="clefs.php";
  require("header.html");
  ?>
<h4><a href="idx.php">MuseScore</a>
   -- <a href="manual.php">Referenz</a>
   -- Clefs</h4>

<table>
<tr><td>
  Clefs are created or changed by dragging a clef symbol from the
  clef palette to a measure or another clef.<br>
  &lt;<b>F9</b>&gt; toggles the palettes window.<br>
  <br>
  <b>Change</b><br>
  Drag a clef from the palette onto a clef in the score. You can
  also drag a clef from the score to another clef of the score
  by using &lt;<b>Shift+leftMouseButton+Drag</b>&gt;.<br>
  <br>
  <b>Add</b><br>
  Drag a clef from the palette onto an empty part of a measure.
  This creates a clef at the beginning of the measure. If the
  measure is not the first measure in the staff it is drawn
  smaller.<br>
  <br>
  <b>Remove</b><br>
  Select a clef and press &lt;<b>Del</b>&gt;.
  </td>
  <td>
     <img src="../pic/clefpalette.png">
     </tr>
  </td>
  </table>

<br>
Note that changing a clef does not change the pitch of any note.
Instead the notes are moved.

<?php require("trailer.html");  ?>

