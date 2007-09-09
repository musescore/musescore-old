<?php
  $lang="de";
  $file="noteentry.php";
  require("header.html");
  ?>
<h4><a href="idx.php">MuseScore</a>
   -- <a href="manual.php">Referenz</a>
   -- Noteneingabe</h4>

Noten werden im &lt;<i>Noteneingabemodus</i>&gt; eingegeben. Wähle zunächst eine
Note oder eine Pause als Startposition für die Noteneingabe aus. Beim Eingeben
von Noten werden immer Pausen oder bereits eingegebene Noten durch die neue
Note ersetzt. Die Dauer eines Taktes ist vorgegeben und ändert sich dadurch nicht.

<br>
<br>

<table cellspacing="0" cellpadding="0">
<tr><td>&lt;<b>N</b>&gt;</td><td>enter &lt;<i>Noteneingabe</i>&gt; beginnen.</td></tr>
<tr><td>&lt;<b>Escape</b>&gt;</td><td> <i>Note Edit</i>&gt; beenden.</td></tr>
</table>
<br>


Im &lt;<i>Noteneingabemodus</i>&gt; kann die Dauer der nächsten
Note aus der Notenpalette oder mit folgenden Tastaturkürzel gewählt werden:
<br>
<br>
<table cellspacing="0" cellpadding="0">
<tr><td>&lt;<b>Alt+1</b>&gt;</td><td>&nbsp;1/4 Note</td>  <td>&nbsp;&nbsp;</td> <td>&lt;<b>Alt+2</b>&gt;</td><td>&nbsp;1/8 Note</td></tr>
<tr><td>&lt;<b>Alt+3</b>&gt;</td><td>&nbsp;1/16 Note</td> <td>&nbsp;&nbsp;</td> <td>&lt;<b>Alt+4</b>&gt;</td><td>&nbsp;1/32 Note</td></tr>
<tr><td>&lt;<b>Alt+5</b>&gt;</td><td>&nbsp;1/64 Note</td> <td>&nbsp;&nbsp;</td> <td>&lt;<b>Alt+6</b>&gt;</td><td>&nbsp;1/1 Note</td></tr>
<tr><td>&lt;<b>Alt+7</b>&gt;</td><td>&nbsp;1/2 Note</td></tr>
</table>
<br>
<table>
<tr>
  <td>Noten werden durch drücken von <br>&lt;<b>C D E F G A B</b>&gt; eingegeben:</td>
  <td><img src="../pic/noteentry1.png" align="center"></td>
</tr>
<tr>
  <td>&lt;<b>Leertaste</b>&gt; erzeugt eine Pause<br>&lt;<b>C D Leertaste E</b>&gt;:</td>
  <td><img src="../pic/noteentry2.png" align="center"></td>
  </tr>
<tr>
  <td>
      &lt;<b>Shift+Notename</b>&gt;<br> fügt eine Note zu einem Akkord hinzu
      &lt;<b>C D Shift+F Shift+A E F</b>&gt;:
      </td>
  <td><img src="../pic/noteentry3.png" align="center"></td>
  </tr>
<tr>
  <td>Balken werden automatisch erzeugt<br>
      &lt;<b>Alt+1 C D Alt+2 E F G A</b>&gt;:
  </td>
  <td><img src="../pic/noteentry4.png" align="center"></td>
  </tr>

</table>

<?php require("trailer.html");  ?>

