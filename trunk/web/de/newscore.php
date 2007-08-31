<?php
  $lang="de";
  $file="newscore.php";
  require("header.html");
  ?>
<h4><a href="idx.php">MuseScore</a>
   -- <a href="manual.php">Referenz</a>
   -- Create new Score</h4>

<h5>Create new Score from Scratch</h5>
The menu entry &lt;<b>Score/New</b>&gt; create an empty
new score. First you have to populate the empty sheet with instruments and
then you can add empty measures.
      <ul>
      <li>&lt;<b>I</b>&gt; popups the instrument menu</li>
      <li>&lt;<b>Ctrl+B</b>&gt; appends an empty measure</li>
      <li>then menu &lt;<b>Create/Measures</b>&gt; can be used to create
         an arbitrary number of empty measures
         </li>
      </ul>

Next steps are usually:
      <a href="clefs.php">set clef</a>,
      <a href="keys.php">set key</a>,
      <a href="timesig.php">set time signature</a>


<h5>Create new Score from Template</h5>

&lt;<b>N</b>&gt; popups the &lt;<b>Template</b>&gt; menu. Select a
template and click &lt;<b>Ok</b>&gt;. This is the simplest way to create
a new score. Note that template files are normal <i>MuseScore</i> files
stored in the template folder.

<h5>Create new Score with the New Wizard</h5>

---the new wizard is not implemented yet---

<?php require("trailer.html");  ?>

