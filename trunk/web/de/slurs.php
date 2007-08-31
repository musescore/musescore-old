<?php
  $lang="de";
  $file="slurs.php";
  require("header.html");
  ?>
<h4><a href="idx.php">MuseScore</a>
   -- <a href="manual.php">Referenz</a>
   -- Slurs</h4>

<table>
<tr>
  <td>select first note:</td>
  <td><img src="../pic/slur1.png" align="center"></td>
</tr>
<tr>
  <td>&lt;<b>Shift+S</b>&gt; creates a slur<br>in <a href="editmode.php">edit mode</a>:</td>
  <td><img src="../pic/slur2.png" align="center"></td>
  </tr>
<tr>
  <td>&lt;<b>Shift+Right</b>&gt; moves the slur<br> end to the next note:</td>
  <td><img src="../pic/slur3.png" align="center"></td>
  </tr>
<tr>
  <td>&lt;<b>X</b>&gt; flips the slur direction:</td>
  <td><img src="../pic/slur4.png" align="center"></td>
  </tr>
<tr>
  <td>&lt;<b>Escape</b>&gt; ends edit mode:</td>
  <td><img src="../pic/slur5.png" align="center"></td>
  </tr>
</table>

<br>
A slur can span several systems and pages. Slur start and end is anchord to
a note/chord or rest. This means that if the notes move in a relayout,
the slur moves to.<br>
<br>
See also
      <a href="ties.php">Ties</a>,
      <a href="editmode.php">Edit Mode</a>.
<?php require("trailer.html");  ?>

