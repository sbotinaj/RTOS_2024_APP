#ifndef RGB_H
#define RGB_H

// Include any necessary headers here

// Define any constants or macros here
#define PIN_RED     25
#define PIN_GREEN   26
#define PIN_BLUE    27

// Declare any global variables or functions here
/**
 * @brief Initializes the RGB LED.
 *
 * This function initializes the RGB LED by configuring the GPIO pins for each color.
 *
 * @param GPIO_RED The GPIO pin number for the red color.
 * @param GPIO_GREEN The GPIO pin number for the green color.
 * @param GPIO_BLUE The GPIO pin number for the blue color.
 */
void init_rgb(int GPIO_RED, int GPIO_GREEN, int GPIO_BLUE);


/**
 * @brief Sets the RGB color.
 *
 * This function sets the RGB color by specifying the intensity of red, green, and blue components.
 *
 * @param red The intensity of the red component (0-255).
 * @param green The intensity of the green component (0-255).
 * @param blue The intensity of the blue component (0-255).
 */
void set_rgb_color(int red, int green, int blue);

#endif // RGB_H