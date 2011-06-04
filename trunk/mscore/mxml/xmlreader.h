/*
 * Summary: the XMLReader implementation
 * Description: API of the XML streaming API based on C# interfaces.
 *
 * Copy: See Copyright for the status of this software.
 *
 * Author: Daniel Veillard
 */

#ifndef __XML_XMLREADER_H__
#define __XML_XMLREADER_H__

#include "xmlversion.h"
#include "tree.h"
#include "xmlIO.h"

/**
 * xmlParserSeverities:
 *
 * How severe an error callback is when the per-reader error callback API
 * is used.
 */
enum xmlParserSeverities {
    XML_PARSER_SEVERITY_VALIDITY_WARNING = 1,
    XML_PARSER_SEVERITY_VALIDITY_ERROR = 2,
    XML_PARSER_SEVERITY_WARNING = 3,
    XML_PARSER_SEVERITY_ERROR = 4
    };

/**
 * xmlTextReaderMode:
 *
 * Internal state values for the reader.
 */
enum xmlTextReaderMode {
    XML_TEXTREADER_MODE_INITIAL = 0,
    XML_TEXTREADER_MODE_INTERACTIVE = 1,
    XML_TEXTREADER_MODE_ERROR = 2,
    XML_TEXTREADER_MODE_EOF =3,
    XML_TEXTREADER_MODE_CLOSED = 4,
    XML_TEXTREADER_MODE_READING = 5
   };

/**
 * xmlParserProperties:
 *
 * Some common options to use with xmlTextReaderSetParserProp, but it
 * is better to use xmlParserOption and the xmlReaderNewxxx and
 * xmlReaderForxxx APIs now.
 */
typedef enum {
    XML_PARSER_LOADDTD = 1,
    XML_PARSER_DEFAULTATTRS = 2,
    XML_PARSER_VALIDATE = 3,
    XML_PARSER_SUBST_ENTITIES = 4
} xmlParserProperties;

/**
 * xmlReaderTypes:
 *
 * Predefined constants for the different types of nodes.
 */
typedef enum {
    XML_READER_TYPE_NONE = 0,
    XML_READER_TYPE_ELEMENT = 1,
    XML_READER_TYPE_ATTRIBUTE = 2,
    XML_READER_TYPE_TEXT = 3,
    XML_READER_TYPE_CDATA = 4,
    XML_READER_TYPE_ENTITY_REFERENCE = 5,
    XML_READER_TYPE_ENTITY = 6,
    XML_READER_TYPE_PROCESSING_INSTRUCTION = 7,
    XML_READER_TYPE_COMMENT = 8,
    XML_READER_TYPE_DOCUMENT = 9,
    XML_READER_TYPE_DOCUMENT_TYPE = 10,
    XML_READER_TYPE_DOCUMENT_FRAGMENT = 11,
    XML_READER_TYPE_NOTATION = 12,
    XML_READER_TYPE_WHITESPACE = 13,
    XML_READER_TYPE_SIGNIFICANT_WHITESPACE = 14,
    XML_READER_TYPE_END_ELEMENT = 15,
    XML_READER_TYPE_END_ENTITY = 16,
    XML_READER_TYPE_XML_DECLARATION = 17
} xmlReaderTypes;

typedef enum {
    XML_TEXTREADER_NONE = -1,
    XML_TEXTREADER_START= 0,
    XML_TEXTREADER_ELEMENT= 1,
    XML_TEXTREADER_END= 2,
    XML_TEXTREADER_EMPTY= 3,
    XML_TEXTREADER_BACKTRACK= 4,
    XML_TEXTREADER_DONE= 5,
    XML_TEXTREADER_ERROR= 6
} xmlTextReaderState;

typedef enum {
    XML_TEXTREADER_NOT_VALIDATE = 0,
    XML_TEXTREADER_VALIDATE_DTD = 1,
    XML_TEXTREADER_VALIDATE_RNG = 2,
    XML_TEXTREADER_VALIDATE_XSD = 4
} xmlTextReaderValidate;

typedef void *  xmlTextReaderLocatorPtr;
typedef void   (XMLCALL *xmlTextReaderErrorFunc)(void *arg,
						 const char *msg,
						 xmlParserSeverities severity,
						 xmlTextReaderLocatorPtr locator);

//---------------------------------------------------------
//   XmlTextReader
//---------------------------------------------------------

class XmlTextReader {
   public:
      int				mode;	/* the parsing mode */
      xmlDocPtr			doc;    /* when walking an existing doc */
      xmlTextReaderValidate       validate;/* is there any validation */
      int				allocs;	/* what structure were deallocated */
      xmlTextReaderState		state;
      xmlParserCtxtPtr		ctxt;	/* the parser context */
      xmlSAXHandlerPtr		sax;	/* the parser SAX callbacks */
      xmlParserInputBufferPtr	input;	/* the input */
      startElementSAXFunc		startElement;/* initial SAX callbacks */
      endElementSAXFunc		endElement;  /* idem */
      startElementNsSAX2Func	startElementNs;/* idem */
      endElementNsSAX2Func	endElementNs;  /* idem */
      charactersSAXFunc		characters;
      cdataBlockSAXFunc		cdataBlock;
      unsigned int		base;	/* base of the segment in the input */
      unsigned int		cur;	/* current position in the input */
      xmlNodePtr			node;	/* current node */
      xmlNodePtr			curnode;/* current attribute node */
      int				depth;  /* depth of the current node */
      xmlNodePtr			faketext;/* fake xmlNs chld */
      int				preserve;/* preserve the resulting document */
      mutable xmlBufferPtr		buffer; /* used to return const xmlChar * */
      xmlDictPtr			dict;	/* the context dictionnary */

      /* entity stack when traversing entities content */
      xmlNodePtr         ent;          /* Current Entity Ref Node */
      int                entNr;        /* Depth of the entities stack */
      int                entMax;       /* Max depth of the entities stack */
      xmlNodePtr        *entTab;       /* array of entities */

      /* error handling */
      xmlTextReaderErrorFunc errorFunc;    /* callback function */
      void                  *errorFuncArg; /* callback function user argument */

      int                preserves;	/* level of preserves */
      int                parserFlags;	/* the set of options set */
      /* Structured error handling */
      xmlStructuredErrorFunc sErrorFunc;  /* callback function */

      const xmlChar* _CONSTSTR(const xmlChar* s) const { return xmlDictLookup(dict, s, -1); }
      int pushData();

   public:
      XmlTextReader();
      XmlTextReader(const char* buffer,
         int size, const char *URL, const char *encoding, int options);

      int setup(xmlParserInputBufferPtr input, const char *URL,
         const char *encoding, int options);

      bool init(const char* filename, const char* encoding, int options);
      bool init(const char *URI);
      bool init(xmlParserInputBufferPtr input, const char *URI);

      ~XmlTextReader();
      const xmlChar* name() const;
      const xmlChar* value() const;
      int getDepth() const;
      int attributeCount() const;
      int nodeType() const;
      bool isEmptyElement() const;
      bool moveToNextAttribute();
      int lineNumber() const;
      bool hasValue() const;
      bool read();
      xmlNodePtr currentNode() const { return curnode ? curnode : node; }
      };

typedef XmlTextReader* xmlTextReaderPtr;

/*
 * Constructors & Destructor
 */
XMLPUBFUN int XMLCALL
            xmlTextReaderSetup(xmlTextReaderPtr reader,
                   xmlParserInputBufferPtr input, const char *URL,
                   const char *encoding, int options);

/*
 * Iterators
 */

XMLPUBFUN xmlChar * XMLCALL
			xmlTextReaderReadString		(xmlTextReaderPtr reader);
XMLPUBFUN int XMLCALL
			xmlTextReaderReadAttributeValue	(xmlTextReaderPtr reader);

/*
 * Attributes of the node
 */
XMLPUBFUN int XMLCALL
			xmlTextReaderHasAttributes(xmlTextReaderPtr reader);
XMLPUBFUN int XMLCALL
			xmlTextReaderIsDefault	(xmlTextReaderPtr reader);
XMLPUBFUN int XMLCALL
			xmlTextReaderQuoteChar	(xmlTextReaderPtr reader);
XMLPUBFUN int XMLCALL
			xmlTextReaderReadState	(xmlTextReaderPtr reader);
XMLPUBFUN int XMLCALL
                        xmlTextReaderIsNamespaceDecl(xmlTextReaderPtr reader);

XMLPUBFUN const xmlChar * XMLCALL
		    xmlTextReaderConstBaseUri	(xmlTextReaderPtr reader);
XMLPUBFUN const xmlChar * XMLCALL
		    xmlTextReaderConstLocalName	(xmlTextReaderPtr reader);
XMLPUBFUN const xmlChar * XMLCALL
		    xmlTextReaderConstNamespaceUri(xmlTextReaderPtr reader);
XMLPUBFUN const xmlChar * XMLCALL
		    xmlTextReaderConstPrefix	(xmlTextReaderPtr reader);
XMLPUBFUN const xmlChar * XMLCALL
		    xmlTextReaderConstXmlLang	(xmlTextReaderPtr reader);
XMLPUBFUN const xmlChar * XMLCALL
		    xmlTextReaderConstString	(xmlTextReaderPtr reader,
						 const xmlChar *str);

/*
 * use the Const version of the routine for
 * better performance and simpler code
 */
XMLPUBFUN xmlChar * XMLCALL
			xmlTextReaderBaseUri	(xmlTextReaderPtr reader);
XMLPUBFUN xmlChar * XMLCALL
			xmlTextReaderLocalName	(xmlTextReaderPtr reader);
XMLPUBFUN xmlChar * XMLCALL
			xmlTextReaderName	(xmlTextReaderPtr reader);
XMLPUBFUN xmlChar * XMLCALL
			xmlTextReaderNamespaceUri(xmlTextReaderPtr reader);
XMLPUBFUN xmlChar * XMLCALL
			xmlTextReaderPrefix	(xmlTextReaderPtr reader);
XMLPUBFUN xmlChar * XMLCALL
			xmlTextReaderXmlLang	(xmlTextReaderPtr reader);
XMLPUBFUN xmlChar * XMLCALL
			xmlTextReaderValue	(xmlTextReaderPtr reader);

/*
 * Methods of the XmlTextReader
 */
XMLPUBFUN int XMLCALL
		    xmlTextReaderClose		(xmlTextReaderPtr reader);
XMLPUBFUN xmlChar * XMLCALL
		    xmlTextReaderGetAttributeNo	(xmlTextReaderPtr reader,
						 int no);
XMLPUBFUN xmlChar * XMLCALL
		    xmlTextReaderGetAttribute	(xmlTextReaderPtr reader,
						 const xmlChar *name);
XMLPUBFUN xmlChar * XMLCALL
		    xmlTextReaderGetAttributeNs	(xmlTextReaderPtr reader,
						 const xmlChar *localName,
						 const xmlChar *namespaceURI);
XMLPUBFUN xmlParserInputBufferPtr XMLCALL
		    xmlTextReaderGetRemainder	(xmlTextReaderPtr reader);
XMLPUBFUN xmlChar * XMLCALL
		    xmlTextReaderLookupNamespace(xmlTextReaderPtr reader,
						 const xmlChar *prefix);
XMLPUBFUN int XMLCALL
		    xmlTextReaderMoveToAttributeNo(xmlTextReaderPtr reader,
						 int no);
XMLPUBFUN int XMLCALL
		    xmlTextReaderMoveToAttribute(xmlTextReaderPtr reader,
						 const xmlChar *name);
XMLPUBFUN int XMLCALL
		    xmlTextReaderMoveToAttributeNs(xmlTextReaderPtr reader,
						 const xmlChar *localName,
						 const xmlChar *namespaceURI);
XMLPUBFUN int XMLCALL
		    xmlTextReaderMoveToFirstAttribute(xmlTextReaderPtr reader);
XMLPUBFUN int XMLCALL
		    xmlTextReaderMoveToElement	(xmlTextReaderPtr reader);
XMLPUBFUN int XMLCALL
		    xmlTextReaderNormalization	(xmlTextReaderPtr reader);
XMLPUBFUN const xmlChar * XMLCALL
		    xmlTextReaderConstEncoding  (xmlTextReaderPtr reader);

/*
 * Extensions
 */
XMLPUBFUN int XMLCALL
		    xmlTextReaderSetParserProp	(xmlTextReaderPtr reader,
						 int prop,
						 int value);
XMLPUBFUN int XMLCALL
		    xmlTextReaderGetParserProp	(xmlTextReaderPtr reader,
						 int prop);
XMLPUBFUN int XMLCALL
            xmlTextReaderGetParserColumnNumber(xmlTextReaderPtr reader);

XMLPUBFUN xmlNodePtr XMLCALL
		    xmlTextReaderPreserve	(xmlTextReaderPtr reader);
XMLPUBFUN xmlDocPtr XMLCALL
		    xmlTextReaderCurrentDoc	(xmlTextReaderPtr reader);
XMLPUBFUN xmlNodePtr XMLCALL
		    xmlTextReaderExpand		(xmlTextReaderPtr reader);
XMLPUBFUN int XMLCALL
		    xmlTextReaderNext		(xmlTextReaderPtr reader);
XMLPUBFUN int XMLCALL
		    xmlTextReaderNextSibling	(xmlTextReaderPtr reader);
XMLPUBFUN int XMLCALL
		    xmlTextReaderIsValid	(xmlTextReaderPtr reader);
XMLPUBFUN const xmlChar * XMLCALL
		    xmlTextReaderConstXmlVersion(xmlTextReaderPtr reader);
XMLPUBFUN int XMLCALL
		    xmlTextReaderStandalone     (xmlTextReaderPtr reader);


/*
 * Index lookup
 */
XMLPUBFUN long XMLCALL
		xmlTextReaderByteConsumed	(xmlTextReaderPtr reader);

/*
 * New more complete APIs for simpler creation and reuse of readers
 */
XMLPUBFUN xmlTextReaderPtr XMLCALL
		xmlReaderWalker		(xmlDocPtr doc);
XMLPUBFUN xmlTextReaderPtr XMLCALL
		xmlReaderForDoc		(const xmlChar * cur,
					 const char *URL,
					 const char *encoding,
					 int options);

/* XMLPUBFUN xmlTextReaderPtr XMLCALL
		xmlReaderForMemory	(const char *buffer,
					 int size,
					 const char *URL,
					 const char *encoding,
					 int options);
*/
XMLPUBFUN xmlTextReaderPtr XMLCALL
		xmlReaderForFd		(int fd,
					 const char *URL,
					 const char *encoding,
					 int options);
XMLPUBFUN xmlTextReaderPtr XMLCALL
		xmlReaderForIO		(xmlInputReadCallback ioread,
					 xmlInputCloseCallback ioclose,
					 void *ioctx,
					 const char *URL,
					 const char *encoding,
					 int options);

XMLPUBFUN int XMLCALL
		xmlReaderNewWalker	(xmlTextReaderPtr reader,
					 xmlDocPtr doc);
XMLPUBFUN int XMLCALL
		xmlReaderNewDoc		(xmlTextReaderPtr reader,
					 const xmlChar * cur,
					 const char *URL,
					 const char *encoding,
					 int options);
XMLPUBFUN int XMLCALL
		xmlReaderNewFile	(xmlTextReaderPtr reader,
					 const char *filename,
					 const char *encoding,
					 int options);
XMLPUBFUN int XMLCALL
		xmlReaderNewMemory	(xmlTextReaderPtr reader,
					 const char *buffer,
					 int size,
					 const char *URL,
					 const char *encoding,
					 int options);
XMLPUBFUN int XMLCALL
		xmlReaderNewFd		(xmlTextReaderPtr reader,
					 int fd,
					 const char *URL,
					 const char *encoding,
					 int options);
XMLPUBFUN int XMLCALL
		xmlReaderNewIO		(xmlTextReaderPtr reader,
					 xmlInputReadCallback ioread,
					 xmlInputCloseCallback ioclose,
					 void *ioctx,
					 const char *URL,
					 const char *encoding,
					 int options);
/*
 * Error handling extensions
 */

/**
 * xmlTextReaderErrorFunc:
 * @arg: the user argument
 * @msg: the message
 * @severity: the severity of the error
 * @locator: a locator indicating where the error occured
 *
 * Signature of an error callback from a reader parser
 */
XMLPUBFUN int XMLCALL
		    xmlTextReaderLocatorLineNumber(xmlTextReaderLocatorPtr locator);
XMLPUBFUN xmlChar * XMLCALL
		    xmlTextReaderLocatorBaseURI (xmlTextReaderLocatorPtr locator);
XMLPUBFUN void XMLCALL
		    xmlTextReaderSetErrorHandler(xmlTextReaderPtr reader,
						 xmlTextReaderErrorFunc f,
						 void *arg);
XMLPUBFUN void XMLCALL
		    xmlTextReaderSetStructuredErrorHandler(xmlTextReaderPtr reader,
							   xmlStructuredErrorFunc f,
							   void *arg);
XMLPUBFUN void XMLCALL
		    xmlTextReaderGetErrorHandler(xmlTextReaderPtr reader,
						 xmlTextReaderErrorFunc *f,
						 void **arg);

#endif /* __XML_XMLREADER_H__ */

