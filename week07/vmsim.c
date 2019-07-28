// COMP1521 19t2 ... virtual memory simulator

#include <assert.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <sysexits.h>

#define PAGESIZE 4096
#define PAGEBITS 12

#define actionName(A) (((A) == 'R') ? "read from" : "write to")

typedef unsigned int uint;

// Page Table Entries
typedef struct PTE {
	struct {
		uint loaded   :1;
		uint modified :1;
	} status;
	int frameNo;      // -1 if page not loaded
	int lastAccessed; // -1 if never accessed
} PTE;


// Global state:
static PTE *PageTable; // process page table
static int *MemFrames; // memory (each frame holds page #, or -1 if empty)
static uint
	nPages,            // how many process pages
	nFrames,           // how many memory frames
	nLoads = 0,        // how many page loads
	nSaves = 0,        // how many page writes (after modification)
	nReplaces = 0,     // how many Page replacements
	clock = 0;         // clock ticks


// Functions:
int physicalAddress (uint vAddr, char action);
void initPageTable (void);
void initMemFrames (void);
void showState (void);


// main:
// read memory references
// maintain VM data structures
// argv[1] = nPages, argv[2] = nFrames
// stdin contains lines of form
//   R Address
//   W Address
// R = read a byte, W = write a byte, Address = byte address
// Address is mapped to a page reference as per examples in lectures
// Note: pAddr is signed, because -ve used for errors
int main (int argc, char *argv[])
{
	setbuf (stdout, NULL);

	if (argc < 3)
		errx (EX_USAGE, "usage: %s <n-pages> <n-frames>", argv[0]);

	// read command-line arguments
	if ((nPages = strtol (argv[1], NULL, 10)) < 1)
		errx (EX_USAGE, "invalid n-pages '%s'", argv[1]);
	if ((nFrames = strtol (argv[2], NULL, 10)) < 1)
		errx (EX_USAGE, "invalid n-frames '%s'", argv[2]);

	initPageTable ();
	initMemFrames ();

	char line[BUFSIZ]; // line buffer
	while (fgets (line, BUFSIZ, stdin) != NULL) {
		// get next line; check valid (barely)
		char action;
		uint vAddr;
		if (! ((sscanf (line, "%c %d\n", &action, &vAddr) == 2) &&
			   (action == 'R' || action == 'W'))) {
			warnx ("invalid input '%s', ignoring...", line);
			continue;
		}

		// do address mapping
		int pAddr = physicalAddress (vAddr, action);
		if (pAddr < 0)
			errx (EX_SOFTWARE, "invalid address %d", vAddr);

		// debugging ...
		printf (
			"\n@ t=%d, %s pA=%d (vA=%d)\n",
			clock, actionName (action), pAddr, vAddr
		);

		// tick clock and show state
		showState ();
		clock++;
	}

	printf (
		"\n#loads = %d, #saves = %d, #replacements = %d\n",
		nLoads, nSaves, nReplaces
	);

	return EXIT_SUCCESS;
}


// map virtual address to physical address
// handles regular references, page faults and invalid addresses
int physicalAddress (uint vAddr, char action)
{	
	int physicaladdress;
	int pagenum = vAddr/PAGESIZE;  
	int offset 	= vAddr%PAGESIZE;  // extract page# and offset from vAddr
	if(pagenum >= nPages) {
		return -1;
	}
	if(PageTable[pagenum].frameNo !=  -1) {
		// set the Modified flag if action is a write
		if(action == 'W') {
			PageTable[pagenum].status.modified = 1;
		}
		// update the access time to the current clock tick
		PageTable[pagenum].lastAccessed = clock;
		// use the frame number and offset to compute a physical address
		physicaladdress = (PageTable[pagenum].frameNo * PAGESIZE) + offset;
		
	} else {
		// otherwise:
		// look for an unused frame
		for(int i = 0; i < nFrames; i++) {
			if(MemFrames[i] == -1) {
				// increment the nLoads counter
				nLoads++;
				// set PageTable entry for the new page
				MemFrames[i] = pagenum;
				// set PageTable entry for the new page
				//  (flags, frame#, accesstime=current clock tick), and
				PageTable[pagenum].status.loaded = 1;
				if(action == 'W') {
					PageTable[pagenum].status.modified = 1;
				}
				PageTable[pagenum].lastAccessed = clock;	//accesstime=current clock tick
				PageTable[pagenum].frameNo = i;				//frame#
				// use the frame number and offset to compute a physical address
				physicaladdress = (PageTable[pagenum].frameNo * PAGESIZE) + offset;
				//  return the physical address
				return physicaladdress;
			}
		} 
		// otherwise:
		// we need to replace a currently loaded frame, so
		// find the Least Recently Used loaded page,
		int least = clock;
		int find = 0;
		for(int j = 0;j < nPages; j++) {
			if(PageTable[j].lastAccessed < least && PageTable[j].lastAccessed > -1) {
				least = PageTable[j].lastAccessed;
				find = j;
			}
		}
		// j is Recently Used loaded page,set its PageTable entry to indicate "no longer loaded"
		//increment the nReplaces counter
		nReplaces++;
		if(PageTable[find].status.modified == 1) {
			nSaves++;
		}
		// use the frame that backed that page.
		int change = PageTable[find].frameNo;
		// set its PageTable entry to indicate "no longer loaded" 
		PageTable[find].lastAccessed = -1;
		PageTable[find].status.loaded = 0;
		PageTable[find].status.modified = 0;
		PageTable[find].frameNo = -1;
		nLoads++;
		MemFrames[change] = pagenum;
		PageTable[pagenum].status.loaded = 1;
		if(action == 'W') {
			PageTable[pagenum].status.modified = 1;
		}
		PageTable[pagenum].frameNo = change;
		PageTable[pagenum].lastAccessed = clock;
		physicaladdress = (PageTable[pagenum].frameNo * PAGESIZE) + offset;
		return physicaladdress;
		}
	return physicaladdress;
}


// allocate and initialise Page Table
void initPageTable (void)
{
	if ((PageTable = calloc (nPages, sizeof (PTE))) == NULL)
		err (EX_OSERR, "couldn't allocate PageTable");

	for (uint i = 0; i < nPages; i++)
		PageTable[i] = (PTE) {
			.status = { .loaded = 0, .modified = 0 },
			.frameNo = -1,
			.lastAccessed = -1
		};
}


// allocate and initialise Memory Frames
void initMemFrames (void)
{
	if ((MemFrames = calloc (nFrames, sizeof (int))) == NULL)
		err (EX_OSERR, "couldn't allocate MemFrames");

	for (uint i = 0; i < nFrames; i++)
		MemFrames[i] = -1;
}


// dump contents of PageTable and MemFrames
void showState (void)
{
	printf ("\nPageTable (Stat,Acc,Frame)\n");
	for (uint pno = 0; pno < nPages; pno++) {
		PTE *p = &PageTable[pno];
		printf (
			"[%2d] %c%c, %2d, %2d",
			pno,
			p->status.loaded   ? 'L' : '-',
			p->status.modified ? 'M' : '-',
			p->lastAccessed, p->frameNo
		);

		int f = p->frameNo;
		if (f >= 0) printf (" @ %d", f * PAGESIZE);

		printf ("\n");
	}

	printf ("MemFrames\n");
	for (uint fno = 0; fno < nFrames; fno++)
		printf (
			"[%2d] %2d @ %d\n",
			fno, MemFrames[fno], fno * PAGESIZE
		);
}
