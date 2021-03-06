//Peter Xu px2117, Patrick Taylor pat2138
#ifndef _VGA_LED_H
#define _VGA_LED_H

#include <linux/ioctl.h>

#define VGA_LED_DIGITS 8

#define CENTER_X	320
#define CENTER_Y	240

typedef struct {
  unsigned char digit;    /* 0, 1, .. , VGA_LED_DIGITS - 1 */
  unsigned char segments; /* LSB is segment a, MSB is decimal point */
} vga_led_arg_t;

typedef struct {
	unsigned int cx;
	unsigned int cy;
} circ_center;

#define VGA_LED_MAGIC 'q'

/* ioctls and their arguments */
#define VGA_LED_WRITE_DIGIT _IOW(VGA_LED_MAGIC, 1, vga_led_arg_t *)
#define VGA_LED_READ_DIGIT  _IOWR(VGA_LED_MAGIC, 2, vga_led_arg_t *)
#define VGA_LED_WRITE_CENTER _IOW(VGA_LED_MAGIC, 3, vga_led_arg_t *)

#endif
