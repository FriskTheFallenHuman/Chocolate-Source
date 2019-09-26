/* Example that uses XMLVector, XMLStringbuf and XMLPool 
   Reads lines into memory from stdin and presents them backwards
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "libparsifal/xmlvect.h"
#include "libparsifal/xmlsbuf.h" /* includes XMLPool too */

#define CHKMEM(p) if (!(p)) { puts("memory allocation failure!"); exit(EXIT_FAILURE); }
#define SIZE_LINEBUF 1024
#define SIZE_POOLITEM 30

int main(int argc, char* argv[])
{	
	char linebuf[SIZE_LINEBUF];
	char *s;
	LPXMLPOOL pool;
	LPXMLVECTOR list;
	LPXMLSTRINGBUF sbuf;
	
	/* create a list of stringbuffers (lines) that will be allocated
	   dynamically by calling XMLVector_Append(list, NULL).
	   Vector grows in 6 * sizeof(XMLSTRINGBUF) sized chunks */
	XMLVector_Create(&list, 6, sizeof(XMLSTRINGBUF));
	CHKMEM(list);
	
	/* create a pool for XMLStringbuf that contains SIZE_POOLITEM sized 
	   fixed length strings, pool grows in 4 * SIZE_POOLITEM sized chunks */
	pool = XMLPool_Create(SIZE_POOLITEM, 4);
	CHKMEM(pool);

	while (!feof(stdin)) {
		fgets(linebuf, SIZE_LINEBUF, stdin);

		/* append new XMLStringbuf to list: */
		sbuf = XMLVector_Append(list, NULL);
		CHKMEM(sbuf);
		
		/* initialize it to use pool as long as string fits into
		   pool itemsize, otherwise XMLStringbuf will allocate 
		   more memory from heap. XMLStringbuf grows in SIZE_POOLITEM
		   sized chunks. */
		s = XMLStringbuf_InitUsePool(sbuf, SIZE_POOLITEM, 0, pool);
		CHKMEM(s);

		/* append string into stringbuf. (You can set debugger breakpoint
		   into XMLStringbuf_Append to see how memory gets allocated) */
		s = XMLStringbuf_Append(sbuf, linebuf, strlen(linebuf)+1);
		CHKMEM(s);
	}
	
	/* present the strings backwards: */
	if (list->length) {
		int i = list->length-1;
		while (i--) {
			sbuf = XMLVector_Get(list, i);
			CHKMEM(sbuf);
			printf("line %d: %s", i+1, sbuf->str);
			XMLStringbuf_Free(sbuf);
		}
	}
	
	/* clean up: */
	_XMLVector_RemoveAll(list);
	XMLVector_Free(list);
	XMLPool_FreePool(pool);
	return 0;
}
