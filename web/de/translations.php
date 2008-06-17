<?php
  $file="translations.php";
  $level="..";
  require("header.html");
  ?>
<!-- Main body -->
<div class="mainbody">

<h3>Translations</h3>

If you want to contribute a translation of <i>MuseScore</i> follow these hints.
The step for step instructions add as an example the new language "french" to
<i>MuseScore</i> by creating the file <code>mscore_fr.qm</code> (<code>fr</code>
is the two-letter ISO 3166 country code for french).
It assumes you have downloaded the latest <i>MuseScore</i> source from the
subversion repository and you are using linux:
<br/><br/>

<table>
  <tr>
    <td valign="top">Step&nbsp;1:&nbsp;&nbsp;</td>
    <td>
      Add a new language target by editing the script file
      <code>gen-qt-projectfile</code>. Insert line<br/>
      <code>echo "    $1/share/locale/mscore_fr.ts \\"</code>
      <br/>below <br/>
      <code>echo "TRANSLATIONS ="</code>:<br/>
      </td>
    </tr>

  <tr>
    <td valign="top">Step 2:</td>
    <td>
      Enter the build directory and type:<br/>
      <code>
        make lupdate
        </code>
      <br/>
      This generates the file "mscore/share/locale/mscore_fr.ts".
      </td>
    </tr>

  <tr>
    <td valign="top">Step 3:</td>
    <td>
      Edit "locale/mscore_fr.ts" manually or use the Qt <i>linguist</i> tool:<br/>
      <code>
        linguist mscore_fr.ts
        </code>
        </td>
    </tr>

  <tr>
    <td valign="top">Step 4:</td>
    <td>
      Save the edited file "mscore_fr.ts" from <i>linguist</i> and
      then in the build directory type<br/>
      <code>
        make lrelease
        </code>
      <br/>
      This produces the compressed translation file mscore_fr.qm.
      </td>
    </tr>

  <tr>
    <td valign="top">Step 5:</td>
    <td>
      install the *.qm files: as superuser type in the build dirctory:<br/>
      <code>
        make install
        </code>
      <br/>
      or for all debian based systems type<br/>
      <code>
        sudo make install
        </code>
      <br/>
      </td>
    </tr>


  <tr>
    <td valign="top">Step 6:</td>
    <td>
      Test:
            set the environment variable LC_ALL to the desired
            Language:<br/>
            <code>
              export LC_ALL=fr
              </code>
            <br/>
            Start mscore and test your translation:<br/>
            <code>
              mscore
              </code>
      </td>
    </tr>

  </table>
<br/>
If you are using the windows version please ask
<a href="mailto:ws at seh dot de">me</a> to create and install an
mscore_xx.ts file for you to translate. The qt <i>linguist</i> is part of
the qt package for windows available
<a href="http://trolltech.com/developer/downloads/qt/windows">here</a>
from trolltech.
</div>

<?php require("trailer.html");  ?>

