#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#endif

#define PADDING 1
#define BLOCK 219 // █
#define SPACE 32 // ' '
#define ENTER '\n'
#define BLOCK_WIDTH 2
#define NUM_WIDTH 3
#define NUM_HEIGHT 5
#define COLON_WIDTH 3
#define COLON_HEIGHT 5
#define FLAGS 9


enum Bitmask {
	SECOND, MINUTE, HOUR,
	DAY, MONTH, YEAR,
	AM_PM,
	DEFAULT,
	DEBUG,
	TERMINATE
};

char flags[FLAGS] = "sthdmyarb";

int _mask = '\x00', _default_mask = '\x3F';

int _debug, _numbers, _colons, _date, _date_length, _width, _height;

char daysLetters[7] = "MTWRFSU";

char *days[7] = { 
		"Monday", "Tuesday", "Wednesday", "Thursday",
		"Friday", "Saturday", "Sunday" };

char monthsLetters[12] = "JFMAMJJASOND";

char *months[12] = { 
		"January", "February", "March", "April", "May", "June",
		"July", "August", "September", "October", "November", "December" };

char usage[] = "Usage: clock.exe -[FLAGS]... -[FLAGS]...\n"
				"<T> (time)\n"
				"\tIf T is below 0 it will be set to 0\n"
				"\tIf T is above 3 it will be set to 3\n"
				"\t\t-T = 0: display nothing\n"
				"\t\t-T = 1: display seconds\n"
				"\t\t-T = 2: above + minutes\n"
				"\t\t-T = 3: above + hours\n"
				"<D> (date)\n"
				"\tIf D is below 0 it will be set to 0\n"
				"\tIf D is above 3 it will be set to 3\n"
				"\t\t-D = 0: display nothing\n"
				"\t\t-D = 1: display day\n"
				"\t\t-D = 2: above + month\n"
				"\t\t-D = 3: above + year\n"
				"<B> (debug - optional)\n\n"
				"\tB doesnt need to be set\n"
				"\tIf B is below 0 it will be set to 0\n"
				"\tIf B is above 1 it will be set to 1\n"
				"\t\t-B = 0: display nothing\n"
				"\t\t-B = 1: turn on debugging mode";

int colon[5] = { SPACE, BLOCK, SPACE, BLOCK, SPACE };

int numbers[10][5][3] = { 
	{
		{BLOCK, BLOCK, BLOCK},	// 0
		{BLOCK, SPACE, BLOCK},
	  	{BLOCK, SPACE, BLOCK},
		{BLOCK, SPACE, BLOCK},
		{BLOCK, BLOCK, BLOCK}
	},
	{
		{SPACE, SPACE, BLOCK},	// 1
		{SPACE, SPACE, BLOCK},
		{SPACE, SPACE, BLOCK},
		{SPACE, SPACE, BLOCK},
		{SPACE, SPACE, BLOCK}
	},
	{
		{BLOCK, BLOCK, BLOCK},	// 2
		{SPACE, SPACE, BLOCK},
		{BLOCK, BLOCK, BLOCK},
		{BLOCK, SPACE, SPACE},
		{BLOCK, BLOCK, BLOCK}
	},
	{
		{BLOCK, BLOCK, BLOCK},	// 3
		{SPACE, SPACE, BLOCK},
		{BLOCK, BLOCK, BLOCK},
		{SPACE, SPACE, BLOCK},
		{BLOCK, BLOCK, BLOCK}
	},
	{
		{BLOCK, SPACE, BLOCK},	// 4
		{BLOCK, SPACE, BLOCK},
		{BLOCK, BLOCK, BLOCK},
		{SPACE, SPACE, BLOCK},
		{SPACE, SPACE, BLOCK}
	},
	{
		{BLOCK, BLOCK, BLOCK},	// 5
		{BLOCK, SPACE, SPACE},
		{BLOCK, BLOCK, BLOCK},
		{SPACE, SPACE, BLOCK},
		{BLOCK, BLOCK, BLOCK}
	},
	{
		{BLOCK, BLOCK, BLOCK},	// 6
		{BLOCK, SPACE, SPACE},
		{BLOCK, BLOCK, BLOCK},
		{BLOCK, SPACE, BLOCK},
		{BLOCK, BLOCK, BLOCK}
	},
	{
		{BLOCK, BLOCK, BLOCK},	// 7
		{SPACE, SPACE, BLOCK},
		{SPACE, SPACE, BLOCK},
		{SPACE, SPACE, BLOCK},
		{SPACE, SPACE, BLOCK}
	},
	{
		{BLOCK, BLOCK, BLOCK},	// 8
		{BLOCK, SPACE, BLOCK},
		{BLOCK, BLOCK, BLOCK},
		{BLOCK, SPACE, BLOCK},
		{BLOCK, BLOCK, BLOCK}
	},
	{
		{BLOCK, BLOCK, BLOCK},	// 9
		{BLOCK, SPACE, BLOCK},
		{BLOCK, BLOCK, BLOCK},
		{SPACE, SPACE, BLOCK},
		{BLOCK, BLOCK, BLOCK}
	}
};


void setBit(int *bit, int n){
	*bit |= (1 << n);
}

void clearBit(int *bit, int n){
	*bit &= ~(1 << n);
}

void flipBit(int *bit, int n){
	*bit ^= (1 << n);
}

int getBit(int bit, int n){
	return (bit & (1 << n)) != 0;
}

void clearScreen(){
	#ifdef _WIN32
	system("cls");
	#else
	system("clear");
	#endif
}

void csleep(int milliseconds){
	#ifdef _WIN32
	Sleep(milliseconds);
	#else
	float fmilliseconds = milliseconds/1000;
	sleep(fmilliseconds);
	#endif
}

void setConsoleSize(int width, int height){
	#ifdef _WIN32
	HANDLE windowHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	COORD coord = {.X = width, .Y = height};
    SMALL_RECT windowSize = {0 , 0 , width-1, height-1};
    SetConsoleScreenBufferSize(windowHandle, coord);
    SetConsoleWindowInfo(windowHandle, TRUE, &windowSize);

    CONSOLE_SCREEN_BUFFER_INFO scr;
	GetConsoleScreenBufferInfo(windowHandle, &scr);
	while(width != scr.dwSize.X || height != scr.dwSize.Y){
		setConsoleSize(width, height);
		GetConsoleScreenBufferInfo(windowHandle, &scr);
	}
    #else
    struct winsize ws;
    int fd;
    /* Open the controlling terminal. */
    fd = open("/dev/tty", O_RDWR);
    if(fd < 0)
        exit(1);
    /* Get window size of terminal. */
    if (ioctl(fd, TIOCGWINSZ, &ws) < 0)
        exit(1);
    //height = &ws.ws_row;
    //width = &ws.ws_col;
    close(fd);
    #endif
}

void cursorVisible(int q){
	#ifdef _WIN32
    HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO info;
    info.dwSize = 25;
    if(q == 1){
        info.bVisible = TRUE;
    }else if(q == 0){
        info.bVisible = FALSE;
    }
    SetConsoleCursorInfo(consoleHandle, &info);
    #else
    if(q == 1){
        printf("\e[?25h");
    }else if(q == 0){
        printf("\e[?25l");
    }
    #endif
}

void gotoxy(int x, int y){
	#ifdef _WIN32
    COORD coord = {.X = x, .Y = y};
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
    #else
    printf("\033[%d;%dH", y, x);
    #endif
}

void restoreConsoleSize(){
	setConsoleSize(120, 30);
	//clearScreen();
}

void printBlock(int c){
	putchar(c);
	putchar(c);
}

void drawTime(struct tm *time){
	int date[_numbers], c = 0;
	if(getBit(_mask, SECOND)){ date[c++] = time->tm_sec%10; date[c++] = time->tm_sec/10; }
	if(getBit(_mask, MINUTE)){ date[c++] = time->tm_min%10; date[c++] = time->tm_min/10; }
	if(getBit(_mask, HOUR)){
		if(getBit(_mask, AM_PM)){
			date[c++] = (((time->tm_hour+11)%12+1))%10;
			date[c++] = (((time->tm_hour+11)%12+1))/10;
		}else{
			date[c++] = time->tm_hour%10;
			date[c++] = time->tm_hour/10;
		}
	}

	for(int i = 0; i < 5; i++){
		c = _colons;
		putchar(ENTER);
		printBlock(SPACE);
		for(int j = 0; j < _numbers; j++){
			for(int k = 0; k < 3; k++){
				printBlock(numbers[date[_numbers-j-1]][i][k]);
			}
			printBlock(SPACE);
			if(c > 0){
				if(((j+1)%2 == 0) && (j < 5)){
					printBlock(colon[i]);
					printBlock(SPACE);
					c--;
				}	
			}
		}
	}

	if(getBit(_mask, AM_PM)){
		char ap = (time->tm_hour > 11)?'P':'A';
		putchar(ap);
		putchar('M');
		printBlock(SPACE);
	}

	putchar(ENTER);
	if(_date > 0){
		putchar(ENTER);
		for(int i = 0; i < _width/2 - _date_length/2 - _date_length%2; i++){
			putchar(SPACE);
		}
		if(getBit(_mask, DAY)){
			printf("%02d", time->tm_mday);
			if(getBit(_mask, MONTH) || getBit(_mask, YEAR)){
				putchar('/');
			}
		}
		if(getBit(_mask, MONTH)){
			printf("%02d", time->tm_mon);
			if(getBit(_mask, YEAR)){
				putchar('/');
			}
		}
		if(getBit(_mask, YEAR))
			printf("%d", time->tm_year+1900);
	}
}

int main(int argc, char **argv){

	atexit(restoreConsoleSize);

	// Finds flags and sets them in the mask
	for(int i = 1; i < argc; i++){
		if(argv[i][0] == '-'){
			int argSize = strlen(argv[i]);
			char *c;
			for(int j = 1; j < argSize; j++){
				if(c = strchr(flags, argv[i][j])){
					setBit(&_mask, c - flags);
				}else{
					printf("Invalid option '-%c'\n", argv[i][j]);
					setBit(&_mask, TERMINATE);
				}
			}
		}else{
			printf("Unrecognized option '%s'\n", argv[i]);
			setBit(&_mask, TERMINATE);
		}
	}

	// Sets the default settings if the flag is enabled
	if(getBit(_mask, DEFAULT)){
		setBit(&_mask, SECOND);
		setBit(&_mask, MINUTE);
		setBit(&_mask, HOUR);
		setBit(&_mask, DAY);
		setBit(&_mask, MONTH);
		setBit(&_mask, YEAR);
		clearBit(&_mask, AM_PM);
	}

	// Sets the enviroment variables
	_numbers = (getBit(_mask, SECOND) + getBit(_mask, MINUTE) + getBit(_mask, HOUR))*2;
	_colons = ((_numbers/2) - 1)*(_numbers > 0);
	_date = getBit(_mask, DAY) + getBit(_mask, MONTH) + getBit(_mask, YEAR);
	_date_length = (getBit(_mask, DAY) + getBit(_mask, MONTH))*2 + getBit(_mask, YEAR)*4 + (_date - 1);
	_width = (_numbers*NUM_WIDTH + _colons*COLON_WIDTH + (_numbers/2)*PADDING + 2*PADDING)*BLOCK_WIDTH + getBit(_mask, AM_PM)*4;
	_height = NUM_HEIGHT + (_date > 0)*2 + 2*PADDING;

	// Enables debug mode
	if(getBit(_mask, DEBUG)){
		restoreConsoleSize();
		printf(	"flags: ");
		for(int i = 0; i < FLAGS; i++){
			if(getBit(_mask, i))
				printf("-%c, \t", flags[i]);
		}
		printf(	"\n");
		printf(	"nums: %d\n"
				"colons: %d\n"
				"date: %d\n"
				"width: %d\n"
				"height: %d\n"
				"date length: %d\n",
				_numbers, _colons, _date, _width, _height, _date_length);
		setBit(&_mask, TERMINATE);
	}

	if(_numbers == 0 && _date == 0){
		printf("\nNo clock\n");
		setBit(&_mask, TERMINATE);
	}
	
	if(getBit(_mask, TERMINATE)){
		return 0;
	}

	#ifdef _WIN32
	system("@ECHO OFF");
	SetConsoleTitle("CLOCK");
	#endif
	clearScreen();
	setConsoleSize(_width, _height);
	time_t rawtime;
	
	while(1){
		cursorVisible(0);
		time(&rawtime);
		struct tm *curr_time = localtime(&rawtime);
		gotoxy(0, 0);
		drawTime(curr_time);
		csleep(125);
	}

	return 0;
}