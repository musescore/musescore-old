ProcessHTML.py create the PDF handbooks for MuseScore. 

ProcessHTML is written and maintained by David Bolton.
http://davidbolton.info

**********************************

SET UP INSTRUCTIONS

Code is written in Python 2.5. The actual PDF creation has several dependencies that must be installed first. See http://www.htmltopdf.org/ for details. I recommend pisa 3.0.31 or later since some earlier versions of pisa contain a image display bug. 

A connection to the Internet is required since ProcessHTML.py fetches HTML and images files from the MuseScore website.


USE INSTRUCTIONS

To create PDF handbooks run ProcessHTML.py from the command line.

By default the script will create the English handbook. To create other language handbooks use the standard two-letter language code as follows:

> Python ProcessHTML.py fr

To create all the language handbooks at once use "all" as the command line argument.

> Python ProcessHTML.py all


*********************************


CHANGE LOG

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



