csim.c                                                                                              0000664 0001750 0001750 00000031143 13462571556 012135  0                                                                                                    ustar   student                         student                                                                                                                                                                                                                /* Cachelab-----CS 2011
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
    int misses; 		 //counter for misses
    int hits;   		 //counter for hits
    int evictions;		//counter for evictions
    set_t* sets; 		//determines the given sets of lines per cache
} cache_t;

/*
 * Function that initializes the cache with the set parameters.
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