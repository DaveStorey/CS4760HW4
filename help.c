#include<stdio.h>

void help(){
	printf("This program will take a file of strings (one per line, 80 characters or less, 100 strings max)\n and create two files: one with all palindromes in the file, one with all non-palindromes.\n");
	printf("There are four options:\n");
	printf("-h, which, as you have just discovered prints a help message.\n");
	printf("-s <number of children>, which specifies  the maximum number of children to be alive at any one time. This number cannot be greater than 20, default is 5.\n");
	printf("-n <number of children>, which specifies the maximum number of total children to be spawned.  Each child examines up to 5 strings, default is 10.\nIf there aren't enough strings for the children, there will be no file output from the later children.");
	printf("-i <inputfile>, which specifies the file with the strings.\n");
	printf("Selecting -i with a properly formatted  input file will cause the program\n");
	printf("to read the file and fork off children to analyze the strings in the file.\n");
	printf("-s, -n, -i, can be used in any combination.\n");
	printf("If no input file is specified, input.txt will be used; if no output, output.txt.\n");
}
