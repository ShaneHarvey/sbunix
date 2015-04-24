#ifndef _SBUNIX_INTERRUPT_PIT_H
#define _SBUNIX_INTERRUPT_PIT_H

#include <sys/types.h>
#include <sbunix/time.h>

#define RTC_PORT_REG   0x70
#define RTC_PORT_DATA  0x71
#define RTC_REG_A      0x0A

#define RTC_REG_B      0x0B
#define RTC_BINARY     0x04     /* all time/date values are in BCD if not set */
#define BCD_TO_BIN(val) ((uint8_t)(((val)&15) + ((val)>>4)*10))

/* RTC Data registers */
#define RTC_SECONDS    0x00
#define RTC_MINUTES    0x02
#define RTC_HOURS      0x04
#define RTC_WEEKDAY    0x06 /* unreliable */
#define RTC_DAY        0x07
#define RTC_MONTH      0x08
#define RTC_YEAR       0x09

extern volatile uint64_t system_time; /* number of seconds since boot */

void timer_sleep(int seconds);
void pit_set_freq(unsigned int hz);

#endif
