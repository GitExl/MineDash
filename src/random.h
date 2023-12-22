#ifndef RANDOM_H
#define RANDOM_H

extern unsigned char random_values[256];
extern unsigned char random_next;

#define RANDOM random_values[random_next++]

void random_init();

#endif
