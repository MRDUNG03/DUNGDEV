#include "hal_delay.h"
#include "hal_types.h"

void delay_us(uint16 us) {
  while(us--) {
    for (volatile uint16 i = 0; i < 4; i++);  // Tùy MCU, di?u ch?nh l?p d? ~1µs
  }
}

void delay_ms(uint16 ms) {
  while(ms--) {
    delay_us(1000);
  }
}
