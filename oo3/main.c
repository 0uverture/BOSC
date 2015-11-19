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

#define NO_UNUSED_FRAMES -1
#define CLEAR_INTERVAL 5
#define CUST_RAND_COUNT 10

struct disk *disk;
char *physmem;

// Page replacement algorithm handler
page_fault_handler_t page_algo_handler;

// Frame table
int *ft;

// Frame index
int fi = 0;

// Stats
int disk_reads = 0;
int disk_writes = 0;
int page_faults = 0;

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
		printf("swapping frame #%d of page #%d with page #%d\n", frame, out_page, in_page);
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
	int i, frame, bits, nframes = page_table_get_nframes(pt);
	int rand_frames[CUST_RAND_COUNT];

	for (i = 0; i < CUST_RAND_COUNT; ++i) {
		rand_frames[i] = lrand48() % nframes;
	}

	for (i = 0; i < CUST_RAND_COUNT; ++i) {
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
	printf("page fault on page #%d\n", page);

	int frame, bits;
	page_table_get_entry(pt, page, &frame, &bits);

	if(has_read(bits)) {
		printf("page #%d was written to\n", page);
		page_table_set_entry(pt, page, frame, bits|PROT_WRITE);

		// DEBUG
		// page_table_print(pt);
		// sleep(1);

		return;
	}

	int unused_frame = next_unused_frame(pt);
	if (unused_frame != NO_UNUSED_FRAMES) {
		printf("frame #%d is unused\n", unused_frame);
		swap_page(pt, page, unused_frame);
	} else {
		page_algo_handler(pt, page);
	}

	// DEBUG
	// page_table_print(pt);
	// sleep(1);
}

int main( int argc, char *argv[] )
{
	int i;
	if(argc == 1 && !strcmp(argv[1], "test")) {
		char *args[5];

		args[0] = NULL;
		args[1] =	"100";
		args[2] =	"10";
		args[3] =	"custom";
		args[4] =	"scan";

		main(5, args);

		exit(EXIT_SUCCESS);
	}

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
	} else if(!strcmp(page_algo,"fifo")) {
		page_algo_handler = fifo_handler;
	} else if(!strcmp(page_algo,"custom")) {
		page_algo_handler = custom_handler;
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
	for (i = 0; i < nframes; ++i)
	{
		ft[i] = -1;
	}

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

	printf("Disk writes: %d\n", disk_writes);
	printf("Disk reads:  %d\n", disk_reads);
	printf("Page faults: %d\n", page_faults);

	free(ft);
	page_table_delete(pt);
	disk_close(disk);

	return 0;
}