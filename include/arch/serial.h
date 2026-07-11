#ifndef OUROBOROS_ARCH_SERIAL_H
#define OUROBOROS_ARCH_SERIAL_H
#include <stdbool.h>
#include <libk/typedef.h>
#include <arch/serial_typedefs.h>

err_t init_serial(serial_port_t port);
bool serial_received(serial_port_t port);
char read_serial(serial_port_t port);
bool is_transmit_empty(serial_port_t port);
void write_serial(serial_port_t port, uint8_t a);

#endif
