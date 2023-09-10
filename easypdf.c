#include "easypdf.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>

PDFResult NOT_IMPLEMENTED = {
  .error = true,
  .message = "Not implemented",
};

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

PDFObject* easypdf_ref(PDFObjectId id) 
{
  PDFObject* object = (PDFObject*)Allocator(sizeof(PDFObject));
  object->type      = PDFTypeObjectReference;
  object->u._ref     = id;

  return object;
}

PDFObject* easypdf_name(PDFString val)
{
  PDFObject* object = (PDFObject*)Allocator(sizeof(PDFObject));
  object->type      = PDFTypeName;
  object->u._str     = val;

  return object;
}

PDFObject* easypdf_int(PDFInt val)
{
  PDFObject* object = (PDFObject*)Allocator(sizeof(PDFObject));
  object->type      = PDFTypeInt;
  object->u._int     = val;

  return object;
}

PDFObject* easypdf_str(PDFString val)
{
  PDFObject* object = (PDFObject*)Allocator(sizeof(PDFObject));
  object->type      = PDFTypeString;
  object->u._str     = val;

  return object;
}

PDFObject* easypdf_dict()
{
  PDFObject* object = (PDFObject*)Allocator(sizeof(PDFObject));
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

PDFObject* easypdf_array()
{
  PDFObject* object = (PDFObject*)Allocator(sizeof(PDFObject));
  object->type      = PDFTypeArray;
  object->next      = NULL; 

  return object;
}

void easypdf_array_add(PDFObject* array, PDFObject* object)
{
  assert(array->type == PDFTypeArray);
  (void)object;

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

  PDFObject *root = easypdf_dict();
  easypdf_addobject(document, root);

  // Adding Default font
  PDFObject* font = easypdf_dict();
  easypdf_addobject(document, font);
  easypdf_dict_add(font, "Type", easypdf_name("Font"));
  easypdf_dict_add(font, "Subtype", easypdf_name("Type1"));
  easypdf_dict_add(font, "Name", easypdf_name("F1"));
  easypdf_dict_add(font, "BaseFont", easypdf_name("Halvetica"));
  
  // Adding document resources
  PDFObject* res = easypdf_dict();
  easypdf_addobject(document, res);

  PDFObject* procset = easypdf_array();
  easypdf_array_add(procset, easypdf_name("PDF"));
  easypdf_array_add(procset, easypdf_name("Text"));
  easypdf_dict_add(res, "ProcSet", procset);
  
  PDFObject* fontset = easypdf_dict();
  easypdf_dict_add(fontset, "F1", easypdf_ref(font->id));
  easypdf_dict_add(res, "Font", fontset);

  PDFObject *page_tree = easypdf_dict();
  easypdf_addobject(document, page_tree);
  easypdf_dict_add(page_tree, "Type", easypdf_name("Pages"));

  // Adding the first page
  PDFObject* page = easypdf_dict();
  easypdf_addobject(document, page);
  easypdf_dict_add(page, "Type", easypdf_name("Page"));

  PDFObject* mediabox = easypdf_array();
  easypdf_array_add(mediabox, easypdf_int(0));
  easypdf_array_add(mediabox, easypdf_int(0));
  easypdf_array_add(mediabox, easypdf_int(612));
  easypdf_array_add(mediabox, easypdf_int(792));
  
  easypdf_dict_add(page, "MediaBox", mediabox);
  easypdf_dict_add(page, "Resources", easypdf_ref(res->id));
  // easypdf_dict_add(page, "Contents", easypdf_str(""));


  PDFObject* kids = easypdf_array();
  easypdf_array_add(kids , easypdf_ref(page->id));
  
  easypdf_dict_add(page_tree, "Kids", kids);
  easypdf_dict_add(page_tree, "Count", easypdf_int(1));
  
  easypdf_dict_add(root, "Type", easypdf_name("Catalog"));
  easypdf_dict_add(root, "Pages", easypdf_ref(page_tree->id));
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


    default:
      break;
  }

  return written;
}

void easypdf_write(PDFDocument* document, FILE* stream)
{
  int written = 0;
  int* offsets = (int*)Allocator(sizeof(int)*document->object_count+1);
  
  // PDF Header
  written += fprintf(stream, "%%PDF-1.4\n");
  written += fprintf(stream, "%%%%EOF\n");

  // PDF Body ie. Definitions of indirect objects
  PDFObject* object = document->root;
  while(object != NULL)
  {
    offsets[object->id] = written;
    written += fprintf(stream, "%ld 0 obj\n", object->id);
    written += easypdf_write_obj(object, stream);
    written += fprintf(stream, "\nendobj\n");

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
  free(offsets);

  // Adding Trailer
  PDFObject* trailer = easypdf_dict();
  easypdf_dict_add(trailer, "Size", easypdf_int(document->object_count+1));
  easypdf_dict_add(trailer, "Root", easypdf_ref(document->root->id));
  
  written += fprintf(stream, "trailer\n");
  written += easypdf_write_obj(trailer, stream);

  written += fprintf(stream, "startxref\n%ld\n", xref_offset);
  written += fprintf(stream, "%%%%EOF\n");
  
  fprintf(stderr,"[INFO] Written %d\n", written);
}
