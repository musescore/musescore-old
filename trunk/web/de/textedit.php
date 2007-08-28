<?php
  $lang="de";
  $file="textedit.php";
  require("header.html");
  ?>
<h4><a href="idx.php">MuseScore</a>
   -- <a href="manual.php">Reference</a>
   -- Text Editing</h4>

  Double click on text to enter <i>edit mode</i>:
  <img src="../pic/textedit.png" align="center"><br>
  <br>
  In text edit mode the following commands are available:

  <ul>
    <li><code>&lt;<b>Ctrl+B</b>&gt;</code> toggles bold face</li>
    <li><code>&lt;<b>Ctrl+I</b>&gt;</code> toggles italic</li>
    <li><code>&lt;<b>Ctrl+U</b>&gt;</code> toggles underline</li>
    <li><code>&lt;<b>Up</b>&gt;</code> start superscript or end subscript if in
        subscript mode</li>
    <li><code>&lt;<b>Down</b>&gt;</code> start subscript or end superscript if in
       superscript mode</li>
    <li>move cursor: <code>&lt;<b>Home</b>&gt; &lt;<b>End</b>&gt; &lt;<b>Left</b>&gt;
       &lt;<b>Right</b>&gt;</code>
    <li><code>&lt;<b>Backspace</b>&gt;</code> remove character left from cursor</li>
    <li><code>&lt;<b>Delete</b>&gt;</code> remove character right from cursor</li>
    <li><code>&lt;<b>Return</b>&gt;</code> start new line</li>
    <li><code>&lt;<b>F2</b>&gt;</code> Show text palette. The text palette
        can be used to enter special characters and symbols.</li>
    </ul>

      See also:
            <a href="chordnames.php">Chord Names</a>,
            <a href="lyrics.php">Lyrics</a>

<?php require("trailer.html");  ?>

