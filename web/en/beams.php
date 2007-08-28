<?php
  $lang="en";
  $file="beams.php";
  require("header.html");
  ?>
<h4><a href="index.php">MuseScore</a>
   -- <a href="manual.php">Reference</a>
   -- Beams</h4>

<table>
<tr><td>
      Beams are set automatically but the automatic setting can be
      manually overridden. For this first select a note were you
      want to change the beam behaviour and press the appropriate
      button in the beam pad.
  </td>
  <td>
     <img src="../pic/beampad.png">
     </td>
  </tr>
  </table>

<table cellpadding="0" cellspacing="0">
  <tr><td><img src="../pic/beambutton1.png"></td>
    <td>&nbsp;&nbsp;</td>
    <td>Start a beam at the selected note.</td>
    </tr>
  <tr><td><img src="../pic/beambutton2.png"></td>
    <td>&nbsp;&nbsp;</td>
    <td>Do not end a beam a the selected note.</td>
    </tr>
  <tr><td><img src="../pic/beambutton3.png"></td>
    <td>&nbsp;&nbsp;</td>
    <td>Do not beam this note.</td>
    </tr>
  <tr><td><img src="../pic/beambutton4.png"></td>
    <td>&nbsp;&nbsp;</td>
    <td>Start a second level beam at the selected note.</td>
    </tr>

  </table>

<br>
<?php require("trailer.html");  ?>

