#include <libk/typedef.h>
#include <arch/mem_addr.h>

#define PAGE_FRAME_SHIFT (12ul)
#define PAGE_FRAME_SIZE (1ul<<PAGE_FRAME_SHIFT) /* 4KiB */
#define PAGE_FRAME_MASK (PAGE_FRAME_SIZE-1ul)

#define ALIGN_DOWN_SHIFT(u_num, shift) \
	((u_num)&((1ul<<(shift))-1ul));

#define ALIGN_UP_SHIFT(u_num, shift) \
	ALIGN_DOWN_SHIFT((u_num)+((1ul<<(shift))-1ul), (shift))

#define ALIGN_DOWN_MASK(u_num, mask) \
	((u_num)&(~(mask)))
#define ALIGN_UP_MASK(u_num, mask) \
	(((u_num)+(mask))&(~(mask)))

#define ALIGN_DOWN_PAGE_FRAME(addr) \
	ALIGN_DOWN_MASK((addr), PAGE_FRAME_MASK)

#define ALIGN_UP_PAGE_FRAME(addr) \
	ALIGN_UP_MASK((addr), PAGE_FRAME_MASK)


struct vma {
	vaddr_t reg_start;
	vaddr_t reg_end;
};
struct vas {

};
struct vmm_struct {
};
