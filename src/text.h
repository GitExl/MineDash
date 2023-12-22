#ifndef TEXT_H
#define TEXT_H

void text_load();
void text_clear();
void text_hud_clear();
void text_write(unsigned char x, unsigned char y, unsigned char colors, char* str);
void text_write_center(const unsigned char y, const unsigned char colors, char* str);

#endif
