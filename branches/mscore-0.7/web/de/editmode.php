<?php
  $lang="de";
  $file="editmode.php";
  require("header.html");
  ?>
<h4><a href="idx.php">MuseScore</a>
   -- <a href="manual.php">Referenz</a>
   -- Editiermodus</h4>

Viele Elemente der Partitur können im &lt;<i><b>Edit Mode</b></i>&gt;:<br>
editiert werden.

<br>

<table cellpadding="0" cellspacing="0">
 <tr>
    <td>&lt;<b>Doppelclick</b>&gt;</td>
    <td>&nbsp;&nbsp;</td>
    <td> startet den &lt;<i><b>Editiermodus</b></i>&gt; </td>
    </tr>
 <tr>
    <td>&lt;<b>Escape</b>&gt;</td>
    <td>&nbsp;&nbsp;</td>
    <td> beendet den &lt;<i><b>Editiermodus</b></i>&gt; </td>
    </tr>
 </table>
<br>

Einige Elemente zeigen im Editiermodus <i>Anfasser</i>, die mit
der Maus oder mit Tastaturkommandos verschoben werden können.
<br>
<br>
<a href="slurs.php">Haltebogen</a> im Editiermodus: <image src="../pic/slur4.png" align="center"><br>
<br>
Verfügbare Tastaturkommandos:<br>
<br>
<table cellpadding="0" cellspacing="0">
  <tr><td>&lt;<b>Linls</b>&gt;</td>
      <td>&nbsp;&nbsp</td>
      <td>verschiebt Anfasser ein Spatium nach links</td>
    </tr>
  <tr><td>&lt;<b>Rechts</b>&gt;</td>
      <td>&nbsp;&nbsp</td>
      <td>vrschiebt Anfasser ein Spatium nach rechts</td>
    </tr>
  <tr><td>&lt;<b>Hoch</b>&gt;</td>
      <td>&nbsp;&nbsp</td>
      <td>verschiebt Anfasser ein Spatium nach oben</td>
    </tr>
  <tr><td>&lt;<b>Tief</b>&gt;</td>
      <td>&nbsp;&nbsp</td>
      <td>verschiebt Anfasser ein Spatium nach unten</td>
    </tr>
  <tr><td>&lt;<b>Ctrl+Links</b>&gt;</td>
      <td>&nbsp;&nbsp</td>
      <td>verschiebt Anfasser 0,1 * Spatium nach links</td>
    </tr>
  <tr><td>&lt;<b>Ctrl+Rechts</b>&gt;</td>
      <td>&nbsp;&nbsp</td>
      <td>verschiebt Anfasser 0,1 * Spatium nach rechts</td>
    </tr>
  <tr><td>&lt;<b>Ctrl+Hoch</b>&gt;</td>
      <td>&nbsp;&nbsp</td>
      <td>verschiebt Anfasser 0,1 * Spatium nach oben</td>
    </tr>
  <tr><td>&lt;<b>Ctrl+Tief</b>&gt;</td>
      <td>&nbsp;&nbsp</td>
      <td>verschiebt Anfasser 0,1 * Spatium nach unten</td>
    </tr>
  <tr><td>&lt;<b>Tab</b>&gt;</td>
      <td>&nbsp;&nbsp</td>
      <td>gehe zum nächsten Anfasser</td>
    </tr>
</table>

<br>
Siehe auch
   <a href="textedit.php">Texte Editieren</a>,
   <a href="slurs.php">Bögen</a>,
   <a href="brackets.php">Klammern</a>,
   <a href="lines.php">Linien</a>.

<?php require("trailer.html");  ?>

