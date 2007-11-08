<?php
  $lang="en";
  $file="editmode.php";
  require("header.html");
  ?>
<h4><a href="index.php">MuseScore</a>
   -- <a href="manual.php">Reference</a>
   -- Edit Mode</h4>

Many elements in the score can be edited in &lt;<i><b>Edit Mode</b></i>&gt;:<br>
<br>

<table cellpadding="0" cellspacing="0">
 <tr>
    <td>&lt;<b>Double Click</b>&gt;</td>
    <td>&nbsp;&nbsp;</td>
    <td> start &lt;<i><b>Edit Mode</b></i>&gt; </td>
    </tr>
 <tr>
    <td>&lt;<b>Escape</b>&gt;</td>
    <td>&nbsp;&nbsp;</td>
    <td> end &lt;<i><b>Edit Mode</b></i>&gt; </td>
    </tr>
 </table>
<br>

Some elements show <i>handles</i> in editmode which can be
moved by mouse dragging or keyboard commands.<br>
<br>
<a href="slurs.php">Slur</a> in edit mode: <image src="../pic/slur4.png" align="center"><br>
<br>
Available keyboard commands:<br>
<br>
<table cellpadding="0" cellspacing="0">
  <tr><td>&lt;<b>Left</b>&gt;</td>
      <td>&nbsp;&nbsp</td>
      <td>move handle left one Space</td>
    </tr>
  <tr><td>&lt;<b>Right</b>&gt;</td>
      <td>&nbsp;&nbsp</td>
      <td>move handle right one Space</td>
    </tr>
  <tr><td>&lt;<b>Up</b>&gt;</td>
      <td>&nbsp;&nbsp</td>
      <td>move handle up one Space</td>
    </tr>
  <tr><td>&lt;<b>Down</b>&gt;</td>
      <td>&nbsp;&nbsp</td>
      <td>move handle down one Space</td>
    </tr>
  <tr><td>&lt;<b>Ctrl+Left</b>&gt;</td>
      <td>&nbsp;&nbsp</td>
      <td>move handle left 0.1 Space</td>
    </tr>
  <tr><td>&lt;<b>Ctrl+Right</b>&gt;</td>
      <td>&nbsp;&nbsp</td>
      <td>move handle right 0.1 Space</td>
    </tr>
  <tr><td>&lt;<b>Ctrl+Up</b>&gt;</td>
      <td>&nbsp;&nbsp</td>
      <td>move handle up 0.1 Space</td>
    </tr>
  <tr><td>&lt;<b>Ctrl+Down</b>&gt;</td>
      <td>&nbsp;&nbsp</td>
      <td>move handle down 0.1 Space</td>
    </tr>
  <tr><td>&lt;<b>Tab</b>&gt;</td>
      <td>&nbsp;&nbsp</td>
      <td>goto next handle</td>
    </tr>
</table>

<br>
See also
   <a href="textedit.php">Text Editing</a>,
   <a href="slurs.php">Slurs</a>,
   <a href="brackets.php">Brackets</a>,
   <a href="lines.php">Lines</a>.

<?php require("trailer.html");  ?>

