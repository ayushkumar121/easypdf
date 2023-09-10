#include "easypdf.h"

#include <stdio.h>

int main()
{
  PDFDocument document;

  easypdf_init(&document);

  easypdf_write(&document, stdout);
  
  easypdf_delete(&document);
  
  return 0;
}
