#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
from optparse import OptionParser

# Save file (to debug output)
def save(contents, file_name):
    out_file = open(file_name,"w")
    out_file.write(contents)
    out_file.close()


# Get handbook from the web
def obtainHTML(url, verbose, language_code='en'):
    if verbose:
        print 'Obtain HTML from musescore.org'

    import urllib2
    sock = urllib2.urlopen(url)
    html_source = sock.read()
    sock.close()

    if verbose:
        print 'Save HTML sources to the sources directory'

    import os
    if not os.path.isdir('sources'):
        os.mkdir('sources')

    file_name = 'MuseScore-'+language_code+'.html'
    out_file = open('sources/'+file_name,"w")
    out_file.write(html_source)
    out_file.close()

    return html_source


# Give level 1 headings an anchor tag
# with a name attribute based on the heading text
def insertH1Anchors(html_source, anchors, verbose):
    if verbose:
        print 'Insert anchors for level one headings'

    import urllib2

    split = html_source.split('<h1')

    for i in range(1, len(split)):
        name = split[i][split[i].index('>')+1:split[i].index('</h1>')].decode("utf-8").lower().encode("utf-8").replace(" ","-")
        
        name = name.replace("&#039;","") #remove HTML encoding for French apostrophe
        name = name.replace(",","").replace("(","").replace(")","") #remove punctuation
        name = name.replace("-a-","-") #drop unnessary words
        name = urllib2.quote(name).lower() #percent encode name to match URLs
        name = name.replace('%c3%89','%c3%a9') #work-around for text encoding bug
        name = name.replace('%c5%81','%c5%82') #manually convert to lower case (Python doesn't seem know the lowercase equivalent of this charater
        name = name.replace('%c3%9a','%c3%ba') #manually convert Ú to lower case ú (Hungarian handbook)
        name = name.replace('%c3%96','%c3%b6') #manually convert Ö to lower case ö (Hungarian handbook)
        name = name.replace('%c3%9c','%c3%bc') #manually convert Ü to lower case ü (Hungarian handbook)
        name = name.replace('li%c3%b1as','li%c3%b1') #workaround incorrect url on website (Galacian handbook)
        split[i-1] = split[i-1] + '<a name="' + name + '"></a>'
        anchors.append(name)
        #print name
        
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
            #if verbose:
                #print ' * ' + html_soup('h1')[i].string
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

    chapter = 'Chapter [number]' #Default English
    
    if language_code == 'nl':
        chapter = 'Hoofdstuk [number]'
    elif language_code == 'ca':
        chapter = 'Cap&iacute;tol [number]'
    elif language_code == 'da':
        chapter = 'Kapitel [number]'
    elif language_code == 'de':
        chapter = 'Kapitel [number]'
    elif language_code == 'el':
        chapter = 'Κεφάλαιο [number]'
    elif language_code == 'es':
        chapter = 'Cap&iacute;tulo [number]'
    elif language_code == 'fi':
        chapter = 'Luku [number]'
    elif language_code == 'fr':
        chapter = 'Chapitre [number]'
    elif language_code == 'gl':
        chapter = 'Cap&iacute;tulo [number]'
    elif language_code == 'hu':
        chapter = '[number] Fejezet'
    elif language_code == 'it':
        chapter = 'Capitolo [number]'
    elif language_code == 'ja':
        chapter = '章[number]'
    elif language_code == 'nb':
        chapter = 'Kapittel [number]'
    elif language_code == 'pl':
        chapter = 'Rozdział [number]'
    elif language_code == 'pt-br':
        chapter = 'Capítulo [number]'
    elif language_code == 'ro':
        chapter = 'Capitolul [number]'
    elif language_code == 'ru':
        chapter = 'Глава [number]'
    elif language_code == 'zh-hans':
        chapter = '第 [number] 章'

    html_source = html_source.replace('<h1 class="print-title"></h1>','') #remove empty header

    counter = 1
    i = html_source.find('<h1 class="book-heading">')
    while i > -1:
        i = html_source.find('<h1 class="book-heading">',i+60)
        html_source = html_source[:i] + html_source[i:].replace('<h1 class="book-heading">','<span class="chapter">' + chapter.replace('[number]',str(counter)) + '</span> <h1 class="book-heading">',1)
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
    import re
    
    if verbose:
        print 'Link to internal anchors'

    split = html_source.split('href="')

    for i in range(1, len(split)):
        original_href = split[i][0:split[i].index('"')]
        if (original_href == "/"):
            original_href = "http://musescore.org/"

        # Fix links to h1 and h2 anchors
        internal_href = original_href.replace(handbook_url + '/','#').replace('%20','-').replace('%2520','-').decode("utf-8").lower().encode("utf-8")
        if internal_href[:1] == '#':
            internal_href = '#' + urllib2.quote(internal_href[1:]).lower() #percent encode URL to match anchor names
        
        # Fix links to h3 anchors
        if 'print/book/export/html/' in internal_href:
            internal_href = internal_href[internal_href.index('#'):]

        # Fix links to MuseScore website
        internal_href = internal_href.replace('../','')
        if internal_href[:1] == '/':
            internal_href = 'http://musescore.org/' + language_code + '/handbook/index' + internal_href
        
##        if internal_href[:3] == '../':
##            internal_href = 'http://musescore.org/en/' + internal_href[3:]

        url_language = re.search('/[a-z]{2}(-[a-z]{2})?/',internal_href)

        split[i] = split[i].replace(original_href, internal_href)
        if internal_href[1:] not in anchors:
            if internal_href[0:17] == 'http://musescore':
                if internal_href.find('/en/') > -1 and language_code != 'en': #check for website bug that sometimes links to English URL instead of local language URL
                    if internal_href.find('/node/1257') < 0: # check it is not a link to a bug report
                        print " * WARNING: English language link: ", internal_href
                elif internal_href.find('freelinking') > -1: #if url contains the "freelinking" text it means there is no matching page in the handbook
                    print " * WARNING: page does not exist: ", internal_href
                elif url_language:
                    if internal_href[url_language.start()+1:url_language.end()-1] != language_code: #check whether url language code and handbook language code match
                        print " * WARNING: Language does not match handbook ", internal_href
            elif internal_href[0:7] != 'mailto:' and internal_href[0:4] != 'http':
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
def addCustomStyles(html_source, verbose, language_code='en'):

    # Allow for language-specific fonts
    def externalFonts(full_css, language_code='en'):
        import re

        external_fonts = 'default'
        
        if (language_code == 'ja'):
            external_fonts = '''/* Normal */
                    @font-face {
                       font-family: "Sazanami Gothic";
                       src: url(font/sazanami-20040629/sazanami-gothic.ttf);
                    }
                    /* Normal */
                    @font-face {
                       font-family: "Sazanami Mincho";
                       src: url(font/sazanami-20040629/sazanami-mincho.ttf);
                    }
                    '''
            full_css = re.sub('DejaVu Sans','Sazanami Gothic',full_css)
            full_css = re.sub('DejaVu Serif','Sazanami Mincho',full_css)
        elif (language_code == 'zh-hans'):
            external_fonts = '''/* Normal */
                    @font-face {
                       font-family: "Zenhei";
                       src: url(font/zh-hans/wqy-zenhei.ttf);
                    }
                    /* Normal */
                    @font-face {
                       font-family: "Ukai";
                       src: url(font/zh-hans/ukai00.ttf);
                    }
                    '''
            full_css = re.sub('DejaVu Sans','Zenhei',full_css)
            full_css = re.sub('DejaVu Serif','Ukai',full_css)

        if (external_fonts != 'default'):
            pattern = re.compile(r'/\* Begin External Fonts \*/.*/\* End External Fonts \*/',re.DOTALL)
            full_css = re.sub(pattern, external_fonts, full_css)

        return full_css

    
    
    if verbose:
        print 'Add custom styles'

    css_file = open("pdfstyle.css","r")
    sock = css_file.read()
    css_file.close()

    sock = externalFonts(sock,language_code)
    if language_code == 'ja' or language_code == 'zh-hans':
        sock += 'body {-pdf-word-wrap:"CJK"}'
    html_source = html_source.replace('</head>','<style type="text/css" media="all">\n'
                                      + sock + '</style>\n</head>')

    if verbose:
        print 'Remove unwanted styles'
    from BeautifulSoup import BeautifulSoup
    BeautifulSoup.NESTABLE_TAGS.update({'kbd':[]}) # add 'kbd' to list of nestable tags
    html_soup = BeautifulSoup(html_source)

    for i in reversed( range(0, len(html_soup('style')) ) ):
        ##if html_soup('h1')[i].parent.parent.parent.name == 'div':
        if html_soup('style')[i].parent.name != 'head':
            if verbose:
                print ' * ' + str(i) + " " + html_soup('style')[i].name
            html_soup('style')[i].extract() # remove style from document

    for i in reversed( range(0, len(html_soup('link')) ) ):
        try:
            if verbose:
                print ' * external stylesheet: %s' % html_soup('link')[i].get("href")
        except:
            if verbose:
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

    if not os.path.isdir('sources'):
        os.mkdir('sources') 

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
            if os.path.isfile('sources/'+file_name): # if file already exists of local computer
                download_image = False

        if download_image:
            if verbose:
                print ' *', file_name

            sock = urllib.urlopen(url)
            out_file = open('sources/'+file_name,"wb")
            out_file.write(sock.read())
            out_file.close()
            sock.close()

    if unusual_urls > 0:
        print "WARNING:",unusual_urls,"unusual image urls found" #reports a bug with the website and language-dependent images
        


# Fix img src attribute now that I removed the base tag
def fixImgSrc(html_source, verbose):
    if verbose:
        print 'Fix image src attributes'

    html_source = html_source.replace('src="/sites/musescore.org/files/','src="sources/')
    html_source = html_source.replace('http://musescore.org/sites/all/modules/filefield/icons/protocons/16x16/mimetypes/image-x-generic.png','sources/image-x-generic.png') #Work-around for temporary bug

    return html_source


# Change first page
def addCoverPage(html_source, verbose):
    if verbose:
        print 'Add cover page'

    # Replace cover text for English version
    html_source = html_source.replace(
        '<a name="handbook"></a><h1 class="book-heading">Handbook</h1>\n<span class="print-link"></span><p>This handbook is for MuseScore version 0.9.2 and above. In order to help improving or translating the handbook, leave a post in the <a href="http://musescore.org/forum/8">MuseScore documentation forum</a> and apply to become a handbook contributor.</p>',
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


# Add spaces between characters so Japanese wraps
def insertSpaces(html_source):
    import re

    h = html_source

    cnt = 0
    space = " " # thin space
    space = " " # normal space
    space = "~" # alternate
    text_pattern = re.compile('>[^'+space+'<\n]([^<]+)<')

    try:
        while (re.search(text_pattern,h)) and (cnt < 99999):
            s = re.search(text_pattern,h)
            text = s.group(0)
            spaced_text = re.sub("(.)",space+"\\1",text)
            #print text+'\n'
            #print spaced_text+'\n'
            h = h.replace(text, spaced_text, 1)
            #print h[h.find(text)-100:h.find(text)+100],'\n'
            cnt = cnt + 1
            #print cnt,'\n\n'
    except:
        print "fail"
        raise


    h = re.sub(space+'>','>',h)
    h = re.sub('>'+space,'>',h)
    h = re.sub(space+'<','<',h)
    h = re.sub(space,' ',h)
    #print h
    
    html_source = h
    
    return html_source

# Generate and save PDF file
def generatePDF(html_source, verbose, language_code='en', pdf_parameter='openpdf'):
    file_name = 'MuseScore-' + language_code + '.pdf'
    print 'Create PDF handbook:',file_name

    try:
        import ho.pisa as pisa
        if verbose:
            pisa.showLogging()
    except:
        print "\nPisa library required from creating PDFs. See README.txt for information\n"
        return

    #import re
    #html_source = re.sub('(.)','\\1 ',html_source)
    #m = re.search(">([^<]*)<",h)
    #m.group(0)

    #if (language_code == 'ja'):
    #    html_source = insertSpaces(html_source)

    pdf = pisa.CreatePDF(
        html_source,
        file(file_name, "wb"))
    
    if not pdf.err and pdf_parameter=='openpdf':
            pisa.startViewer(file_name)


# Create handbook based on language parameter
def createHandbook(language_code, download_images='missing', pdf='openpdf', verbose=False, heading_switch=True,offline=False):

    url = ''
    internal = ''
    language_code = language_code.lower()
    language_code_pdf = language_code;
    if language_code == 'en':
        url = 'http://musescore.org/en/print/book/export/html/51'
        internal = 'http://musescore.org/en/handbook'
    elif language_code == 'ca':
        url = 'http://musescore.org/ca/print/book/export/html/3414'
        internal = 'http://musescore.org/ca/manual'
    elif language_code == 'da':
        url = 'http://musescore.org/da/print/book/export/html/1947'
        internal = 'http://musescore.org/da/håndbog'
    elif language_code == 'de':
        url = 'http://musescore.org/de/print/book/export/html/98'
        internal = 'http://musescore.org/de/handbuch'
    elif language_code == 'el':
        url = 'http://musescore.org/el/print/book/export/html/3533'
        internal = 'http://musescore.org/el/εγχειρίδιο' #%CE%B5%CE%B3%CF%87%CE%B5%CE%B9%CF%81%CE%AF%CE%B4%CE%B9%CE%BF
    elif language_code == 'es':
        url = 'http://musescore.org/es/print/book/export/html/137'
        internal = 'http://musescore.org/es/manual'
    elif language_code == 'fi':
        url = 'http://musescore.org/fi/print/book/export/html/1057'
        internal = 'http://musescore.org/fi/käsikirja' #k%e4sikirja'
    elif language_code == 'fr':
        url = 'http://musescore.org/fr/print/book/export/html/115'
        internal = 'http://musescore.org/fr/manuel'
    elif language_code == 'gl':
        url = 'http://musescore.org/gl/print/book/export/html/534'
        internal = 'http://musescore.org/gl/manual-galego'
    elif language_code == 'hu':
        url = 'http://musescore.org/hu/print/book/export/html/1935'
        internal = 'http://musescore.org/hu/kézikönyv' #k%C3%A9zik%C3%B6nyv
    elif language_code == 'it':
        url = 'http://musescore.org/it/print/book/export/html/772'
        internal = 'http://musescore.org/it/manuale'
    elif language_code == 'ja':
        url = 'http://musescore.org/ja/print/book/export/html/2696'
        internal = 'http://musescore.org/ja/ハンドブック' #%E3%83%8F%E3%83%B3%E3%83%89%E3%83%96%E3%83%83%E3%82%AF'
    elif language_code == 'nb':
        url = 'http://musescore.org/nb/print/book/export/html/2122'
        internal = 'http://musescore.org/nb/håndbok' #h%C3%A5ndbok'
    elif language_code == 'nl':
        url = 'http://musescore.org/nl/print/book/export/html/375'
        internal = 'http://musescore.org/nl/handboek'
    elif language_code == 'pl':
        url = 'http://musescore.org/pl/print/book/export/html/2495'
        internal = 'http://musescore.org/pl/podręcznik' #podr%C4%99cznik'
    elif language_code == 'pt-br':
        url = 'http://musescore.org/pt-br/print/book/export/html/1248'
        internal = 'http://musescore.org/pt-br/manual-pt-br' #podr%C4%99cznik'
        language_code_pdf = "pt_BR";
    elif language_code == 'ro':
        url = 'http://musescore.org/ro/print/book/export/html/3081'
        internal = 'http://musescore.org/ro/manual'
    elif language_code == 'ru':
        url = 'http://musescore.org/ru/print/book/export/html/2352'
        internal = 'http://musescore.org/ru/cправочник' #c%D0%BF%D1%80%D0%B0%D0%B2%D0%BE%D1%87%D0%BD%D0%B8%D0%BA'
    elif language_code == 'zh-hans':
        url = 'http://musescore.org/zh-hans/print/book/export/html/5541'
        internal = 'http://musescore.org/zh-hans/用户手册' #%E7%94%A8%E6%88%B7%E6%89%8B%E5%86%8C'
        language_code_pdf = "zh_CN";
        
    print "Create handbook for",language_code

    if not offline:
        html = obtainHTML(url, verbose, language_code)
    else:
        file_name = 'MuseScore-'+language_code+'.html'
        html_file = open('sources/'+file_name,"r")
        html = html_file.read()
        html_file.close()

    anchors = [] #list of anchor names throughout document
    
    html, anchors = insertH1Anchors(html, anchors, verbose)

    if heading_switch:
        html = markAsH2(html, verbose)
        html = changeToH2(html)

    html = chapterHeading(html, verbose, language_code)
  
    html, anchors = insertH3Anchors(html, anchors, verbose)
    html = fixLinks(html, anchors, verbose, internal, language_code)
    html = removeBaseTag(html, language_code)
    html = addCustomStyles(html, verbose, language_code)
    html = addPageNumbers(html, verbose)

    if download_images != 'local' and not offline:
        downloadImages(html, verbose, download_images)
    
    html = fixImgSrc(html, verbose)
    html = addCoverPage(html, verbose)
    html = addLastPage(html, verbose, internal, language_code)
    
    saveHTML(html, language_code)
    if pdf != 'nopdf':
        generatePDF(html, verbose, language_code_pdf, pdf)

    print ''


def main():
    language_choices = ['all','en','ca','da','de','el','es','fi','fr','gl','hu','it','ja','nb','nl','pl','pt-BR','ro','ru', 'zh-hans']
  
    parser = OptionParser()
    parser.add_option("-l","--lang", dest="language_code",
                    help="Specify language code for which to build manual",
                    choices=language_choices,
                    default="all")
    parser.add_option("-o","--offline", dest="offline",
                    help="Specify for offline mode",
                    action="store_true",
                    default=False)
    parser.add_option("-v","--verbose", dest="verbose",
                    action="store_true",
                    help="Verbose output",
                    default=False)
    parser.add_option("-t", "--type", dest="pdf",
                    help="PDF type",
                    choices=('default','pdf','openpdf','nopdf'),
                    default='pdf')
    parser.add_option("-n", "--no-heading", dest="heading_switch",
                    action="store_false",
                    help="Heading level switching off",
                    default=True)

    (opts, args) = parser.parse_args()

    language_code = opts.language_code
    pdf = opts.pdf
    heading_switch = opts.heading_switch
    verbose = opts.verbose
    offline = opts.offline
    download_images = 'missing'

    # Check for PDF library dependency
    if pdf != "nopdf":
        try:
            import ho.pisa as pisa
        except:
            print "ImportError: No module named ho.pisa"
            print "\nPisa library required from creating PDFs. See README.txt for information\n"
            return
    
    # Create Handbooks for all languages
    if language_code == 'all':
        print 'Creating handbooks for all languages...'
        if pdf == 'default':
            pdf = 'pdf'
        for language in language_choices:
            if language != "all":
                createHandbook(language, download_images, pdf, verbose, heading_switch, offline)

    # Create Handbook for specific language
    else:
        if pdf == 'default':
            pdf = 'openpdf'
        createHandbook(language_code, download_images, pdf, verbose, heading_switch, offline)

    print "Done"
    


if __name__ == '__main__':
    main() 
    #createHandbook("hu")
