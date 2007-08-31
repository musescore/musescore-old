<?php
  $lang="de";
  $file="noteentry.php";
  require("header.html");
  ?>
<h4><a href="idx.php">MuseScore</a>
   -- <a href="manual.php">Referenz</a>
   -- Enter Notes</h4>

Notes are entered in &lt;<i>Note Edit</i>&gt; mode. First select a note
or rest as start position for note entry. When entering notes you are
always replacing existing notes or rests. So the duration of a measure
does not change.<br>
<br>

<table cellspacing="0" cellpadding="0">
<tr><td>&lt;<b>N</b>&gt;</td><td>enter &lt;<i>Note Edit</i>&gt; mode.</td></tr>
<tr><td>&lt;<b>Escape</b>&gt;</td><td> leave <i>Note Edit</i>&gt; mode.</td></tr>
</table>
<br>

After entering &lt;<i>Note Edit</i>&gt; mode you should select the
duration of the note you want to enter by selecting a value from
the note palette or by pressing:
<br>
<br>
<table cellspacing="0" cellpadding="0">
<tr><td>&lt;<b>Alt+1</b>&gt;</td><td>&nbsp;1/4 note</td>  <td>&nbsp;&nbsp;</td> <td>&lt;<b>Alt+2</b>&gt;</td><td>&nbsp;1/8 note</td></tr>
<tr><td>&lt;<b>Alt+3</b>&gt;</td><td>&nbsp;1/16 note</td> <td>&nbsp;&nbsp;</td> <td>&lt;<b>Alt+4</b>&gt;</td><td>&nbsp;1/32 note</td></tr>
<tr><td>&lt;<b>Alt+5</b>&gt;</td><td>&nbsp;1/64 note</td> <td>&nbsp;&nbsp;</td> <td>&lt;<b>Alt+6</b>&gt;</td><td>&nbsp;1/1 note</td></tr>
<tr><td>&lt;<b>Alt+7</b>&gt;</td><td>&nbsp;1/2 note</td></tr>
</table>
<br>
<table>
<tr>
  <td>Notes are entered by pressing<br>&lt;<b>C D E F G A B</b>&gt;:</td>
  <td><img src="../pic/noteentry1.png" align="center"></td>
</tr>
<tr>
  <td>&lt;<b>Space</b>&gt; creates a rest<br>&lt;<b>C D Space E</b>&gt;:</td>
  <td><img src="../pic/noteentry2.png" align="center"></td>
  </tr>
<tr>
  <td>Notes are added to chords by
      pressing &lt;<b>Shift+Notename</b>&gt;<br>
      &lt;<b>C D Shift+F Shift+A E F</b>&gt;:
      </td>
  <td><img src="../pic/noteentry3.png" align="center"></td>
  </tr>
<tr>
  <td>Beams are created automatically<br>
      &lt;<b>Alt+1 C D Alt+2 E F G A</b>&gt;:
  </td>
  <td><img src="../pic/noteentry4.png" align="center"></td>
  </tr>

</table>

<?php require("trailer.html");  ?>

