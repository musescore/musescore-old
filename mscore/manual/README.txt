ProcessHTML.py creates the PDF handbooks for MuseScore. 

ProcessHTML is written and maintained by David Bolton.
http://davidbolton.info

**********************************

SET UP INSTRUCTIONS

Code is written for Python 2.6. The actual PDF creation 
has several dependencies that must be installed first. 
See http://www.htmltopdf.org/ for details.

A connection to the Internet is required since 
ProcessHTML.py fetches HTML and images files from the 
MuseScore website.


FURTHER SETUP AND PATCHES

If you get "AttributeError: 'NoneType' object has no 
attribute 'bands'" then see 
http://code.google.com/p/xhtml2pdf/issues/detail?id=65

To create the Japanese and Chinese handbooks see
http://code.google.com/p/xhtml2pdf/issues/detail?id=63

If you encounter
File "C:\Python26\lib\site-packages\reportlab\platypus\doctemplate.py", line 783, in handle_flowable
    raise LayoutError(ident)
reportlab.platypus.doctemplate.LayoutError
Comment line 783 in "C:\Python26\lib\site-packages\reportlab\platypus\doctemplate.py"


USE INSTRUCTIONS

To create PDF handbooks run ProcessHTML.py from the 
command line.

By default the script will create the English handbook. 
To create other language handbooks use the standard 
two-letter language code as follows:

> Python ProcessHTML.py -l fr

To create all the language handbooks at once use "all" 
as the command line argument.

> Python ProcessHTML.py all


*********************************

INSTRUCTIONS FOR ADDING NEW LANGUAGE

1. In chapterHeading() add the translation for 
   "Chapter". For example:

    elif language_code == 'nb':
        chapter = 'Kapittel'

2. In createHandbook() add the URL for the print 
   version of the handbook and the top-level URL for 
   the handbook on the website. Make sure the internal 
   URL using Unicode characters rather than escaped 
   charaters ('http://musescore.org/nb/håndbok' instead 
   of 'http://musescore.org/nb/h%C3%A5ndbok'). For 
   example

    elif language_code == 'nb':
        url = 'http://musescore.org/nb/print/book/export/html/2122'
        internal = 'http://musescore.org/nb/håndbok'

3. In main() the two-letter code the language_choices
   list. For example

    language_choices = ['all','en','nl','de','es','fi','fr','gl','it','ja','nb','ru','pl','pt-BR']


*********************************


CHANGE LOG

Version 1.6 (March 2010)
* Update style sheet for 0.9.6
* Fix for "/" link in English handbook
* Compatibility update: remove "www." from handbook links again (HTML from website changed)
* Add Hungarian, Catalan, Romanian, and Greek
* Work around lower case problem for a couple letters in Hungarian alphabet
* More flexibility with Chapter headings (numbers can go before the word chapter for Hungarian or right after without a space for Japanese)
* Update instructions in README file for adding new language

Version 1.5 (February 2010)
* Unix style command line options
* Fix: broken images
* Fix: removable of stylesheets (change in website revealed bug)
* Better handling of link checks

Version 1.4 (November 2009)
* Add sources and font directory to SVN
* Drop unimportant words (such as "a") from anchors to match URLs on website
* Repress pisa warnings unless verbos output requested 
* Repress information about external CSS unless versbose output requested
* Check whether URL's in handbook match language of handbook
* Use DejaVu font for all text (allows non-latin characters for Russian handbook)
* Ability to specify a font depending on the language (for example Japanese needs its own font)
* Add Russian, Japanese, Brazillian Portuguese, and Polish
* Compatibility update: add "www." back to handbook links (HTML from website changed)
* Add "./" to start of image src's so that OpenOffice.org can understand

Version 1.3 (August 2009):
* Compatibility update: remove "www." from handbook links (HTML from website changed)
* Update CSS for release of version 0.9.5 and to embed DejaVuSans font for Mac shortcut symbols
* Add Norwegian and wrote instructions for adding new languages
* Check for broken images
* Check for base and link tags before processing them

Version 1.2 (9 July 2009):
* Style sheet designed for pisa 3.0.31 (no longer designed for 3.0.27)
* Hides breadcrumb links
* When creating all handbooks, no longer open all the PDFs (to save memory)
* Create "files" directory if it doesn't already exit
* Does not download javascript files

Version 1.1 (17 April 2009):
* Compatibility updates for language dependent images (HTML from website changed)
* Compatibility updates for cover page and last page (HTML from website changed)
* Compatibility updates for page number (workaround for bug in pisa 3.0.31)
* Last version to have a style sheet designed for 3.0.27

Version 1.0 (12 March 2009):
* Added Italian handbook (it)
* Added Finnish handbook (fi)
* Compatibility updates for internal links (HTML from website changed)
* Show warning for URLs that contain the text "freelinking"
* Add UTF-8 comment to start of script (fixes URL encoding problems with accented "e" in French handbook)
* Made "missing" the default image parameter
* Added new PDF parameters: pdf, nopdf, openpdf (openpdf is default)

Version 0.9 (29 January 2009):
* Made screen output less verbose by default
* Percent escaped id attributes (necessary for PDF links to work with non-ASCII characters)
* Removed base tag from all languages
* Added numbered chapter headings
* Updated English title page

Version 0.8 (11 January 2009):
* Compatibility updates for new release of pisa 3.0.29
* Compatibility updates for internal links (HTML from website changed)

Version 0.7 (1 January 2009):
* Add support for all handbook translations
* Code clean up and modularization
* Fix level-two headings and other improvements
* Preliminary support for command-line arguments

Version 0.5 (30 December 2008):
* Experimental version for English handbook only



