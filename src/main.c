#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <libk/assert.h>
#include <libk/stdio.h>
#include <libk/string.h>
#include <libk/typedef.h>
#include <libk/util.h>

#include <output/debug.h>

#include <arch/init.h>
#include <arch/spinlock.h>

#include <pmm/pmm.h>

#include <limine.h>

// clang-format off
//
// Set the base revision to 4, this is recommended as this is the latest
// base revision described by the Limine boot protocol specification.
// See specification for further info.
__attribute__((used, section(".limine_requests")))
static volatile uint64_t limine_base_revision[] = LIMINE_BASE_REVISION(4);

// The Limine requests can be placed anywhere, but it is important that
// the compiler does not optimise them away, so, usually, they should
// be made volatile or equivalent, _and_ they should be accessed at least
// once or marked as used with the "used" attribute as done here.

// Finally, define the start and end markers for the Limine requests.
// These can also be moved anywhere, to any .c file, as seen fit.
__attribute__((used, section(".limine_requests_start")))
static volatile uint64_t limine_requests_start_marker[] = LIMINE_REQUESTS_START_MARKER;
__attribute__((used, section(".limine_requests_end")))
static volatile uint64_t limine_requests_end_marker[] = LIMINE_REQUESTS_END_MARKER;

// clang-format on

#include <libk/container_of.h>
#include <libk/data_structures/red_black_tree.h>

#define ARR_CAP 1024
struct test {
	struct rb_node rb_node;
	int key;
} arr[ARR_CAP];

int comp_test(struct rb_node *node_a, struct rb_node *node_b) {
	if (node_a == NULL && node_b) {
		return 0;
	}
	if (node_b == NULL) {
		return 1;
	}
	if (node_a == NULL) {
		return -1;
	}
	struct test *a = container_of(node_a, struct test, rb_node);
	struct test *b = container_of(node_b, struct test, rb_node);
	return a->key - b->key;
}
void print_rb_node_test(struct rb_node *node) {
	if (node == NULL) {
		printf_limited("{}");
		return;
	}
	struct test *a = container_of(node, struct test, rb_node);
	printf_limited("{\"key\": %d,\"C\": '%c', \"L\": ", a->key, node->color == RB_BLACK ? 'b' : 'r');
	print_rb_node_test(node->left);
	printf_limited(", \"R\": ");
	print_rb_node_test(node->right);
	printf_limited("}");
}
// for testing
uint64_t rand_next(uint64_t *seed) {
	uint64_t z = (*seed += 0x9E3779B97F4A7C15ULL);
	z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
	z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
	return z ^ (z >> 31);
}
#define NUM_TESTS 100
#define KEY_RANGE 10000
void kmain(void) {
	// Ensure the bootloader actually understands our base revision (see spec).
	if (LIMINE_BASE_REVISION_SUPPORTED(limine_base_revision) == false) {
		hcf();
	}

	if (!IS_SUCCESS(init_debug_output())) {
		hcf();
	}
	init_boot_cpu();

	uint64_t seed = 0;
	struct rb_tree tree = RB_TREE_INIT;
	
	int used_space = 0;
	for (int i=0;i<NUM_TESTS;i++) {
		int num_insert = rand_next(&seed) % (ARR_CAP-used_space);
		for (int j=0;j<num_insert;j++) {
			arr[used_space+j].key = (rand_next(&seed) % (KEY_RANGE))-KEY_RANGE/2;
			if (! IS_SUCCESS(rb_insert(&arr[used_space+j].rb_node, &tree, comp_test))) {
				used_space--;
			}
		}
		
		printf_limited("post insert: \n");
		print_rb_node_test(tree.root);
		printf_limited("\n");
		used_space += num_insert;
		int num_delete = rand_next(&seed) % (used_space);
		for (int j=1;j<=num_delete;j++) {
			rb_delete(&arr[used_space-j].rb_node, &tree);
		}
		used_space -= num_delete;
		printf_limited("post delete: \n");
		print_rb_node_test(tree.root);
		printf_limited("\n");
	}
	// We're done, just hang...
	hcf();
}
