# This Python file uses the following encoding: utf-8
import sys


# Get handbook from the web
def obtainHTML(url, verbose):
    if verbose:
        print 'Obtain HTML from musescore.org'
    import urllib2
##    urllib.urlcleanup()
    sock = urllib2.urlopen(url)
    html_source = sock.read()
    sock.close()

    return html_source


# Give level 1 headings an anchor tag
# with a name attribute based on the heading text
def insertH1Anchors(html_source, anchors, verbose):
    if verbose:
        print 'Insert anchors for level one headings'

    import urllib2

    split = html_source.split('<h1')

    for i in range(1, len(split)):
        name = split[i][split[i].index('>')+1:split[i].index('</h1>')].lower().replace(" ","-")

        name = name.replace("&#039;","") #remove HTML encoding for French apostrophe
        name = name.replace(",","").replace("(","").replace(")","") #remove punctuation
        name = urllib2.quote(name).lower() #percent encode name to match URLs
        name = name.replace('%c3%89','%c3%a9') #work-around for text encoding bug
        split[i-1] = split[i-1] + '<a name="' + name + '"></a>'
        anchors.append(name)
        
    html_source = '<h1'.join(split)

    return html_source, anchors


# Find h1 tags that should be h2 tags
# Mark these tags with a 'h2-heading' class
def markAsH2(html_source, verbose):
    if verbose:
        print "Find level one headings that should be level two headings:"

    from BeautifulSoup import BeautifulSoup
    BeautifulSoup.NESTABLE_TAGS.update({'kbd':[]}) # add 'kbd' to list of nestable tags
    html_soup = BeautifulSoup(html_source)

    for i in range(1, len(html_soup('h1'))):
        ##if html_soup('h1')[i].parent.parent.parent.name == 'div':
        if html_soup('h1')[i].parent['class'] == 'section-3':
            if verbose:
                print ' * ' + html_soup('h1')[i].string
            html_soup('h1')[i]['class'] = 'h2-heading ' + html_soup('h1')[i]['class']

    html_source = str(html_soup)

    return html_source


# Replace h1 tags with h2 tags when they are marked with the 'h2-heading' class 
def changeToH2(html_source):
    while html_source.find('<h1 class="h2-heading ') > -1:
        i = html_source.index('<h1 class="h2-heading ')
        html_source = html_source[:i] + html_source[i:].replace('<h1 class="h2-heading ','<h2 class="',1).replace('</h1>','</h2>',1)

    return html_source


# Give h1 tags a chapter heading
def chapterHeading(html_source, verbose, language_code):
    if verbose:
        print "Add chapter headings"

    chapter = 'Chapter' #Default English
    
    if language_code == 'nl':
        chapter = 'Hoofdstuk'
    elif language_code == 'de':
        chapter = 'Kapitel'
    elif language_code == 'es':
        chapter = 'Cap&iacute;tulo'
    elif language_code == 'gl':
        chapter = 'Cap&iacute;tulo'
    elif language_code == 'fi':
        chapter = 'Luku'
    elif language_code == 'fr':
        chapter = 'Chapitre'
    elif language_code == 'it':
        chapter = 'Capitolo'
    elif language_code == 'nb':
        chapter = 'Kapittel'


    html_source = html_source.replace('<h1 class="print-title"></h1>','') #remove empty header

    counter = 1
    i = html_source.find('<h1 class="book-heading">')
    while i > -1:
        i = html_source.find('<h1 class="book-heading">',i+60)
        html_source = html_source[:i] + html_source[i:].replace('<h1 class="book-heading">','<span class="chapter">' + chapter + ' ' + str(counter) + '</span> <h1 class="book-heading">',1)
        counter = counter + 1

    return html_source
    

# Give level 3 headings an anchor tag
# with a name attribute based on h3 id attribute
def insertH3Anchors(html_source, anchors, verbose):
    if verbose:
        print 'Insert anchors for level three headings'

    split = html_source.split('<h3 id="')

    for i in range(1, len(split)):
        id = split[i][0:split[i].index('"')]
        split[i-1] = split[i-1] + '<a name="' + id + '"></a>'
        anchors.append(id) # list of anchors throughout document

    html_source = '<h3 id="'.join(split)

    return html_source, anchors


# Fix links so that they link to headings within the document
def fixLinks(html_source, anchors, verbose, handbook_url, language_code='en'):
    import urllib2
    
    if verbose:
        print 'Link to internal anchors'

    split = html_source.split('href="')

    for i in range(1, len(split)):
        original_href = split[i][0:split[i].index('"')]

        # Fix links to h1 and h2 anchors
        internal_href = original_href.replace(handbook_url + '/','#').replace('%20','-').replace('%2520','-').lower()
        if internal_href[:1] == '#':
            internal_href = '#' + urllib2.quote(internal_href[1:]).lower() #percent encode URL to match anchor names

        # Fix links to h3 anchors
        if 'print/book/export/html/' in internal_href:
            internal_href = internal_href[internal_href.index('#'):]

        # Fix links to MuseScore website
        internal_href = internal_href.replace('../','')
        if internal_href[:1] == '/':
            internal_href = 'http://www.musescore.org/' + language_code + '/handbook/index' + internal_href
        
##        if internal_href[:3] == '../':
##            internal_href = 'http://musescore.org/en/' + internal_href[3:]
        
        split[i] = split[i].replace(original_href, internal_href)
        if internal_href[1:] not in anchors:
            if internal_href[0:7] == 'http://':
                if internal_href.find('/en/') > -1 and language_code != 'en': #check for website bug that sometimes links to English URL instead of local language URL
                    print " * WARNING: English language link: ", internal_href
                elif internal_href.find('freelinking') > -1: #if url contains the "freelinking" text it means there is no matching page in the handbook
                    print " * WARNING: page does not exist: ", internal_href
            elif internal_href[0:7] != 'mailto:' and internal_href[0:19] != 'https://help.ubuntu':
                print " * WARNING: no anchor tag corresponding to ", internal_href

    html_source = 'href="'.join(split)

    return html_source


# Remove base tag which interfers with internal links
def removeBaseTag(html_source, language_code='en'):

    from BeautifulSoup import BeautifulSoup
    BeautifulSoup.NESTABLE_TAGS.update({'kbd':[]}) # add 'kbd' to list of nestable tags
    html_soup = BeautifulSoup(html_source)

    if (html_source.find('base') > -1):   
        html_soup('base')[0].extract() # remove base tag from document

    html_source = str(html_soup)
    
    return html_source


# Link pdfstyle.css and remove css from website
def addCustomStyles(html_source, verbose):
    if verbose:
        print 'Add custom styles'

    css_file = open("pdfstyle.css","r")
    sock = css_file.read()
    css_file.close()

    html_source = html_source.replace('</head>','<style type="text/css" media="all">\n'
                                      + sock + '</style>\n</head>')

    if verbose:
        print 'Remove unwanted styles'
    from BeautifulSoup import BeautifulSoup
    BeautifulSoup.NESTABLE_TAGS.update({'kbd':[]}) # add 'kbd' to list of nestable tags
    html_soup = BeautifulSoup(html_source)

    for i in range(0, len(html_soup('style'))):
        ##if html_soup('h1')[i].parent.parent.parent.name == 'div':
        if html_soup('style')[i].parent.name != 'head':
            if verbose:
                print ' * ' + str(i) + " " + html_soup('style')[i].name
            html_soup('style')[i].extract() # remove style from document

    for i in range(0, len(html_soup('link'))):
        if html_soup('link')[i].get("rel", None) == "stylesheet":
            try:
                print ' * external stylesheet: %s' % html_soup('link')[i].get("href")
            except:
                print ' * external stylesheet'
            html_soup('link')[i].extract()

    html_source = str(html_soup)



    return html_source


# Add page number tag for PDF
def addPageNumbers(html_source, verbose):
    if verbose:
        print 'Add page numbers'

    html_source = html_source.replace('<body>', '<body>\n<div id="footerContent">\n<pdf:pagenumber />\n</div>')


    return html_source


# Get images from web
def downloadImages(html_source, verbose, download_images='all'):
    if verbose:
        print 'Obtain necessary images from musescore.org'

    import urllib
    import os

    i = 1
    unusual_urls = 0
    file_name = ""

    if not os.path.isdir('files'):
        os.mkdir('files') 

    broken_image = html_source.find('NOT FOUND:') #indicates a broken image on the website
    if broken_image > -1:
        broken_image_name = html_source[broken_image+11:html_source.find('</span>',broken_image)]
        print ' * WARNING: At least one broken image (' + broken_image_name + ')'
        
    while html_source[i:].find('src="') > -1:
        i = html_source[i:].index('src="') + i + 5
        url = 'http://musescore.org' + html_source[i : html_source[i:].index('"') + i]

        if url.find('files/') > -1:
            file_name = url[url.index('files/')+6:]
        else:
            unusual_urls = unusual_urls + 1
            if verbose:
                print "WARNING: Unusual image url:", url


        download_image = True

        if url.find('files/js/') > -1: #don't download javascript files
                download_image = False
                
        if download_images == 'missing':
            if os.path.isfile('files/'+file_name): # if file already exists of local computer
                download_image = False

        if download_image:
            if verbose:
                print ' *', file_name

            sock = urllib.urlopen(url)
            out_file = open('files/'+file_name,"wb")
            out_file.write(sock.read())
            out_file.close()
            sock.close()

    if unusual_urls > 0:
        print "WARNING:",unusual_urls,"unusual image urls found" #reports a bug with the website and language-dependent images
        


# Fix img src attribute now that I removed the base tag
def fixImgSrc(html_source, verbose):
    if verbose:
        print 'Fix image src attributes'

    html_source = html_source.replace('src="/sites/musescore.org/','src="')
    html_source = html_source.replace('http://www.musescore.org/sites/all/modules/filefield/icons/protocons/16x16/mimetypes/image-x-generic.png','files/image-x-generic.png') #Work-around for temporary bug

    return html_source


# Change first page
def addCoverPage(html_source, verbose):
    if verbose:
        print 'Add cover page'

    # Replace cover text for English version
    html_source = html_source.replace(
        '<a name="handbook"></a><h1 class="book-heading">Handbook</h1>\n<span class="print-link"></span><p>This handbook is for MuseScore version 0.9.2 and above. In order to help improving or translating the handbook, leave a post in the <a href="http://www.musescore.org/forum/8">MuseScore documentation forum</a> and apply to become a handbook contributor.</p>',
        '''
        <div style="text-align:center">
        <h1 style="border:0; padding-top:3cm">MuseScore Handbook</h1>
        <h2>MuseScore 0.9.5</h2>
        <p style="padding-top:12cm">English handbook written by Werner Schweer and David Bolton. Contributions by Thomas Bonte, Toby Smithe, and others. </p>
        <p>Copyright &copy; 2002-2009. Licensed under the <a href="http://creativecommons.org/licenses/by/3.0">Creative Commons Attribution 3.0</a> license</p>
        </div>
        ''')

    return html_source


# Change/fix last page
def addLastPage(html_source, verbose, handbook_url, language_code='en'):
    if verbose:
        print 'Add last page'

    import re

    #Replace Source link (that got changed with the link fixes)
    html_source = re.sub(
        '</strong> <a href="#.*">#.*</a>',
        '</strong> <a href="'+handbook_url+'">'+handbook_url+'</a>',
        html_source)

    return html_source


# Save modified HTML file
# which is ready for converting to PDF
def saveHTML(html_source, language_code='en'):
    file_name = 'MuseScore-' + language_code + '.html'
    print 'Save changes to HTML:',file_name

    out_file = open(file_name,"w")
    out_file.write(html_source)
    out_file.close()


# Generate and save PDF file
def generatePDF(html_source, language_code='en', pdf_parameter='openpdf'):
    file_name = 'MuseScore-' + language_code + '.pdf'
    print 'Create PDF handbook:',file_name

    import ho.pisa as pisa
    pisa.showLogging()

    pdf = pisa.CreatePDF(
        html_source,
        file(file_name, "wb"))
    
    if not pdf.err and pdf_parameter=='openpdf':
            pisa.startViewer(file_name)


# Create handbook based on language parameter
def createHandbook(language_code, download_images='missing', pdf='openpdf', verbose=False, heading_switch=True):

    url = ''
    internal = ''
    
    if language_code == 'en':
        url = 'http://musescore.org/en/print/book/export/html/51'
        internal = 'http://musescore.org/en/handbook'
    elif language_code == 'nl':
        url = 'http://musescore.org/nl/print/book/export/html/375'
        internal = 'http://musescore.org/nl/handboek'
    elif language_code == 'de':
        url = 'http://musescore.org/de/print/book/export/html/98'
        internal = 'http://musescore.org/de/handbuch'
    elif language_code == 'es':
        url = 'http://musescore.org/es/print/book/export/html/137'
        internal = 'http://musescore.org/es/manual'
    elif language_code == 'gl':
        url = 'http://musescore.org/gl/print/book/export/html/534'
        internal = 'http://musescore.org/gl/manual-galego'
    elif language_code == 'fi':
        url = 'http://musescore.org/fi/print/book/export/html/1057'
        internal = 'http://musescore.org/fi/käsikirja' #k%e4sikirja'
    elif language_code == 'fr':
        url = 'http://musescore.org/fr/print/book/export/html/115'
        internal = 'http://musescore.org/fr/manuel'
    elif language_code == 'it':
        url = 'http://musescore.org/it/print/book/export/html/772'
        internal = 'http://musescore.org/it/manuale'
    elif language_code == 'nb':
        url = 'http://musescore.org/nb/print/book/export/html/2122'
        internal = 'http://musescore.org/nb/håndbok' #h%C3%A5ndbok'

    print "Create handbook for",language_code

    html = obtainHTML(url, verbose)

    anchors = [] #list of anchor names throughout document
    
    html, anchors = insertH1Anchors(html, anchors, verbose)

    if heading_switch:
        html = markAsH2(html, verbose)
        html = changeToH2(html)

    html = chapterHeading(html, verbose, language_code)
        
    html, anchors = insertH3Anchors(html, anchors, verbose)
    html = fixLinks(html, anchors, verbose, internal, language_code)
    html = removeBaseTag(html, language_code)
    html = addCustomStyles(html, verbose)
    html = addPageNumbers(html, verbose)

    if download_images != 'local':
        downloadImages(html, verbose, download_images)
        
    html = fixImgSrc(html, verbose)
    html = addCoverPage(html, verbose)
    html = addLastPage(html, verbose, internal, language_code)
    
    saveHTML(html, language_code)
    if pdf != 'nopdf':
        generatePDF(html, language_code, pdf)

    print ''


def main():
    language_code = 'en'
    download_images = 'missing'
    pdf = 'default'
    heading_switch = True
    verbose = False

    # Parse command line variable for language (en, nl, de, es, gl, fr)
    if len(sys.argv) > 1:
        language_code = sys.argv[1]

    # Parse command line variable for images (all, local, missing)
    if len(sys.argv) > 2:
        if sys.argv[2] == 'openpdf':
            pdf = 'openpdf'
        elif sys.argv[2] == 'nopdf':
            pdf = 'nopdf'
        elif sys.argv[2] == 'pdf':
            pdf = 'pdf'
        else:
            download_images = sys.argv[2]

    # Parse command line variable for heading level switching which is processor heavy (on off)
    if len(sys.argv) > 3:
        if sys.argv[3] == 'off':
            heading_switch = False
        elif sys.argv[3] == 'verbose':
            verbose = True
        elif sys.argv[3] == 'openpdf':
            pdf = 'openpdf'
        elif sys.argv[3] == 'nopdf':
            pdf = 'nopdf'
        elif sys.argv[3] == 'pdf':
            pdf = 'pdf'
    
    # Create Handbooks for all languages
    if language_code == 'all':
        if pdf == 'default':
            pdf = 'pdf'
        createHandbook('en', download_images, pdf, verbose, heading_switch)
        createHandbook('nl', 'missing', pdf, verbose, heading_switch) # download only missing images for translations
        createHandbook('de', 'missing', pdf, verbose, heading_switch)
        createHandbook('es', 'missing', pdf, verbose, heading_switch)
        createHandbook('gl', 'missing', pdf, verbose, heading_switch)
        createHandbook('fi', 'missing', pdf, verbose, heading_switch)
        createHandbook('fr', 'missing', pdf, verbose, heading_switch)
        createHandbook('it', 'missing', pdf, verbose, heading_switch)
        createHandbook('nb', 'missing', pdf, verbose, heading_switch)

    # Create Handbook for specific language
    else:
        if pdf == 'default':
            pdf = 'openpdf'
        createHandbook(language_code, download_images, pdf, verbose, heading_switch)    

    print "Done"
    


if __name__ == '__main__':
     main() 
