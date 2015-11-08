/*
Main program for the virtual memory project.
Make all of your modifications to this file.
You may add or rearrange any code or data as you need.
The header files page_table.h and disk.h explain
how to use the page table and disk interfaces.
*/

#include "page_table.h"
#include "disk.h"
#include "program.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <unistd.h>

#define NO_UNUSED_FRAMES -1

page_fault_handler_t page_algo_handler;
struct disk *disk;
char *physmem;
int *ft;

/**
 * Evaluates if bits contains the PROT_READ flag.
 */
bool has_read(int bits) {
	return (bits & PROT_READ) == PROT_READ;
}

/**
 * Evaluates if bits contains the PROT_WRITE flag.
 */
bool has_write(int bits) {
	return (bits & PROT_WRITE) == PROT_WRITE;
}

/**
 * Returns the next unused frame, based on the PROT_READ flag,
 * or NO_UNUSED_FRAMES if none are available.
 */
int next_unused_frame(struct page_table *pt) {
	int frame, bits, i,
			npages = page_table_get_npages(pt),
			nframes = page_table_get_nframes(pt);

	bool unused[nframes];
	for (i = 0; i < nframes; ++i)
		unused[i] = true;

	for (i = 0; i < npages; ++i)
	{
		page_table_get_entry(pt, i, &frame, &bits);
		unused[frame] &= !has_read(bits);
		// LOGIC AND is important, as multiple pages may point to the same frame
	}

	for (i = 0; i < nframes; ++i)
		if (unused[i]) return i;

	return NO_UNUSED_FRAMES;
}

void swap_page(struct page_table *pt, int out_page, int in_page) {
	printf("swapping frame of page #%d with page #%d\n", out_page, in_page);

	int frame, bits;
	page_table_get_entry(pt, out_page, &frame, &bits);

	if(has_write(bits)) {
		// Memory has been modified - write to disk
		disk_write(disk, out_page, &physmem[frame * PAGE_SIZE]);
		disk_read(disk, in_page, &physmem[frame * PAGE_SIZE]);
	}

	// Update page table
	page_table_set_entry(pt, out_page, 0, 0);
	page_table_set_entry(pt, in_page, frame, PROT_READ);

	// Update frame table
	ft[frame] = in_page;
}

void rand_handler(struct page_table *pt, int page) {
	// Get page of random frame from ft
	int frame = rand() % page_table_get_nframes(pt);
	int out_page = ft[frame];
	swap_page(pt, out_page, page);
}

void page_fault_handler( struct page_table *pt, int page )
{
	printf("page fault on page #%d\n", page);

	int frame, bits;
	page_table_get_entry(pt, page, &frame, &bits);

	if(has_read(bits)) {
		printf("page #%d was written to\n", page);
		page_table_set_entry(pt, page, frame, bits|PROT_WRITE);

		// DEBUG
		page_table_print(pt);
		sleep(1);

		return;
	}

	int unused_frame = next_unused_frame(pt);
	if (unused_frame != NO_UNUSED_FRAMES) {
		printf("frame #%d is unused\n", unused_frame);
		page_table_set_entry(pt, page, unused_frame, PROT_READ);
		disk_read(disk, page, &physmem[unused_frame * PAGE_SIZE]);
		ft[unused_frame] = page;
	} else {
		page_algo_handler(pt, page);
	}

	// DEBUG
	page_table_print(pt);
	sleep(1);
}

int main( int argc, char *argv[] )
{
	if(argc!=5) {
		printf("use: virtmem <npages> <nframes> <rand|fifo|custom> <sort|scan|focus>\n");
		return 1;
	}

	int npages = atoi(argv[1]);
	int nframes = atoi(argv[2]);
	const char *page_algo = argv[3];
	const char *program = argv[4];

	if(!strcmp(page_algo,"rand")) {
		page_algo_handler = rand_handler;
	} else {
		fprintf(stderr,"unknown page replacement algorithm: %s\n", page_algo);
		exit(EXIT_FAILURE);
	}

	disk = disk_open("myvirtualdisk",npages);
	if(!disk) {
		fprintf(stderr,"couldn't create virtual disk: %s\n",strerror(errno));
		return 1;
	}

	struct page_table *pt = page_table_create( npages, nframes, page_fault_handler );
	if(!pt) {
		fprintf(stderr,"couldn't create page table: %s\n",strerror(errno));
		return 1;
	}

	ft = malloc(nframes * sizeof(int));
	int i;
	for (i = 0; i < nframes; ++i)
	{
		ft[i] = 0;
	}

	srand(62087);

	char *virtmem = page_table_get_virtmem(pt);

	physmem = page_table_get_physmem(pt);

	if(!strcmp(program,"sort")) {
		sort_program(virtmem,npages*PAGE_SIZE);

	} else if(!strcmp(program,"scan")) {
		scan_program(virtmem,npages*PAGE_SIZE);

	} else if(!strcmp(program,"focus")) {
		focus_program(virtmem,npages*PAGE_SIZE);

	} else {
		fprintf(stderr,"unknown program: %s\n", program);

	}

	free(ft);
	page_table_delete(pt);
	disk_close(disk);

	return 0;
}
