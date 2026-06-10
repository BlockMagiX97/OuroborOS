#include <arch/curr/serial.h>
#include <arch/curr/spinlock.h>
#include <libk/typedef.h>
#include <stddef.h>

static spinlock_t serial_write = SPINLOCK_INIT_LOCKED;

#define DEBUG_COM_PORT PORT_COM1

err_t init_debug_output() {
	if(! IS_SUCCESS(init_serial(DEBUG_COM_PORT))) {
		return GENERIC_FAIL;
	};
	spinlock_release(&serial_write, NULL);
	return SUCCESS;
}
// use only when outputing only one char (locks and sends one and then unlocks)
void send_debug_output_char(char ch) {
	flags_t flags;
	spinlock_acquire(&serial_write, &flags);
	write_serial(DEBUG_COM_PORT, (uint8_t)ch);
	spinlock_release(&serial_write, &flags);
}

// prevent fragmenting msg when multiprocessing (locks before msg and unlocks after)
void send_debug_output(const char *str) {
	flags_t flags;
	spinlock_acquire(&serial_write, &flags);
	while (*str) {
		write_serial(DEBUG_COM_PORT, (uint8_t)*str);
		str++;
	}
	spinlock_release(&serial_write, &flags);
}

// used only with send_debug_output_no_lock, when code needs to print ultra long lines and needs to not be interupted in readable way
void lock_debug_output(flags_t *flags) {
	spinlock_acquire(&serial_write, flags);
}
// used only with send_debug_output_no_lock, when code needs to print ultra long lines and needs to not be interupted in readable way
void send_debug_output_no_lock(const char *str) {
	while (*str) {
		write_serial(DEBUG_COM_PORT, (uint8_t)*str);
		str++;
	}
}
// used only with send_debug_output_no_lock, when code needs to print ultra long lines and needs to not be interupted in readable way
void unlock_debug_output(flags_t *flags) {
	spinlock_release(&serial_write, flags);
}
