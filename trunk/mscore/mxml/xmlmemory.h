/*
 * Summary: interface for the memory allocator
 * Description: provides interfaces for the memory allocator,
 *              including debugging capabilities.
 *
 * Copy: See Copyright for the status of this software.
 *
 * Author: Daniel Veillard
 */


#ifndef __DEBUG_MEMORY_ALLOC__
#define __DEBUG_MEMORY_ALLOC__

#include <stdio.h>
#include "xmlversion.h"

typedef void (XMLCALL *xmlFreeFunc)(void *mem);
typedef void *(LIBXML_ATTR_ALLOC_SIZE(1) XMLCALL *xmlMallocFunc)(size_t size);
typedef void *(XMLCALL *xmlReallocFunc)(void *mem, size_t size);
typedef char *(XMLCALL *xmlStrdupFunc)(const char *str);

/*
 * The way to overload the existing functions.
 * The xmlGc function have an extra entry for atomic block
 * allocations useful for garbage collected memory allocators
 */
XMLPUBFUN int XMLCALL
	xmlMemSetup	(xmlFreeFunc freeFunc,
			 xmlMallocFunc mallocFunc,
			 xmlReallocFunc reallocFunc,
			 xmlStrdupFunc strdupFunc);
XMLPUBFUN int XMLCALL
	xmlMemGet	(xmlFreeFunc *freeFunc,
			 xmlMallocFunc *mallocFunc,
			 xmlReallocFunc *reallocFunc,
			 xmlStrdupFunc *strdupFunc);
XMLPUBFUN int XMLCALL
	xmlGcMemSetup	(xmlFreeFunc freeFunc,
			 xmlMallocFunc mallocFunc,
			 xmlMallocFunc mallocAtomicFunc,
			 xmlReallocFunc reallocFunc,
			 xmlStrdupFunc strdupFunc);
XMLPUBFUN int XMLCALL
	xmlGcMemGet	(xmlFreeFunc *freeFunc,
			 xmlMallocFunc *mallocFunc,
			 xmlMallocFunc *mallocAtomicFunc,
			 xmlReallocFunc *reallocFunc,
			 xmlStrdupFunc *strdupFunc);

/*
 * These are specific to the XML debug memory wrapper.
 */
XMLPUBFUN int XMLCALL
	xmlMemUsed	(void);
XMLPUBFUN int XMLCALL
	xmlMemBlocks	(void);
XMLPUBFUN void XMLCALL
	xmlMemDisplay	(FILE *fp);
XMLPUBFUN void XMLCALL
	xmlMemDisplayLast(FILE *fp, long nbBytes);
XMLPUBFUN void XMLCALL
	xmlMemShow	(FILE *fp, int nr);
XMLPUBFUN void XMLCALL
	xmlMemoryDump	(void);
XMLPUBFUN void * XMLCALL
	xmlMemMalloc	(size_t size) LIBXML_ATTR_ALLOC_SIZE(1);
XMLPUBFUN void * XMLCALL
	xmlMemRealloc	(void *ptr,size_t size);
XMLPUBFUN void XMLCALL
	xmlMemFree	(void *ptr);
XMLPUBFUN char * XMLCALL
	xmlMemoryStrdup	(const char *str);
XMLPUBFUN void * XMLCALL
	xmlMallocLoc	(size_t size, const char *file, int line) LIBXML_ATTR_ALLOC_SIZE(1);
XMLPUBFUN void * XMLCALL
	xmlReallocLoc	(void *ptr, size_t size, const char *file, int line);
XMLPUBFUN void * XMLCALL
	xmlMallocAtomicLoc (size_t size, const char *file, int line) LIBXML_ATTR_ALLOC_SIZE(1);
XMLPUBFUN char * XMLCALL
	xmlMemStrdupLoc	(const char *str, const char *file, int line);


#ifndef __XML_GLOBALS_H
#ifndef __XML_THREADS_H__
#include "threads.h"
#include "globals.h"
#endif
#endif

#endif  /* __DEBUG_MEMORY_ALLOC__ */

