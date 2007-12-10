<?php
  $file="accidentals.php";
  require("header.html");
  ?>
<h4><a href="index.php">MuseScore</a>
   -- <a href="manual.php">Reference</a>
   -- Accidentals</h4>

<table>
<tr><td>
      Accidentals can be set or changed by dragging an accidental symbol
      from the accidental palette to a note in the score.<br>
      <br>
      If you only want to change the pitch of a note you can also
      select the note and:<br>
      <br>
  </td>
  <td>
     <img src="../pic/accidentalspalette.png">
     </tr>
  </td>
  </table>

  <table cellpadding="0" cellspacing="0">
  <tr><td>&lt;<b>Up</b>&gt;</td><td>Increase the pitch of a note for one semitone.</td></tr>
  <tr><td>&lt;<b>Down</b>&gt;</td><td>Decrease the pitch of a note for one semitone.</td></tr>
  <tr><td>&lt;<b>Shift+Up</b>&gt;</td><td>Increase the pitch of a note for one octave.</td></tr>
  <tr><td>&lt;<b>Shift+Down</b>&gt;</td><td>Decrease the pitch of a note for one Octave.</td></tr>
  </table>
  <br>
  <i>MuseScore</i> automatically tries to set an appropriate accidental for
  the changed pitch. If you don't agree with this or you want to place
  an cautionary accidental (editorial accidental), then you have to manually
  drag an accidental from the accidental palette to the note. If you again change
  the pitch with cursor keys, this manual set accidental will be lost.<br>
  <br>
  The menu function &lt;<b>Notes/Pitch spell</b>&gt; tries to guess the
  right accidentals for the whole score.
<?php require("trailer.html");  ?>

