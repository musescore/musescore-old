<?php
  $file="frames.php";
  require("header.html");
  ?>

<h4><a href="idx.php">MuseScore</a>
   -- <a href="manual.php">Reference</a>
   -- Frames</h4>

Frames provide empty space outside of normal measures. They can also contain
text or pictures. <i>MuseScore</i> has two kind of frames:
<br>
<br>
<table cellspacing="0" cellpadding="0">
<tr>
   <td valign="top"><b>horizontal</b></td>
   <td> &nbsp;&nbsp;&nbsp;</td>
   <td>Horizontal frames break a System. The width is adjustable and the
      height equals the system height. HFrames can be used to separate
      a Coda.
      </td>
   </tr>
<tr><td><br></td></tr>
<tr>
   <td valign="top"><b>vertical</b></td>
   <td> &nbsp;&nbsp;&nbsp;</td>
   <td>
   Vertical frames provide empty space between or before systems.
   The height is adjustable and the width equals the system width.
   Vertical frames are used to place titel, subtitle or composer.
   I you create a titel, a vertical frame is placed before the first
   measure automatically if its not there already.
   </td>
   </tr>
</table>
<br>

<h5>Create a frame</h5>
First select a measure. The command to insert a frame is found at
the pulldown menu <b>Create->Measures</b>. The frame is inserted
<i>before</i> the selected measure.

<h5>Delete a frame</h5>
Select the frame and press &lt;<b>Del</b>&gt;.

<h5>Edit frame</h5>
Doubleclick at the frame to enter
<a href="editmode.php"><b>Editmode</b></a>. A handle appears which
can be used to drag the size of the frame.
<br><br>
Title frame in edit mode:<br><br>
<img src="../pic/frame.png">
<?php require("trailer.html");  ?>

