/*
 * CS 261 Data Structures
 * Assignment 5
 * Name: Pavel Gordeyev
 * Date: 3/10/20
 */

#include "hashMap.h"
#include <assert.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/**
 * Allocates a string for the next word in the file and returns it. This string
 * is null terminated. Returns NULL after reaching the end of the file.
 * @param file
 * @return Allocated string or NULL.
 */
char* nextWord(FILE* file)
{
    int maxLength = 16;
    int length = 0;
    char* word = malloc(sizeof(char) * maxLength);
    while (1)
    {
        char c = fgetc(file);
        if ((c >= '0' && c <= '9') ||
            (c >= 'A' && c <= 'Z') ||
            (c >= 'a' && c <= 'z') ||
            c == '\'')
        {
            if (length + 1 >= maxLength)
            {
                maxLength *= 2;
                word = realloc(word, maxLength);
            }
            word[length] = c;
            length++;
        }
        else if (length > 0 || c == EOF)
        {
            break;
        }
    }
    if (length == 0)
    {
        free(word);
        return NULL;
    }
    word[length] = '\0';
    return word;
}

/**
 * Loads the contents of the file into the hash map.
 * @param file
 * @param map
 */
void loadDictionary(FILE* file, HashMap* map)
{
    char * word = nextWord(file);

    int count = 0;

    while(word != NULL){
        hashMapPut(map,word,count);
        free(word);
        word = nextWord(file);
        count++;
    }

}

/**
 * Returns the minimum of 3 numbers
 * @param a
 * @param b
 * @param c
 */
int minInts(int a, int b, int c){

    if(a < b){
        if(a < c){
            return a;
        }else{
            return c;
        }
    }else{
        if(b < c){
            return b;
        }else{
            return c;
        }
    }
}

/**
 * Returns the Levenshtein distance for a given key and input
 * @param key
 * @param input
 */
int getLevDist(const char* key, const char* input){

    int keyLen = strlen(key) + 1,
        inputLen = strlen(input) + 1,
        levDist;

    // Create 2D matrix for Levenshtein distances
    int ** levMat = (int**)malloc(sizeof(int*) * keyLen);
    assert(levMat != 0);

    for(int i = 0; i < keyLen; i++){
        levMat[i] = (int*)malloc(sizeof(int) * inputLen);
    }

    // Fill in the left column with 0 to length of key
    for(int i = 0; i < keyLen; i++){
        levMat[i][0] = i;
    }

    // Fill in the top row with 0 to length of input
    for(int i = 0; i < inputLen; i++){
        levMat[0][i] = i;
    }

    // Populate the rest of the matrix

    int a,b,c;

    for(int i = 1;i < keyLen; i++){
        for(int j = 1;j < inputLen; j++){
            a = levMat[i-1][j] + 1;
            b = levMat[i][j-1] + 1;
            c = levMat[i-1][j-1];

            // c + 1 if key[i] is not equal to input[j]
            if(key[i-1] != input[j-1]){
                c++;
            }

            levMat[i][j] = minInts(a,b,c);
        }
    }

    levDist = levMat[keyLen-1][inputLen-1];

    // Free memory
    for(int i = 0;i < keyLen; i++){
        free(levMat[i]);
    }

    free(levMat);

    return levDist;
}

/**
 * Sets the value of a key in the hashmap to the Levenshtein distance
 * @param map
 * @param input
 */
void setLevDist(HashMap * map, char * input){

    int levDist;

    for(int i = 0;i < hashMapCapacity(map);i++){

        HashLink * link = map->table[i];

        while(link != NULL){
            levDist = getLevDist(link->key,input);
            
            hashMapPut(map,link->key,levDist);

            link = link->next;
        }
    }
}

/**
 * Prints out the closest matches to the input word, based on the Levenshtein distance
 * @param map
 * @param inputBuffer - lowercased version of input
 * @param inputOrig - original input from user with varying casing
 */
void printClosestMatches(HashMap * map,char * inputBuffer, char * inputOrig){
	int i = 0,
		levMin = 1,
		count = 0,
		maxCount = 5;

	printf("\n**** Note: Delay when using valgrind...Please wait for words to load! ****\n");
	printf("The inputted word %s is spelled incorrectly. Did you mean ... ?\n",inputOrig);

	// Reset hashmap values with the Levenshtein distance from the input
	setLevDist(map,inputBuffer);

	// Loop through hashmap to find the lowest value of Levenshtein distance
	// Print out up to 5 values
	while(i < hashMapCapacity(map) && count < maxCount){

		HashLink * link = map->table[i];

		while(link != NULL && count < maxCount){
			if(link->value == levMin){
				printf("- %s\n",link->key);
				count++;
			}

			link = link->next;
		}

		i++;

		// Not enough values found; increment levMin and try again
		if(i == hashMapCapacity(map) && count < maxCount){
			levMin++;
			i = 0;
		}
	}
}

/**
 * Checks the spelling of the word provded by the user. If the word is spelled incorrectly,
 * print the 5 closest words as determined by a metric like the Levenshtein distance.
 * Otherwise, indicate that the provded word is spelled correctly. Use dictionary.txt to
 * create the dictionary.
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, const char** argv)
{
    HashMap* map = hashMapNew(1000);

    FILE* file = fopen("dictionary.txt", "r");
    clock_t timer = clock();
    loadDictionary(file, map);
    timer = clock() - timer;
    printf("\nDictionary loaded in %f seconds\n", (float)timer / (float)CLOCKS_PER_SEC);
    fclose(file);

    char inputBuffer[256];

    int	quit = 0;

    while (!quit)
    {	
        printf("\nEnter a word or \"quit\" to quit: ");
        scanf("%s", inputBuffer);

        char inputOrig[256];
        strcpy(inputOrig,inputBuffer);

        // Remove case-sensitivty; set all user input to lower case
        for(int i = 0;inputBuffer[i] != '\0';i++){
            inputBuffer[i] = tolower(inputBuffer[i]);
        }

        if (strcmp(inputBuffer, "quit") == 0){
            quit = 1;
            printf("\n");
        }else{

            // Check if there is a key match in the dictionary
            if(hashMapContainsKey(map,inputBuffer)){
                printf("\nThe inputted word %s is spelled correctly\n",inputOrig);
            }else{
            	printClosestMatches(map,inputBuffer,inputOrig);
            }
        }
    }

    hashMapDelete(map);
    return 0;
}

