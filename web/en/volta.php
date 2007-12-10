<?php
  $file="volta.php";
  require("header.html");
  ?>

<h4><a href="idx.php">MuseScore</a>
   -- <a href="manual.php">Reference</a>
   -- Volta</h4>

Volta brackets are used to mark different endings in a repeat:
<br><br>
<img src="../pic/volta.png" align="center">
<br><br>

The brackets can span more than one measure. Double click the volta to
enter  <a href="editmode.php">edit mode</a>  and then move the handles
with:
<br>
<br>
<table cellspacing="0" cellpadding="0">
<tr><td>&lt;<b>Shift+Right</b>&gt;</td><td> &nbsp;&nbsp;one measure left</td></tr>
<tr><td>&lt;<b>Shift+Left</b>&gt;</td><td> &nbsp;&nbsp;one measure right</td></tr>
</table>
<br>
This commands move the "logical" start or end of the volta which determine
the bracketet measures.
Other commands in
<a href="editmode.php">edit mode</a>
also move the
handles but do not change how the repeat is played.

If you move the handles, a dashed line from the logical position to
the actual position is shown

<br><br>
<img src="../pic/volta2.png" align="center">
<br><br>

<h5>Properties</h5>

<table cellspacing="0" cellpadding="0">
<tr>
  <td>
      <b>Repeat List</b><br>
      This list determines in which repeat the volta should be played.
      If the volta is played in more than one repeat all repeat numbers
      must be entered separated with a ",".
      <br><br>
      <b>Text</b><br>
      You can set an arbitrary volta text. Of course the text
      should correspond with the repeat list.
     </td>
  <td valign="top">
     <img src="../pic/volta3.png">
     </td>
  </tr>
</table>
<br>
<img src="../pic/volta4.png" align="center">

