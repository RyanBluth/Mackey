#define WINVER 0x0500
#define LINE_BUFFER_DEFAULT 128
#define TEXT_BUFFER_DEFAULT 1024
#define NO_TIME -1

#include <windows.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>

struct KeyAction {
	char   key;
	float  minTime;
	float  maxTime;
};


void readFile(const char* filename, KeyAction** actions, int* numActions) {
	FILE * f = fopen(filename, "r+");
	fseek(f, 0, SEEK_END);
	long fsize = ftell(f);
	rewind(f);

	char* fc = (char*)malloc(fsize + 1);
	fread(fc, fsize, 1, f);
	fclose(f);
	
	fc[fsize] = 0;

	int numLines = 0;
	size_t linesSize = LINE_BUFFER_DEFAULT;
	size_t currSize = TEXT_BUFFER_DEFAULT;
	char** lines = (char**)malloc(sizeof(char*) * linesSize);
	char* curr = (char*)malloc(sizeof(char) * currSize);
	int c = 0;
	for (int i = 0; i < fsize; ++i) {
		if (c > currSize) {
			realloc(curr, sizeof(char) * (currSize += TEXT_BUFFER_DEFAULT));
		}
		if (i > linesSize) {
			realloc(lines, sizeof(char*) * (linesSize += LINE_BUFFER_DEFAULT));
		}
		char tok = fc[i];
		if (tok == '\n' || i == fsize - 1) {
			size_t currSize = sizeof(char) * c + 1;
			char * newLine = (char*)malloc(currSize);
			memcpy(newLine, curr, c);
			newLine[c] = '\0';
			lines[numLines] = newLine;
			++numLines;
			c = 0;
			continue;
		}
		curr[c] = fc[i];
		c++;
	}

	free(curr);

	*actions = (KeyAction *)malloc(sizeof(KeyAction) * numLines);
	*numActions = numLines;

	enum Token {
		UNKOWN = 0,
		KEY = 1,
		MIN_TIME = 2,
		MAX_TIME = 3
	};
	for (int i = 0; i < numLines; i++){
		int idx = 0;
		KeyAction ka = {};
		ka.maxTime = NO_TIME;
		ka.minTime = NO_TIME;
		Token curTokType = UNKOWN;
		char * curToken = (char*)malloc(sizeof(char) * 17);
		for (int x = 0;;) {
			char tok = lines[i][idx++];
			if (tok == ' ' || tok == '\0' || x == 16 ) {
				curToken[x] = '\0';
				switch (curTokType + 1) {
				case KEY:
					ka.key = curToken[x-1];
					break;
				case MIN_TIME:
					ka.minTime = atof(curToken);
					break;
				case MAX_TIME:
					ka.maxTime = atof(curToken);
					break;
				}
				curTokType = (Token)((int)curTokType + 1);
				x = 0;
			}else {
				curToken[x] = tok;
				x++;
			}
			if(tok == '\0') {
				if(ka.minTime == NO_TIME) {
					printf("Min time must be provided");
					exit(1);
				}
				if(ka.maxTime == NO_TIME) {
					ka.maxTime = ka.minTime;
				}
				(*actions)[i] = ka;
				break;
			}
		}
		free(curToken);
	}
	free(lines);
	free(fc);
}


int main(int argc, const char* argv[]){
	
	srand(time(NULL));

	INPUT ip;

	ip.type = INPUT_KEYBOARD;
	ip.ki.wScan = 0;
	ip.ki.time = 0;
	ip.ki.dwExtraInfo = 0;

	int * numActions = (int*)malloc(sizeof(int));
	KeyAction ** actions = (KeyAction**)malloc(sizeof(KeyAction*));

	readFile(argv[1], actions, numActions);

	while (true) {
		for (int i = 0; i < *numActions; i++) {
			KeyAction ka = (*actions)[i];
			float r = rand() / ( ka.maxTime / ka.minTime);
			
			Sleep(r);

			ip.ki.wVk = toupper(ka.key);
			ip.ki.dwFlags = 0; 
			SendInput(1, &ip, sizeof(INPUT));

			ip.ki.dwFlags = KEYEVENTF_KEYUP;
			SendInput(1, &ip, sizeof(INPUT));
		}
	}
}


