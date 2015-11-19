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
#include <time.h>

#define MAX(x, y) (((x) > (y)) ? (x) : (y))

#define NO_UNUSED_FRAMES -1
#define CUST_RAND_PCT 30

struct disk *disk;
char *physmem;

// Page replacement algorithm handler
page_fault_handler_t page_algo_handler;

// Frame table
int *ft;

// Frame index
int fi;

// Stats
int disk_reads;
int disk_writes;
int page_faults;

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
	int i;
	
	for (i = 0; i < page_table_get_nframes(pt); ++i)
		if (ft[i] == -1) return i;

	return NO_UNUSED_FRAMES;
}

void swap_page(struct page_table *pt, int in_page, int frame) {
	if (ft[frame] != -1) {
		int bits, out_page = ft[frame];
		page_table_get_entry(pt, out_page, &frame, &bits);

		if(has_write(bits)) {
			// Memory has been modified - write to disk
			disk_write(disk, out_page, &physmem[frame * PAGE_SIZE]);
			disk_writes++;
		}
	
		page_table_set_entry(pt, out_page, 0, 0);
	}
	
	disk_read(disk, in_page, &physmem[frame * PAGE_SIZE]);
	page_table_set_entry(pt, in_page, frame, PROT_READ);
	disk_reads++;

	// Update frame table
	ft[frame] = in_page;
}

void rand_handler(struct page_table *pt, int page) {
	// Get page of random frame from ft
	int frame = lrand48() % page_table_get_nframes(pt);
	swap_page(pt, page, frame);
}

void fifo_handler(struct page_table *pt, int page) {
	swap_page(pt, page, fi++ % page_table_get_nframes(pt));
}

void custom_handler(struct page_table *pt, int page) {
	int i, frame, bits,
		nframes = page_table_get_nframes(pt),
		rand_count = MAX(1, nframes * CUST_RAND_PCT / 100),
		rand_frames[rand_count];

	for (i = 0; i < rand_count; ++i) {
		rand_frames[i] = lrand48() % nframes;
	}

	for (i = 0; i < rand_count; ++i) {
		page_table_get_entry(pt, ft[rand_frames[i]], &frame, &bits);
		if (!has_write(bits)) {
			swap_page(pt, page, frame);
			return;
		}
	}

	swap_page(pt, page, frame);
}

void page_fault_handler( struct page_table *pt, int page )
{
	page_faults++;

	int frame, bits;
	page_table_get_entry(pt, page, &frame, &bits);

	if(has_read(bits)) {
		page_table_set_entry(pt, page, frame, bits|PROT_WRITE);
		return;
	}

	int unused_frame = next_unused_frame(pt);
	if (unused_frame != NO_UNUSED_FRAMES) {
		swap_page(pt, page, unused_frame);
	} else {
		page_algo_handler(pt, page);
	}
}

int *run(int npages, int nframes, page_fault_handler_t handler, void (*program)(char *, int)) {
	int i;
	page_algo_handler = handler;

	disk = disk_open("myvirtualdisk",npages);
	if(!disk) {
		fprintf(stderr,"couldn't create virtual disk: %s\n",strerror(errno));
		exit(EXIT_FAILURE);
	}

	struct page_table *pt = page_table_create( npages, nframes, page_fault_handler );
	if(!pt) {
		fprintf(stderr,"couldn't create page table: %s\n",strerror(errno));
		exit(EXIT_FAILURE);
	}

	fi = 0;
	disk_reads = 0;
	disk_writes = 0;
	page_faults = 0;

	// Init frame table
	ft = malloc(nframes * sizeof(int));
	for (i = 0; i < nframes; ++i)
	{
		ft[i] = -1;
	}

	char *virtmem = page_table_get_virtmem(pt);
	physmem = page_table_get_physmem(pt);

	program(virtmem,npages*PAGE_SIZE);

	printf("Disk writes: %d\n", disk_writes);
	printf("Disk reads:  %d\n", disk_reads);
	printf("Page faults: %d\n", page_faults);

	free(ft);
	page_table_delete(pt);
	disk_close(disk);

	int *stats = malloc(sizeof(int) * 3);
	stats[0] = disk_reads;
	stats[1] = disk_writes;
	stats[2] = page_faults;

	return stats;
}

void test() {
	int pages = 100, frames = 10;

	// fifo
	printf("FIFO FOCUS:\n");
	free(run(pages, frames, fifo_handler, focus_program));
	printf("-----------------------\n");
	printf("FIFO SORT:\n");
	free(run(pages, frames, fifo_handler, sort_program));
	printf("-----------------------\n");
	printf("FIFO SCAN:\n");
	free(run(pages, frames, fifo_handler, scan_program));
	printf("-----------------------\n");

	// rand
	printf("RAND FOCUS:\n");
	free(run(pages, frames, rand_handler, focus_program));
	printf("-----------------------\n");
	printf("RAND SORT:\n");
	free(run(pages, frames, rand_handler, sort_program));
	printf("-----------------------\n");
	printf("RAND SCAN:\n");
	free(run(pages, frames, rand_handler, scan_program));
	printf("-----------------------\n");

	// custom
	printf("CUSTOM FOCUS:\n");
	free(run(pages, frames, custom_handler, focus_program));
	printf("-----------------------\n");
	printf("CUSTOM SORT:\n");
	free(run(pages, frames, custom_handler, sort_program));
	printf("-----------------------\n");
	printf("CUSTOM SCAN:\n");
	free(run(pages, frames, custom_handler, scan_program));
	printf("-----------------------\n");
}

int main( int argc, char *argv[] )
{
	if(argc == 2 && !strcmp(argv[1], "test")) {
		test();
		exit(EXIT_SUCCESS);
	}

	if(argc!=5) {
		printf("use: virtmem <npages> <nframes> <rand|fifo|custom> <sort|scan|focus>\n");
		exit(EXIT_FAILURE);
	}

	int npages = atoi(argv[1]);
	int nframes = atoi(argv[2]);

	const char *handler_str = argv[3];
	const char *program_str = argv[4];

	page_fault_handler_t handler;
	void (*program)(char *, int);

	if(!strcmp(handler_str,"rand")) {
		handler = rand_handler;
	} else if(!strcmp(handler_str,"fifo")) {
		handler = fifo_handler;
	} else if(!strcmp(handler_str,"custom")) {
		handler = custom_handler;
	} else {
		fprintf(stderr,"unknown page replacement algorithm: %s\n", handler_str);
		exit(EXIT_FAILURE);
	}

	if(!strcmp(program_str,"sort")) {
		program = sort_program;
	} else if(!strcmp(program_str,"scan")) {
		program = scan_program;
	} else if(!strcmp(program_str,"focus")) {
		program = focus_program;
	} else {
		fprintf(stderr,"unknown program: %s\n", program_str);
	}

	run(npages, nframes, handler, program);
	exit(EXIT_SUCCESS);
}