<?php
  $lang="en";
  $file="download.php";
  require("header.html");
  ?>
<div id="intro">
  <h2> Downloads</h2>
  <h3> Sources</h3>
    <i>MuseScore</i> sources are available at
  <a href="http://sourceforge.net/project/showfiles.php?group_id=109430">SourceForge</a>.
  <br><br>
  <h3> Binaries</h3>
  <b>Windows: </b>
     <a href="http://sourceforge.net/project/showfiles.php?group_id=109430">SourceForge</a>.
     <br>(The window version has no sequencer and no synthesizer.)

  <h3> Subversion Sourcecode Repository</h3>
  The latest <i>MuseScore</i> code is always available in the
  SourceForge SVN repository.<br>
  For help please look at the SourceForge project page.

</div>

  <h2> Requirements</h2>
<ul>
  <li> <a href="ftp://ftp.trolltech.com/qt/source">qt gui lib version 4.3</a>
      or newer
      <br>
      <i>MuseScore</i> does probably not compile with older versions.<br>

      Precompiled packages are often split into "runtime package" and
      "development package". You need to install both packages.

  <li> a recent X11 with freetype2 support and render extension
        (which gives antialiased screen fonts)

  <li> <a href="http://www.alsa-project.org/">ALSA</a>
       Version 1.0 or newer; this is only used for
       midi keyboard entry

  <li> CMake 2.4
</ul>

The <i>MuseScore</i> development platform is Kubuntu 7.04.

<?php require("trailer.html");  ?>

