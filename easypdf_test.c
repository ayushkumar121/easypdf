#include "easypdf.h"

#include <stdio.h>

int main(void)
{
  PDFDocument document;
 
  easypdf_init(&document);

  easypdf_addpage(&document);
  easypdf_addpage(&document);
  easypdf_text(&document, "Hello PDF ~", 100, 100);
  
  easypdf_write(&document, stdout);
  easypdf_delete(&document);
  
  return 0;
}
