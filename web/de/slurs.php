<?php
  $lang="de";
  $file="slurs.php";
  require("header.html");
  ?>
<h4><a href="idx.php">MuseScore</a>
   -- <a href="manual.php">Referenz</a>
   -- Haltebögen</h4>

<table>
<tr>
  <td>erst eine Note selektieren:</td>
  <td><img src="../pic/slur1.png" align="center"></td>
</tr>
<tr>
  <td>&lt;<b>Shift+S</b>&gt; erzeugt einen Haltebogen<br>im <a href="editmode.php">Editiermodus</a>:</td>
  <td><img src="../pic/slur2.png" align="center"></td>
  </tr>
<tr>
  <td>&lt;<b>Shift+Right</b>&gt; verlängert den Bogen zur nächsten Note:</td>
  <td><img src="../pic/slur3.png" align="center"></td>
  </tr>
<tr>
  <td>&lt;<b>X</b>&gt; kippt die Bogenrichtung:</td>
  <td><img src="../pic/slur4.png" align="center"></td>
  </tr>
<tr>
  <td>&lt;<b>Escape</b>&gt; beendet den Editiermodus:</td>
  <td><img src="../pic/slur5.png" align="center"></td>
  </tr>
</table>

<br>
Ein Haltebogen kann mehrere Systeme oder Seiten überspannen. Der Anfang
und das Ende eines Haltebogens ist an einer Note oder Pause verankert.
Verschieben sich die Notenpositionen nach einem Relayout, dann verschiebt
sich auch der Haltbogen.
<br>
<br>
Siehe auch
      <a href="ties.php">Ties</a>,
      <a href="editmode.php">Edit Mode</a>.
<?php require("trailer.html");  ?>

