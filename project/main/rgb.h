#ifndef RGB_H
#define RGB_H

// Include any necessary headers here

// Define any constants or macros here
#define PIN_RED     25
#define PIN_GREEN   26
#define PIN_BLUE    27

// Declare any global variables or functions here
void init_rgb(int GPIO_RED, int GPIO_GREEN, int GPIO_BLUE);
void set_rgb_color(int red, int green, int blue);

#endif // RGB_H