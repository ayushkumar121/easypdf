#include "easypdf.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

PDFResult OK_RESULT = {
  .error = false,
  .message = "OK",
};

void easypdf_addobject(PDFDocument* document, PDFObject* object)
{
  object->id = (document->object_count++)+1;
  
  PDFObject* node = document->root;

  if (node == NULL) {
    document->root = object;
    return;
  }
  
  while (node->next != NULL) {
    node = node->next;
  }

  node->next = object;
}

PDFObject* easypdf_ref(Arena* arena, PDFObjectId id) 
{
  PDFObject* object = (PDFObject*)arena_alloc(arena, sizeof(PDFObject));
  object->type      = PDFTypeObjectReference;
  object->u._ref    = id;
  object->next      = NULL; 

  return object;
}

PDFObject* easypdf_name(Arena* arena, PDFString val)
{
  PDFObject* object = (PDFObject*)arena_alloc(arena, sizeof(PDFObject));
  object->type      = PDFTypeName;
  object->u._str    = val;
  object->next      = NULL; 

  return object;
}

PDFObject* easypdf_int(Arena* arena, PDFInt val)
{
  PDFObject* object = (PDFObject*)arena_alloc(arena, sizeof(PDFObject));
  object->type      = PDFTypeInt;
  object->u._int    = val;
  object->next      = NULL; 

  return object;
}

PDFObject* easypdf_str(Arena* arena, PDFString str)
{
  PDFObject* object = (PDFObject*)arena_alloc(arena, sizeof(PDFObject));
  object->type      = PDFTypeString;
  object->u._str    = str;
  object->next      = NULL; 

  return object;
}

PDFObject* easypdf_stream(Arena* arena, uint8_t* data, size_t size)
{
  PDFObject* object = (PDFObject*)arena_alloc(arena, sizeof(PDFObject));
  object->type      = PDFTypeStream;
  object->length    = size;
  object->u._buf    = data;
  object->next      = NULL; 

  return object;
}

PDFObject* easypdf_dict(Arena* arena)
{
  PDFObject* object = (PDFObject*)arena_alloc(arena, sizeof(PDFObject));
  object->type      = PDFTypeDictionary;
  object->next      = NULL; 

  return object;
}

void easypdf_dict_add(PDFObject* dict, PDFString name, PDFObject* object)
{
  assert(dict->type == PDFTypeDictionary);
  object->name = name;
  
  PDFObject* node = dict->u._node;
  if (node == NULL)
  {
    dict->u._node = object;
    return;
  }

  while(node->next != NULL) {
    node = node->next;
  }

  node->next = object;
}

PDFObject* easypdf_array(Arena* arena)
{
  PDFObject* object	= (PDFObject*)arena_alloc(arena, sizeof(PDFObject));
  object->type		= PDFTypeArray;
  object->next		= NULL; 

  return object;
}

void easypdf_array_add(PDFObject* array, PDFObject* object)
{
  assert(array->type == PDFTypeArray);

  PDFObject* node = array->u._node;
  if (node == NULL) {
    array->u._node= object;
    return;
  }
  
  while(node->next != NULL) {
    node = node->next;
  }

  node->next = object;
}

void easypdf_init(PDFDocument* document)
{
  //Zero setting entire pdf object justincase
  memset(document, 0, sizeof(PDFDocument));

  document->arena = arena_new(1024);
  Arena* arena = document->arena;
  
  PDFObject *root = easypdf_dict(arena);
  easypdf_addobject(document, root);

  // Adding Default font
  PDFObject* font = easypdf_dict(arena);
  easypdf_addobject(document, font);
  easypdf_dict_add(font, "Type", easypdf_name(arena, "Font"));
  easypdf_dict_add(font, "Subtype", easypdf_name(arena, "Type1"));
  easypdf_dict_add(font, "Name", easypdf_name(arena, "F0"));
  easypdf_dict_add(font, "BaseFont", easypdf_name(arena, "Helvetica"));
  
  // Adding document resources
  PDFObject* res = easypdf_dict(arena);
  easypdf_addobject(document, res);
  
  document->resources = res;

  PDFObject* procset = easypdf_array(arena);
  easypdf_array_add(procset, easypdf_name(arena, "PDF"));
  easypdf_array_add(procset, easypdf_name(arena,"Text"));
  easypdf_dict_add(res, "ProcSet", procset);
  
  PDFObject* fontset = easypdf_dict(arena);
  easypdf_dict_add(fontset, "F1", easypdf_ref(arena, font->id));
  easypdf_dict_add(res, "Font", fontset);
 
  easypdf_dict_add(root, "Type", easypdf_name(arena, "Catalog"));
 }

void easypdf_addpage(PDFDocument* document)
{
  Arena* arena = document->arena;
  
  PDFObject* page = easypdf_dict(arena);
  easypdf_addobject(document, page);
  easypdf_dict_add(page, "Type", easypdf_name(arena, "Page"));
  
  PDFObject** pages = (PDFObject**)arena_alloc(arena, sizeof(PDFObject*)*(document->page_count+1));
  
  if ( document->pages != NULL ) {
    size_t n = sizeof(size_t)*document->page_count;
    memmove(pages, document->pages, n);
  }

  document->pages = pages;
  document->pages[document->page_count++] = page;
}

void easypdf_text(PDFDocument* document, 
		  char* text, int x, int y)
{ 
  assert(document->page_count > 0);

  Arena* arena = document->arena;
  
  PDFString postscript= ""
    "BT\n"
    "/F0 10 Tf\n"
    "%d %d Td\n"
    "(%s)Tj\n"
    "ET";

  // Allocate buffer for text
  char* data = (char*)arena_alloc(arena, strlen(text)+strlen(postscript));
  sprintf(data, postscript, x, y,  text);
 
  size_t data_size = strlen(data);
 
  PDFObject* contents = easypdf_stream(arena, (uint8_t*)data, data_size);
  easypdf_addobject(document, contents);

  PDFObject* mediabox = easypdf_array(arena); 
  easypdf_array_add(mediabox, easypdf_int(arena, 0)); 
  easypdf_array_add(mediabox, easypdf_int(arena, 0)); 
  easypdf_array_add(mediabox, easypdf_int(arena, 612)); 
  easypdf_array_add(mediabox, easypdf_int(arena, 792));

  PDFObject* page = document->pages[document->page_count-1];
  
  easypdf_dict_add(page, "MediaBox", mediabox); 
  easypdf_dict_add(page, "Resources", easypdf_ref(arena, document->resources->id)); 
  easypdf_dict_add(page, "Contents", easypdf_ref(arena, contents->id)); 
}

int easypdf_write_obj(PDFObject* object, FILE* stream)
{
  int written = 0;

  switch (object->type) {
    case PDFTypeInt:
      written += fprintf(stream, "%d", object->u._int);
    break;

    case PDFTypeReal:
      written += fprintf(stream, "%f", object->u._real);
    break;

    case PDFTypeObjectReference:
      written += fprintf(stream, "%ld 0 R", object->u._ref);
    break;

    case PDFTypeName:
      written += fprintf(stream, "/%s", object->u._str);
    break;
    
    case PDFTypeString:
      written += fprintf(stream, "(%s)", object->u._str);
    break;

  case PDFTypeArray:
      {
        PDFObject* node = object->u._node;
        written += fprintf(stream, "[ ");
        while(node != NULL) {
          written += easypdf_write_obj(node, stream);
          written += fprintf(stream, " ");
          node = node->next;
        }
        written += fprintf(stream, "]");
      }
    break;

  case PDFTypeDictionary:
      {
        PDFObject* node = object->u._node;
        written += fprintf(stream, "<<\n");
        while(node != NULL) {
          written += fprintf(stream, " /%s ", node->name);
          written += easypdf_write_obj(node, stream);
          written += fprintf(stream, "\n");
          node = node->next;
        }
        written += fprintf(stream, ">>");
      }
    break;

  case PDFTypeStream:
      {
        written += fprintf(stream, "<<\n");
        written += fprintf(stream, " /Length %ld\n", object->length);
        written += fprintf(stream, ">>\n");
        written += fprintf(stream, "stream\n");
        written += fprintf(stream, "%.*s", 
          (int)object->length, 
          (char*)object->u._buf);
        written += fprintf(stream, "\nendstream");
      }
    break;


    default:
      assert(false && "Unreachabled");
      break;
  }

  return written;
}

void easypdf_addpagetree(PDFDocument* document)
{
  Arena* arena = document->arena;
  PDFObject* root = document->root;
    
  PDFObject* page_tree = easypdf_dict(arena);
  easypdf_addobject(document, page_tree);
  easypdf_dict_add(page_tree, "Type", easypdf_name(arena, "Pages"));

  PDFObject* kids = easypdf_array(arena);
  for (size_t i=0; i<document->page_count; ++i) {
    PDFObject* page = document->pages[i];
    easypdf_array_add(kids , easypdf_ref(arena, page->id));
  }
  
  easypdf_dict_add(page_tree, "Kids", kids);
  easypdf_dict_add(page_tree, "Count", easypdf_int(arena, document->page_count));
  easypdf_dict_add(root, "Pages", easypdf_ref(arena, page_tree->id));  
}

void easypdf_write(PDFDocument* document, FILE* stream)
{
  easypdf_addpagetree(document);
  
  Arena* arena = document->arena;  
  int written = 0;
  int offsets_size = sizeof(int)*(document->object_count+1);
  int* offsets = (int*)arena_alloc(arena, offsets_size);
  
  // PDF Header
  written += fprintf(stream, "%%PDF-1.6\n");
  written += fprintf(stream, "%%âãÏÓ\n");

  // PDF Body ie. Definitions of indirect objects
  PDFObject* object = document->root;
  while(object != NULL)
  {
    offsets[object->id] = written;
    written += fprintf(stream, "%ld 0 obj\n", object->id);
    written += easypdf_write_obj(object, stream);
    written += fprintf(stream, "\nendobj\n\n");

    object = object->next;    
  }

  size_t xref_offset = written;

  // Adding cross reference table
  written += fprintf(stream, "xref\n");
  written += fprintf(stream, "0 %ld\n", document->object_count+1);
  written += fprintf(stream, "%010d 65535 f\n", 0);
  for (size_t i=1; i<document->object_count+1; i++) {
    written += fprintf(stream, "%010d 00000 n\n", offsets[i]);
  }

  // Adding Trailer
  PDFObject* trailer = easypdf_dict(arena);
  easypdf_dict_add(trailer, "Size", easypdf_int(arena, document->object_count+1));
  easypdf_dict_add(trailer, "Root", easypdf_ref(arena, document->root->id));
  
  written += fprintf(stream, "trailer\n");
  written += easypdf_write_obj(trailer, stream);

  written += fprintf(stream, "startxref\n%ld\n", xref_offset);
  written += fprintf(stream, "%%%%EOF\n");
  
  fprintf(stderr,"INFO: Written %d\n", written);
}

void easypdf_delete(PDFDocument* document)
{
  arena_free(document->arena);
}
