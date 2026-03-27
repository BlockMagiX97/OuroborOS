#include <limits.h>
#include <stddef.h>
#include <stdint.h>

#include <arch/x86_64/gdt.h>
#include <arch/x86_64/mem_addr.h>

segment_selector_t kernel_code_segment_sel=0;
segment_selector_t kernel_data_segment_sel=0;
struct global_descriptor_table_entry gdt[MAX_NUM_GDT_ENTRIES];

// returns -1 on encoding error, 0 on success
int gdt_set_gate(size_t index, uint64_t base, uint32_t limit, uint8_t access, uint8_t flags) {
	if (base > UINT32_MAX) {
		return -1;
	}
	gdt[index].base_low = base & 0xffff;
	gdt[index].base_mid = (base >> 16) & 0xff;
	gdt[index].base_high = (base >> (16 + 8)) & 0xff;
	gdt[index].limit_low = limit & 0xffff;
	gdt[index].access = access;
	gdt[index].flags = (limit >> 16) & ((1 << 4) - 1);
	gdt[index].flags |= (flags << 4);
	return 0;
}
struct {
	uint16_t limit;
	uint64_t base;
} __attribute__((packed)) gdtptr;

#define GDT_A_PRESENT (1 << 7)
#define GDT_A_DLP(ring) ((ring & 0b11) << 5)
#define GDT_A_NOT_SYSTEM (1 << 4)
#define GDT_A_EXECUTABLE (1 << 3)
#define GDT_A_DIRECTION_CONFORMING (1 << 2)
#define GDT_A_READABLE_WRITABLE (1 << 1)
#define GDT_A_ACCESSED (1)

#define GDT_F_GRANULARITY (1<<3)
#define GDT_F_DB (1<<2)
#define GDT_F_LONG (1<<1)


extern void load_gdt(void *gdtptr);

void init_gdt() {
	// null descriptor: segment selector 0
	gdt_set_gate(0, 0, 0, 0, 0);

	kernel_code_segment_sel = 8;
	// 64 bit, ring 0, code segment writable: segment selector 8=0x8
	gdt_set_gate(kernel_code_segment_sel/8, 0, 0xffffffff,
		     GDT_A_PRESENT | GDT_A_DLP(0) | GDT_A_NOT_SYSTEM | GDT_A_EXECUTABLE | GDT_A_READABLE_WRITABLE | GDT_A_ACCESSED,
		     GDT_F_GRANULARITY | GDT_F_LONG);
	kernel_data_segment_sel = 0x10;
	// 64 bit, ring 0, data segment writable: segment selector 16=0x10
	gdt_set_gate(kernel_data_segment_sel/8, 0, 0xffffffff, GDT_A_PRESENT | GDT_A_DLP(0) | GDT_A_NOT_SYSTEM  | GDT_A_READABLE_WRITABLE | GDT_A_ACCESSED,
		     GDT_F_GRANULARITY | GDT_F_LONG);
	gdtptr.base = ((vaddr_t)&gdt);
	gdtptr.limit = (sizeof(gdt) - 1);
	// hope for the best
	load_gdt(&gdtptr);
}
void init_shared_gdt() {
	load_gdt(&gdtptr);
};
