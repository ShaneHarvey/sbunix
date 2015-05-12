#ifndef _SBUNIX_SERIAL_H
#define _SBUNIX_SERIAL_H

#define PORT 0x3f8   /* COM1 */

void serial_init(void);

void serial_write(uint8_t a);
uint8_t serial_read(void);

#endif //_SBUNIX_SERIAL_H
