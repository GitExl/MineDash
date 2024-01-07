#ifndef TEXT_H
#define TEXT_H

void text_load();
void text_clear();
void text_hud_clear();
void text_write(unsigned char x, unsigned char y, unsigned char colors, char* str);
void text_write_center(const unsigned char y, const unsigned char colors, char* str);
void text_clear_line(const unsigned char line);
void text_box(const unsigned char x, const unsigned char y, const unsigned char width, const unsigned char height, const unsigned char colors);
void text_blind();
void text_blind_clear();

#endif
