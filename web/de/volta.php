<?php
  $lang="de";
  $file="volta.php";
  require("header.html");
  ?>

<h4><a href="idx.php">MuseScore</a>
   -- <a href="manual.php">Referenz</a>
   -- Volta Klammern</h4>

Soll in einer Wiederholung beim zweiten mal das Ende etwas anders gespielt
werden, dann verwendet man die normalen Wiederholungszeichen und kennzeichnet
die beider Endvarianten mit Klammern:
<br><br>
<img src="../pic/volta.png" align="center">
<br><br>

Die Klammern können auch mehr als einen Takt überspannen. Durch Doppelclick
auf eine Klammer kommen wir in den Editmodus und können die Anfasser
mit den Kommandos
<br>
<br>
<table cellspacing="0" cellpadding="0">
<tr><td>&lt;<b>Umsch+Rechts</b>&gt;</td><td> &nbsp;&nbsp;um einen Takt nach rechts</td></tr>
<tr><td>&lt;<b>Umsch+Links</b>&gt;</td><td> &nbsp;&nbsp;um einen Takt nach links</td></tr>
</table>
<br>
verschieben. Durch diese Kommandos wird das "logische" Ende bzw. der Anfang
verschoben, die bestimmen, welche Takte die Volta umklammert.
Andere Kommandos im <a href="editmode.php">Editiermodus</a> verschieben die
Anfasser auch, haben jedoch keine Auswirkungen bei der Wiedergabe der
Wiederholungen sondern wirken nur optisch.
Wird der Anfang oder das Ende einer Voltaklammer verschoben, dann zeigt eine
gestrichelte Linie von der logischen Voltaposition zur aktuellen Position.

<br><br>
<img src="../pic/volta2.png" align="center">
<br><br>

<h5>Eigenschaften</h5>

<table cellspacing="0" cellpadding="0">
<tr>
  <td>
      <b>Wiederholungsliste</b><br>
      In der Wiederholungsliste wird angegeben, in welcher Wiederholung die
      Volta gespielt werden soll. Gibt es mehrere Wiederholungen und soll die
      Volta in mehreren Wiederholungen gespielt werden, dann müssen sie einzeln
      per Komma getrennt aufgeführt werden.
      <br><br>
      <b>Text</b><br>
      Über den Eigenschaften Dialog läßt sich ein beliebiger Text für die
      Volta einstellen. Der Text sollte natürlich mit der Liste der Wiederholungen
      korrespondieren.
     </td>
  <td valign="top">
     <img src="../pic/volta3.png">
     </td>
  </tr>
</table>
<br>
<img src="../pic/volta4.png" align="center">

