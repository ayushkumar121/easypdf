#ifndef EASYPDF_H
#define EASYPDF_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#define Allocator malloc

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

typedef bool PDFBool;
typedef int PDFInt;
typedef double PDFReal;
typedef char* PDFString;
typedef size_t PDFObjectId;

typedef struct PDFObject PDFObject;
typedef struct PDFObject
{
  PDFObjectId   id;
  PDFObjectType type;
  PDFString     name;

  union
  {
    PDFBool        _bool;
    PDFInt         _int;
    PDFReal        _real;
    PDFString      _str;
    PDFObject*     _node;
    PDFObjectId    _ref;
  } u;

  PDFObject* next;
  
} PDFObject;


typedef struct
{
  size_t object_count;
  PDFObject* root;
} PDFDocument;


typedef struct
{
  bool error;
  char* message;
} PDFResult;

/*Takes in the PDF documents and initialises it*/
void easypdf_init(PDFDocument* document);

/*
  Takes the PDF document renders write the final document to 
  stream
*/
void easypdf_write(PDFDocument* document, FILE* stream);

#endif // EASYPDF
