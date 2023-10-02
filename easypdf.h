#ifndef EASYPDF_H
#define EASYPDF_H

#include "arena.h"

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

typedef enum 
{
  PDFTypeNull,
  PDFTypeBoolean,
  PDFTypeInt,
  PDFTypeReal,
  PDFTypeName,
  PDFTypeString,
  PDFTypeArray,
  PDFTypeDictionary,
  PDFTypeStream,
  PDFTypeObjectReference,
} PDFObjectType;

typedef struct PDFObject PDFObject;

typedef bool     PDFBool;
typedef int      PDFInt;
typedef double   PDFReal;
typedef char*    PDFString;
typedef uint8_t* PDFStream;
typedef size_t   PDFObjectId;

typedef struct PDFObject
{
  PDFObjectId   id;
  PDFObjectType type;
  PDFString     name;   // Set incase of dictionary entry
  size_t        length; // Set incase of stream object
  
  union
  {
    PDFBool        _bool;
    PDFInt         _int;
    PDFReal        _real;
    PDFString      _str;
    PDFStream      _buf;
    PDFObject*     _node;
    PDFObjectId    _ref;
  } u;

  PDFObject* next;
  
} PDFObject;

typedef struct
{
  /* Tree of all the objects in the pdf*/
  PDFObject*  root;
  size_t      object_count;

  /* Array of all the page references in the document*/
  PDFObject** pages;
  size_t      page_count;

  size_t      font_size;
  PDFObject*  resources;
  
  /* Arena used for all the allocations for the pdf */
  Arena* arena;
} PDFDocument;

typedef struct
{
  bool  error;
  char* message;
} PDFResult;

void easypdf_init(PDFDocument* document);

void easypdf_addpage(PDFDocument* document);

void easypdf_text(PDFDocument* document, 
		  char* text, int x, int y);

void easypdf_write(PDFDocument* document, FILE* stream);

void easypdf_delete(PDFDocument* document);

#endif // EASYPDF
