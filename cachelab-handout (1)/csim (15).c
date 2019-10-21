/* Cachelab-----CS 2011
 *
 * Written by Katie Lin (klin2) and Carley Gilmore (cngilmore)
 * */

#include "cachelab.h"
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>

typedef struct line{
	int valid_bit;			//determines if data is good, ever been accessed
	int lru;			 //least recently used--clock cycles that has elapsed, aka lru
	unsigned int tag;	 //identifier for each line
} line_t;

typedef struct set{
	line_t* lines;
} set_t;

typedef struct cache{
	int s;				 //determines how many sets are in the cache
	int E;			    //determines how many lines per set are in the cache
	int b; 				//determines how many bits are in a line
	int B; 				//init B 2^B
	int size; //size in bytes accessed
	//int sets;
	//struct line** line;
	int misses; 		 //counter for misses
	int hits;   		 //counter for hits
	int evictions;		//counter for evictions
	set_t* sets; 		//determines the given sets of lines per cache
} cache_t;

/*
 * Function that initializes the cache.
 */
cache_t* init_cache(int s, int E, int b){

	//Allocate space for the lines:
	cache_t* cache = (cache_t*) malloc(sizeof(cache_t));

	//set cache parameters from inputs
	cache->s=s;
	cache->E=E;
	cache->b=b;

	//set misses, hits, and evictions to 0
	cache->misses=0;
	cache->hits=0;
	cache->evictions=0;
	int setSize = (1 << s);  						//defined constant for 2^s
	cache->size=setSize;
	int setB = (1 << b);							//defined constant for 2^b
	cache->B=setB;

	//Next allocate space for all the lines:
	cache->sets= (set_t*)malloc(sizeof(set_t)*setSize);

	//Next initialize all values inside the sets to zero
	for(int i=0; i< setSize; i++){
		//index into set structure, give i set, allocate into lines field
		cache->sets[i].lines = (line_t*)malloc(sizeof(line_t)*E);
		for(int j=0; j< E; j++){
			cache->sets[i].lines[j].valid_bit = 0; //set cache fields
			cache->sets[i].lines[j].lru = 0;
			cache->sets[i].lines[j].tag = 0;
		}
	}

	//want to return pointer to a cache
	return cache;
}


/*
 *Function that just prints out the user help menu.
 */
void useHelpMenu(){
	printf("Usage: ./csim [-hv] -s <num> -E <num> -b <num> -t <file>\n");
	printf("Options:\n");
	printf("  -h         Print help message.\n");
	printf("  -v         Optional verbose flag.\n");
	printf("  -s <num>   Number of set index bits.\n");
	printf("  -E <num>   Number of lines per set.\n");
	printf("  -b <num>   Number of block offset bits.\n");
	printf("  -t <file>  Trace file.\n");
	printf("Examples:\n");
	printf("linux>  ./csim -s 4 -E 1 -b 4 -t traces/yi.trace\n");
	printf("linux>  ./csim -v -s 8 -E 2 -b 4 -t traces/yi.trace\n");
	exit(0);
}


/*
 * Function that takes the cache and frees the data
 */
void cleanup(cache_t* cache){
	for(int i = 0; i < cache->size; i++) {
		free(cache->sets[i].lines);
	}
	free(cache->sets);
	free(cache);
}


/*Function that checks if the set is full (isSetFull)
*		if the set is not full then put that tag in the 0/0/0 slot one of those lines is going to have a valid bit of 0
*		if it is full then you want to evict the LRU value //all of the valid bits in the lines are 1
*/
int isSetFull(cache_t* newCache, int set){

	int full = 1;  														//if it is full, then return 1, if not full return 0; assuming is full
	for(int i=0;i < newCache->E;i++){								   // if E=4, 4 lines per set
			if ((newCache->sets[set].lines[i].valid_bit) != 1){ 		//if the valid bit is not full
				full = 0;
			}
	}
	return full;
}


/*Function to return the index of the first spot where the valid bit is not 1(or 0)
 */
int getFirstEmptyPosition(cache_t* newCache, int set){

	int index = -1; //default value in case they are all valid
	for(int i=0;i < newCache->E;i++){ // if E=4, 4 lines per set
			if ((newCache->sets[set].lines[i].valid_bit) != 1){ //if the valid bit is not full
				index = i;
				break;
			}
	}
	return index;
}

/*
 * Function that increments the lru (age) of every item in the cache.
 */
void incrementLRU(cache_t* newCache, int set){

	for(int i = 0; i < newCache->E; i++){
		int age = 0;
		age = newCache->sets[set].lines[i].lru++;    					 //increase lru for every element
		}
}


/*TASKS
*-function that records hits (in access cache)
*		if there's a hit all that happens is hits is incremented the age of everything is incremented
*-hit or miss you're going to have to reset the age of the line that was just used
*		if you have two lines with the same LRU you're going to evict the one that you least recently accessed (placement in the array)
*
*lines in sets : tag, valid bit, lru
* hit--> you don't need to change the tag
* 		the first time you access the line that is a hit you need to make valid bit = 1
* 		lru the line you're on needs an lru of 0, everything else needs to lru++
* miss with vacant--> the tag is going to change relative to wherever the miss is placed.
* 					lru accessed line is 0 everything else increments
* (X)miss with evicted--> the tag is going to change to the tag of the evicted lot
* 						lru accessed line is 0 everything else increments
*/
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/*
 * Function that looks at the cache data and records the hits, misses, and evictions
 */
cache_t* accessCache(cache_t* newCache, unsigned long address, char instrType){

	//NOTES
	//if 04d75 -> 23 // look up 23 - hit, 27, miss, fill empty with 27, you tried to access
	//check valid bit, only will be 0 at beginning of program, means empty, tell if occupied or not
	//address tells set index
	//tag/valid bit/age
	//increment age of everything that wasn't used
	//lru is reset every time for the line that was accessed.
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	//get the tag, and get the set bit for indexing later
	unsigned int tag = (address >> newCache->s) >> newCache->b;
	int set = (address << (64-((newCache->s)-(newCache->b))) >> (64-(newCache->s)));


	//access cache structure, define as local variables for easy use
	int s = newCache->s;									  //determines how many sets are in the cache
	int E = newCache->E;								   	  //determines how many lines per set are in the cache
	int b = newCache->b; 									  //determines how many bits are in a line

	int setHit = 0; 										  //if there is a hit, set to 1, if miss, set 0
	int setEvict = 0; 										  //if needs to be evicted set 1


	//HANDLE HITS
	//compare the blocks for a match in the row for the set; find the hits
	int hitIndex = 0;
	for(int i=0;i < E;i++){ // if E=4, 4 lines per set

		if (((newCache->sets[set].lines[i].tag) == tag) && ((newCache->sets[set].lines[i].valid_bit) == 1)){
			newCache->hits++;
			setHit = 1;
			hitIndex = i;

			//make sure that the instruction is not 'M'
			if(instrType != 'M'){
				newCache->hits++;			//'M' will count hits twice
				printf("Hit ");
			}

			printf("Hit\n");
		}
	}

	newCache->sets[set].lines[hitIndex].lru = 0;

	 ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



	//HANDLE MISSES/EVICTIONS
	//if there is not a hit, handles the misses and evictions
	if(!setHit){
		newCache->misses++;
		printf("Miss\n");


		//HANDLE EVICTIONS
		//function that checks which line in a given set is the greatest LRU value same thing as age (maxLRU)
		//if it has the greatest LRU after a miss is filed then it will be evicted and replaced with the value accordingly
		//if set is full, then evict the line with the greatest age meaning the least recently used
		int maxLRU = 0;
		int maxLRUIndex = 0;

		for(int i = 0; i < E; i++){
			int age = 0;
			age = newCache->sets[set].lines[i].lru;    				 // increment somewhere else++;  //increase lru for every
			if(age >= maxLRU){										// if >=, takes care of edge case where lru might be the same
				maxLRU = age;
				maxLRUIndex = i;
			}
		}

		//this is for evict // this is for max lru
		 newCache->sets[set].lines[maxLRUIndex].lru = 0;
		 newCache->sets[set].lines[maxLRUIndex].tag = tag;

		 newCache->evictions++;
		 setEvict = 1;

		 ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


		//check for empty lines
		int vacantIndex;
		int occupiedIndex;

		/*for (int i =0; i< newCache->E;i++){
			if((newCache->sets[set].lines[i].valid_bit) != 1){
				vacantIndex = i;
				occupiedIndex = 0; //set not in use
				break;
			}
		}*/
		/*if(occupiedIndex == 1){
			for (int i =0; i< newCache->E;i++){
		//if()
				;
			}
		}*/

		 ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		//print out if there was an eviction
		if(setEvict){
			printf("Eviction. ");
		}

		//Increment hits for'M' instruction
		if(instrType != 'M'){
			newCache->hits++;			//'M' will count hits twice
			printf("Hit ");
		}
		//take care of M instruction

		//print endline
		printf("\n");



		//find first empty line, and if no empty lines, must get rid of something

	}
	//END OF HANDLING MISSES/EVICTIONS
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



	return newCache;
}


/*
 * Function that parses the input data given from the file.
 */
cache_t* parse_input(FILE* fp, cache_t* newCache){
	char *str;

		// read each line at a time until end-of-file, or EOF
		while (fgets(str, 20, fp) != NULL) {

		    char instrType;											     // holds first character of line
		    unsigned long address; 										 // holds second character of line
		    int size;													 // holds integer address of line

		    //Handles the different cases for memory traces
		    //		Note: Case 'I' is intruction load
		    if(str[0]== 'I'){
		    	continue;												 // nothing special, move on
		    }

		    sscanf(str, " %c %llx, %d", &instrType, &address, &size);	 //scan in the first two characters followed by the address
		    switch(instrType){
		    case 'L':
		    	newCache = accessCache(newCache, address, instrType);
		    	break;
		    case 'S':
		    	newCache = accessCache(newCache, address, instrType);
		    	break;
		    case 'M':
		    	newCache = accessCache(newCache, address, instrType); 				  //load and save at the same time
		    	newCache = accessCache(newCache, address, instrType);
		    	break;
		    default:
		    	break;
		    }

		    //know what command it is, -> either (accessing cache)load, save, or modify(does both)
		}
	return newCache;
}


/*
 * Function that simulates a cache.
 */
int main(int argc, char* argv[]){

	//initialize variables needed to handle cache data
	char opt;
	//unsigned long isH = 0;
	int verFlag = 0;
	int instrNum = 0;  //initialize instruction number to 0
	int s = 0;
	int E = 0;
	int b = 0;
	int B = 0;
	instrNum = 1;
	char *fileName;

	//call the help menu if  there are no arguments
	if(argc == 1){
		useHelpMenu();
	}

	//make a cache
	cache_t* newCache;


	//Retrieving the user input from the different arguments and performing the set operations accordingly
	while((opt = getopt(argc, argv, "hvs:E:b:t:")) != -1){
		switch (opt){
		case 'h':
			//isH = 1;
			useHelpMenu();  //optional help flag that prints usage info
			break;
		case 'v':
			verFlag = 1; //set the verbose flag to true
			break;
		case 's':
			s = atoi(optarg);
			if(s<= 0){ 			//s can't be <=0 because s is the number of set index bits
				printf("The argument 's' must not be less than or equal to 0.\n");
				useHelpMenu();
			}
			break;
		case 'E':
			E = atoi(optarg);
			if(E<=0){			//E is the number of lines per set, so it cannot be negative or 0
				printf("The argument 'E' must not be less than or equal to 0.\n");
				useHelpMenu();
			}
			break;
		case 'b':
			b = atoi(optarg);
			if(b<=0){			//b is the number of block bits per line, so it cannot be negative or 0
				printf("The argument 'b' must not be less than or equal to 0.\n");
				useHelpMenu();
			}
			//newCache->b = b; //set the cache bits
			//B = (1<< b);
			//newCache->B = B;
			break;
		case 't':
			fileName = optarg; //sets the trace file argument in order to replay
			break;
		}
	}

	//print current file name and open the file
	printf("The file name is: %s\n", fileName);
	FILE *fp = fopen(fileName, "r"); //this will open up the trace file


	// make sure that there is a trace file to be opened
	if(fp == NULL){
		printf("No File.\n");
		exit(0);
	}

	//initialize cache structure
	newCache = init_cache(s, E, b);

	//parse cache data
	newCache = parse_input(fp, newCache);			//inside parse input, caching is handled


	//int maxNumLines = 267990; //maximum number of lines being checked for the long trace
	//int currLineNum = 1; // this counter is for checking the evictions on the instructions number
	//char instructLetter[maxNumLines][2];  //this is an array that contains only the first letter in the trace file
	//char lineAddress[maxNumLines][3]; //this is an array that constitutes all the addresses in the trace line


	fclose(fp);							//close the file

    printSummary(newCache->hits, newCache->misses, newCache->evictions);
    cleanup(newCache);

    return 0;

}
