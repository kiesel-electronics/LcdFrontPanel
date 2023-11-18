/***********************************************************************|
| Modern AVR analog comparator library                                  |
|                                                                       |
| Simple_comparator.ino                                                 |
|                                                                       |
| A library for interfacing with the modern AVR analog comparator(s).   |
| Developed in 2019-2022 by MCUdude                                     |
| https://github.com/MCUdude/                                           |
|                                                                       |
| Ported to tinyAVR 2020-2021 by Spence Konde.                          |
|                                                                       |
| In this example we use the negative and positive input 0 of the       |
| comparator. The output goes high if the positive input is higher than |
| the negative input, and low otherwise.                                |
|
| In effect this turns the three pins into the inputs and outputs of a  |
| LM339-like chip, except that these are rail to rail
|***********************************************************************/

#include <Comparator.h>

void setup() {
  // Configure relevant comparator parameters
  Comparator.input_p = comparator::in_p::in0;      // Use positive input 0 (PA7)
  Comparator.input_n = comparator::in_n::in0;      // Use negative input 0 (PA6)
  Comparator.output = comparator::out::enable;     // Enable output on PIN_PA5 (digital pin 1)
  //                                   // or PIN_PA3 (digital pin 4) on ATtiny402/202)

  // Initialize comparator
  Comparator.init();

  // Start comparator
  Comparator.start();
}

void loop() {

}
