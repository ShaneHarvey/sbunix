#ifndef _SBUNIX_INTERRUPT_PIT_H
#define _SBUNIX_INTERRUPT_PIT_H

#include <sys/types.h>

extern uint64_t system_time;
extern uint32_t time_counter;
extern uint32_t time_reset;

void pit_set_freq(unsigned int hz);

#endif
