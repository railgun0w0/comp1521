// COMP1521 19t2 ... Assignment 2: heap management system

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "myHeap.h"

/** minimum total space for heap */
#define MIN_HEAP 4096
/** minimum amount of space for a free Chunk (excludes Header) */
#define MIN_CHUNK 32


#define ALLOC 0x55555555
#define FREE  0xAAAAAAAA

/// Types:

typedef unsigned int  uint;
typedef unsigned char byte;

typedef uintptr_t     addr; // an address as a numeric type

/** The header for a chunk. */
typedef struct header {
	uint status;    /**< the chunk's status -- ALLOC or FREE */
	uint size;      /**< number of bytes, including header */
	byte data[];    /**< the chunk's data -- not interesting to us */
} header;

/** The heap's state */
struct heap {
	void  *heapMem;     /**< space allocated for Heap */
	uint   heapSize;    /**< number of bytes in heapMem */
	void **freeList;    /**< array of pointers to free chunks */
	uint   freeElems;   /**< number of elements in freeList[] */
	uint   nFree;       /**< number of free chunks */
};


/// Variables:

/** The heap proper. */
static struct heap Heap;


/// Functions:

static addr heapMaxAddr (void);


/** Initialise the Heap. */
int initHeap (int size)
{
	Heap.nFree = 0;
	Heap.freeElems = 0;

	/// TODO ///
	// adjust N so (N%4==0) and at least MIN_HEAP
	if(size < MIN_HEAP) size = MIN_HEAP;
	while(size % 4 != 0) {
		size++;
	}
	
	// allocate a region of memory with N bytes  (heapMem) and set heapMem to be all zeroes
	// set heapMem to be a single free chunk of size N
	header *newchunkheader = calloc(size,1);
	if(newchunkheader == NULL) return -1;
	newchunkheader->status = FREE;
	newchunkheader->size = size;
	Heap.heapMem = newchunkheader;
	Heap.heapSize = size;
	Heap.freeElems = size/MIN_CHUNK;
	// allocate an array of N/MIN_CHUNK pointers  (freeList)
	Heap.freeList = calloc(Heap.freeElems,sizeof(Heap.freeList));
	if(Heap.freeList == NULL) return -1;
	Heap.nFree = 1;
	Heap.freeList[0] = newchunkheader;

	return 0; // this just keeps the compiler quiet
}

/** Release resources associated with the heap. */
void freeHeap (void)
{
	free (Heap.heapMem);
	free (Heap.freeList);
}

/** Allocate a chunk of memory large enough to store `size' bytes. */
void *myMalloc (int size)
{
	/// TODO ///
	
	if (size < 1) return NULL;
	while(size % 4 != 0) {
		size++;
	}
	int normalsize = size + sizeof(header);
	int diffsize =  size + sizeof(header) + MIN_CHUNK;
	int i = 0;
	int found = 0;
	int minsize = 0xffffffff;
	int foundfreesize = 0; 
	while(i < Heap.nFree) {
		header *curr =(header *) Heap.freeList[i];
		if(normalsize <= curr->size && curr->size < minsize) {
			found = 1;
			minsize = curr->size;
			foundfreesize = i;	
		}
		i++;
	}
	if(found == 0) {
		// Not found free chunk larger than N + HeaderSize
		printf("cant found free chunk\n");
		return NULL;
	}
	if(found == 1) {
		// found free chunk larger than N + HeaderSize
		// check whether smaller than N + HeaderSize + MIN_CHUNK
		header *curr = (header *) Heap.freeList[foundfreesize];
		if(curr->size < diffsize) {
			//allocate the whole chunk
			curr->status = ALLOC;
			// move forward the free list
			int changenum = foundfreesize;
			while(changenum < Heap.nFree-1) {
				Heap.freeList[changenum] = Heap.freeList[changenum+1];
				changenum++;
			}
			Heap.freeList[changenum] = NULL;
			Heap.nFree = Heap.nFree - 1;
			return curr->data;
		}else{
			// split it into two chunks
			header *lowerchunk = (header *)Heap.freeList[foundfreesize];
			lowerchunk->status = ALLOC;
			int newsize = lowerchunk->size - normalsize;
			lowerchunk->size = normalsize;
			addr addbigchunk = (addr)lowerchunk + lowerchunk->size;
			header *biggerchunk = (header *)addbigchunk;
			biggerchunk->size = newsize;
			biggerchunk->status = FREE;
			Heap.freeList[foundfreesize] = biggerchunk;
			return lowerchunk->data;
		}
	}
	return NULL; // this just keeps the compiler quiet
}

/** Deallocate a chunk of memory. */
void myFree (void *obj)
{
	/// TODO ///
	//	check obj whether represent an allocated chunk in the heap and
	// 	an address somewhere in the middle of an allocated block
	addr headeradd = (addr)obj - sizeof(header);
	header *freechunk = (header *)headeradd;
	if(freechunk->status != ALLOC) {
		printf("Attempt to free unallocated chunk\n");
		exit(1);
	}
	// turns allocated chunk into free chunk (and zeroes it out)
	freechunk->status = FREE;
	memset(obj,0,(freechunk->size-sizeof(header)));
	// add it  to freelist
	int x = 0;
	addr freelistx =(addr)Heap.freeList[x];
	while(freelistx < headeradd && x < Heap.nFree) {
		x++;
		freelistx = (addr)Heap.freeList[x];
	}
	int poistion = x;
	int y = Heap.nFree;
	while(x < y) {
		Heap.freeList[y] = Heap.freeList[y - 1];
		y--;
	}
	Heap.freeList[poistion] = freechunk;
	Heap.nFree++;
	Heap.freeList[Heap.nFree] = NULL;
	 
	// check next chunk is free or alloc

	addr nextheadadd = (addr)obj+ freechunk->size - sizeof(header);  
	header *nextchunk = (header *)nextheadadd;
	if(nextchunk->status == FREE) {
		freechunk->size = freechunk->size + nextchunk->size;
		// find next chunk in which freelist
		int i = 0; 
		while(Heap.freeList[i] != nextchunk) {
			i++;
		}
		int findnext = i;
		while(findnext < Heap.nFree-1) {
			Heap.freeList[findnext] = Heap.freeList[findnext+1];
			findnext++;
		}
		Heap.freeList[i - 1] = freechunk;
		Heap.nFree--;
		Heap.freeList[Heap.nFree] = NULL;
	}
	
	// check prev chunk
	// find prev chunk
	int j = 0;
	while(Heap.freeList[j] != freechunk){
		j++;
	}
	if(j != 0) {
	    header *prevchunk = (header *)Heap.freeList[j-1];
	    if(prevchunk->status == FREE) {
		    // check whether connect with this 
		    addr checkchunk = (addr)prevchunk + prevchunk->size;
		    header *checknextchunk = (header *)checkchunk;
		    if(checknextchunk == freechunk) {
			    prevchunk->size = prevchunk->size + freechunk->size;
			    Heap.freeList[j-1] = prevchunk;
			    while(j < Heap.nFree - 1) {
				    Heap.freeList[j] = Heap.freeList[j+1];
				    j++;
			    }
			    Heap.freeList[j] = NULL;
			    Heap.nFree--; 
		    }
	    }
    }
}

/** Return the first address beyond the range of the heap. */
static addr heapMaxAddr (void)
{
	return (addr) Heap.heapMem + Heap.heapSize;
}

/** Convert a pointer to an offset in the heap. */
int heapOffset (void *obj)
{
	addr objAddr = (addr) obj;
	addr heapMin = (addr) Heap.heapMem;
	addr heapMax =        heapMaxAddr ();
	if (obj == NULL || !(heapMin <= objAddr && objAddr < heapMax))
		return -1;
	else
		return (int) (objAddr - heapMin);
}

/** Dump the contents of the heap (for testing/debugging). */
void dumpHeap (void)
{
	int onRow = 0;

	// We iterate over the heap, chunk by chunk; we assume that the
	// first chunk is at the first location in the heap, and move along
	// by the size the chunk claims to be.
	addr curr = (addr) Heap.heapMem;
	while (curr < heapMaxAddr ()) {
		header *chunk = (header *) curr;

		char stat;
		switch (chunk->status) {
		case FREE:  stat = 'F'; break;
		case ALLOC: stat = 'A'; break;
		default:
			fprintf (
				stderr,
				"myHeap: corrupted heap: chunk status %08x\n",
				chunk->status
			);
			exit (1);
		}

		printf (
			"+%05d (%c,%5d)%c",
			heapOffset ((void *) curr),
			stat, chunk->size,
			(++onRow % 5 == 0) ? '\n' : ' '
		);

		curr += chunk->size;
	}

	if (onRow % 5 > 0)
		printf ("\n");
}
