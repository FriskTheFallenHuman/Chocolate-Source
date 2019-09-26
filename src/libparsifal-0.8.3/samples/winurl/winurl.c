#include <stdio.h>
#include "windows.h"
#include "urlmon.h"
#include "libparsifal/parsifal.h"

#define MAX_URILEN 512
XMLCH argbase[MAX_URILEN] = {'\0'}; /* used as base directory if arg[1] is specified as command line param */

int StartElement(void *UserData, const XMLCH *uri, const XMLCH *localName, const XMLCH *qName, LPXMLVECTOR atts);
int EndElement(void *UserData, const XMLCH *uri, const XMLCH *localName, const XMLCH *qName);
int PI(void *UserData, const XMLCH *target, const XMLCH *data);
int Characters(void *UserData, const XMLCH *chars, int cbChars);
int Comment(void *UserData, const XMLCH *chars, int cbChars);
int CharactersWide(void *UserData, const XMLCH *chars, int cbChars);
int StartCData(void *UserData);
int EndCData(void *UserData);
int DoctypeDecl(void *UserData, const XMLCH *name, const XMLCH *publicID, const XMLCH *systemID, int hasInternalSubset);
void ErrorHandler(LPXMLPARSER parser);
int ResolveEntity(void *UserData, LPXMLENTITY entity, LPBUFFEREDISTREAM reader);
int FreeInputData(void *UserData, LPXMLENTITY entity, LPBUFFEREDISTREAM reader);
int SkippedEntity(void *UserData, const XMLCH *name);
int urlstream(BYTE *buf, int cBytes, int *cBytesActual, void *inputData);
                
int StartElement(void *UserData, const XMLCH *uri, const XMLCH *localName, const XMLCH *qName, LPXMLVECTOR atts)
{	            
	if (*uri)   
		printf("\nStart tag: %s uri: %s localName: %s", qName, uri, localName);	
	else	
		printf("\nStart tag: %s", qName);		

	if (atts->length) {
		int i;	
		LPXMLRUNTIMEATT att;
		
		printf("\nhas %d attributes:", atts->length);
		for (i=0; i<atts->length; i++) {
			att = (LPXMLRUNTIMEATT)XMLVector_Get(atts, i);
			
			if (*att->uri)
				printf("\n  Name: %s Value: %s Prefix: %s LocalName: %s Uri: %s", 
					att->qname, att->value,
							att->prefix, att->localName, 
								att->uri);
			else 
				printf("\n  Name: %s Value: %s", 
					att->qname, att->value);		
		}
		
		/* this demonstrates XMLParser_GetNamedItem and XML_ABORT, just
			change the name "findthis" here to abort parsing when
			this attribute is encountered: */
		
		if (att = XMLParser_GetNamedItem(UserData, "findthis")) {
			printf("FOUND ATTRIBUTE %s value: %s\nAborting...\n", att->qname, att->value);
			return XML_ABORT;
		}
	}
	return 0;
}

int EndElement(void *UserData, const XMLCH *uri, const XMLCH *localName, const XMLCH *qName)
{	
	printf("\nEnd tag: %s", qName);
	return 0;
}

int Characters(void *UserData, const XMLCH *chars, int cbChars)
{	
	printf("\nText (%d bytes): ", cbChars);
	for (; cbChars; cbChars--, chars++) putc(*chars, stdout);
	return 0;
}

int SkippedEntity(void *UserData, const XMLCH *name)
{
	printf("\nskipped entity: %s", name);
	return 0;
}

int DoctypeDecl(void *UserData, const XMLCH *name, const XMLCH *publicID, const XMLCH *systemID, int hasInternalSubset)
{
	printf("\nDOCTYPE Name: %s", name);
	if (publicID) printf(" publicID: %s", publicID);
	if (systemID) printf(" systemID: %s", systemID);
	printf(" hasInternalSubset: %d", hasInternalSubset);
	return 0;	
}

int StartCData(void *UserData) 
{ 
	printf("\nStart CData tag\n");
	/* will call Characters to report CDATA contents */
	return 0; 
}

int EndCData(void *UserData) 
{ 
	printf("\nEnd CData tag");
	return 0; 
}

int Comment(void *UserData, const XMLCH *chars, int cbChars)
{
	printf("\nComment (%d bytes): ", cbChars);
	for (; cbChars; cbChars--, chars++) putc(*chars, stdout);
	return 0;
}

int PI(void *UserData, const XMLCH *target, const XMLCH *data)
{
	printf("\nPI tag - target: %s data: %s", target, ((*data) ? data : "no data"));
	return 0;
}

void ErrorHandler(LPXMLPARSER parser) 
{
	/* you should treat ERR_XMLP_ABORT as "user error" and give somekind of
	  description before returning from callbacks, otherwise we present parser error: */
	if (parser->ErrorCode != ERR_XMLP_ABORT) {
		XMLCH *SystemID = XMLParser_GetSystemID(parser);
		LPXMLENTITY curEnt = XMLParser_GetCurrentEntity(parser);		
		printf("\nParsing Error: %s\nCode: %d",
			parser->ErrorString, parser->ErrorCode);
		if (curEnt && !curEnt->systemID) printf("\nin entity: '%s'", curEnt->name);
		if (SystemID) printf("\nSystemID: '%s'", SystemID);
	}
	printf("\nLine: %d\nColumn: %d", parser->ErrorLine, parser->ErrorColumn);
}

/*
	Converts UTF-8 string to wchar_t string
	and shows converted string in MessageBoxW
	
	Set charactersHandler = CharactersWide if
	you want to test UTF-8 to wchar_t conversion for
	text content, you shouldn't run large documents with
	CharactersWide because those MessageBoxes can get annoying...
*/	
int CharactersWide(void *UserData, const XMLCH *chars, int cbChars)
{	
	wchar_t wstr[1024];
	int size;
		
	if (cbChars > 1023) cbChars = 1024;
	
	if ((size = MultiByteToWideChar(CP_UTF8, 0, chars, cbChars, wstr, 1024))) {
		*(wstr+size) = L'\0';
		MessageBoxW(NULL, wstr, L"WinUrl sample", MB_OK);
	}
	else {
		printf("Unicode conversion error!");
		return XML_ABORT;	
	}
	return 0;
}

int ResolveEntity(void *UserData, LPXMLENTITY entity, LPBUFFEREDISTREAM reader)
{
	IStream *pStm = NULL; /* pointer to COM stream */
	HRESULT hr;
	XMLCH uri[MAX_URILEN]; 
	XMLCH *base = XMLParser_GetPrefixMapping((LPXMLPARSER)UserData, "xml:base");
	
	if (argbase) {
		strcpy(uri, argbase);
		strcat(uri, entity->systemID);
	}
	else if (base) {
		printf("\nxml:base set to %s", base);
		strcpy(uri, base);
		strcat(uri, entity->systemID);
	}
	else {
		strcpy(uri, entity->systemID);
	}
	
	hr = URLOpenBlockingStream(0, uri, &pStm, 0,0);  	
	if (!SUCCEEDED(hr)) {
		printf("\nError opening url '%s'", uri);
		if (pStm) pStm->lpVtbl->Release(pStm);
		return XML_ABORT;	
	}
	
	reader->inputData = pStm;
	return 0;
}

int FreeInputData(void *UserData, LPXMLENTITY entity, LPBUFFEREDISTREAM reader)
{	
	IStream *pStm = (IStream*)reader->inputData;
	if (pStm) pStm->lpVtbl->Release(pStm);
	return 0;
}

int urlstream(BYTE *buf, int cBytes, int *cBytesActual, void *inputData)
{	
	/* calls IStream.Read in C COM way: */
	HRESULT hr = ((IStream*)inputData)->lpVtbl->Read((IStream*)inputData, buf, cBytes, cBytesActual);		
	return (*cBytesActual < cBytes || !SUCCEEDED(hr));
}

int main(int argc, char* argv[])
{	
	LPXMLPARSER parser;		
	IStream *pStm = NULL; /* pointer to COM stream */
	HRESULT hr;
	char szUrl[MAX_URILEN];
	
	if (argc == 2) {
		strcpy(argbase, argv[1]);
		printf("Base directory set to: %s\n", argbase);
	}
	
	printf("\nWINURL Parsifal sample\n\nUrl to parse: ");
	gets(szUrl);
	
	/* open the url stream: */	
	hr = URLOpenBlockingStream(0, szUrl, &pStm, 0,0);  	
	if (!SUCCEEDED(hr)) {
		puts("Error opening url!");
		if (pStm) pStm->lpVtbl->Release(pStm);
		exit(1);	
	}

	if (!XMLParser_Create(&parser)) {
		puts("Error creating parser!");
		exit(1);
	}

	parser->startElementHandler = StartElement;
	parser->endElementHandler = EndElement;	
	parser->charactersHandler = Characters; /* set to CharactersWide 
		to convert UTF-8 -> wchar_t */
	parser->processingInstructionHandler = PI;
	parser->commentHandler = Comment;
	parser->startCDATAHandler = StartCData;
	parser->endCDATAHandler = EndCData;
	parser->errorHandler = ErrorHandler;
	parser->startDTDHandler = DoctypeDecl;
	parser->skippedEntityHandler = SkippedEntity;
	parser->resolveEntityHandler = ResolveEntity;
	parser->externalEntityParsedHandler = FreeInputData;
	parser->UserData = parser;
	
	printf("XMLFlags:\n");
	printf("XMLFLAG_NAMESPACES: %d\n", _XMLParser_GetFlag(parser,XMLFLAG_NAMESPACES));
	printf("XMLFLAG_NAMESPACE_PREFIXES: %d\n", _XMLParser_GetFlag(parser,XMLFLAG_NAMESPACE_PREFIXES));
	printf("XMLFLAG_EXTERNAL_GENERAL_ENTITIES: %d\n", _XMLParser_GetFlag(parser,XMLFLAG_EXTERNAL_GENERAL_ENTITIES));
	printf("XMLFLAG_PRESERVE_GENERAL_ENTITIES: %d\n", _XMLParser_GetFlag(parser,XMLFLAG_PRESERVE_GENERAL_ENTITIES));
	printf("XMLFLAG_UNDEF_GENERAL_ENTITIES: %d\n", _XMLParser_GetFlag(parser,XMLFLAG_UNDEF_GENERAL_ENTITIES));
	printf("XMLFLAG_PRESERVE_WS_ATTRIBUTES: %d\n", _XMLParser_GetFlag(parser,XMLFLAG_PRESERVE_WS_ATTRIBUTES));
	
	XMLParser_Parse(parser, urlstream, pStm, NULL);
	
	if (pStm) pStm->lpVtbl->Release(pStm);
	XMLParser_Free(parser);
	
	return 0;
}


