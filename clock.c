#include <stdio.h>
#include <stdlib.h>
#include <time.h>
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
#define BLOCK_WIDTH 2
#define NUM_WIDTH 3
#define NUM_HEIGHT 5
#define COLON_WIDTH 3
#define COLON_HEIGHT 5


int _debug, _numbers, _colons, _date, _date_length, _width, _height;

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

int colon[5] = { SPACE, BLOCK, SPACE, BLOCK, SPACE };


void setConsoleSize(int width, int height){
	#ifdef _WIN32
	HANDLE windowHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	COORD coord = {.X = width, .Y = height};
    SMALL_RECT windowSize = {0 , 0 , width-1, height-1};
    SetConsoleScreenBufferSize(windowHandle, coord);
    SetConsoleWindowInfo(windowHandle, TRUE, &windowSize);
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

void printBlock(int c){
	putchar(c);
	putchar(c);
}

void drawTime(struct tm *time){
	int date[_numbers], c = 0;
	if(_numbers/2 >= 1){
		date[c++] = time->tm_sec%10;
		date[c++] = time->tm_sec/10;
	}
	if(_numbers/2 >= 2){
		date[c++] = time->tm_min%10;
		date[c++] = time->tm_min/10;		
	}
	if(_numbers/2 >= 3){
		date[c++] = time->tm_hour%10;
		date[c++] = time->tm_hour/10;
	}

	for(int i = 0; i < 5; i++){
		c = _colons;
		putchar('\n');
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

	putchar('\n');
	if(_date > 0){
		putchar('\n');
		for(int i = 0; i < _width/2 - _date_length/2 - _date_length%2; i++){
			putchar(SPACE);
		}
		if(_date >= 1)
			printf("%02d", time->tm_mday);
		if(_date >= 2)
			printf("/%02d", time->tm_mon);
		if(_date >= 3)
			printf("/%d", time->tm_year+1900);
	}
}

int main(int argc, char **argv){

	if(argc < 3){
		setConsoleSize(120, 30);
		printf("Usage: clock.exe <T> <D> <B>\n");
		printf(	"<T> (time)\n"
				"\tIf T is below 0 it will be set to 0\n"
				"\tIf T is above 3 it will be set to 3\n"
				"\t\t-T = 0: display nothing\n"
				"\t\t-T = 1: display seconds\n"
				"\t\t-T = 2: above + minutes\n"
				"\t\t-T = 3: above + hours\n");
		printf( "<D> (date)\n"
				"\tIf D is below 0 it will be set to 0\n"
				"\tIf D is above 3 it will be set to 3\n"
				"\t\t-D = 0: display nothing\n"
				"\t\t-D = 1: display day\n"
				"\t\t-D = 2: above + month\n"
				"\t\t-D = 3: above + year\n");
		printf( "<B> (debug - optional)\n\n"
				"\tB doesnt need to be set\n"
				"\tIf B is below 0 it will be set to 0\n"
				"\tIf B is above 1 it will be set to 1\n"
				"\t\t-B = 0: display nothing\n"
				"\t\t-B = 1: turn on debugging mode");
		return 0;
	}
	if(argc == 4){
		_debug = atoi(argv[3]);
	}

	_numbers = atoi(argv[1])*2;
	_colons = (_numbers/2) - 1;
	_date = atoi(argv[2]);
	_date_length = (_date > 0)*(_date*2 + (_date == 3)*2 + (_date-1));
	_width = (_numbers*NUM_WIDTH + _colons*COLON_WIDTH + (_numbers/2)*PADDING + 2*PADDING)*BLOCK_WIDTH;
	_height = NUM_HEIGHT + (_date > 0)*2 + 2*PADDING;

	if(_debug > 0){
		setConsoleSize(120, 30);
		printf(	"nums: %d\n"
				"colons: %d\n"
				"date: %d\n"
				"width: %d\n"
				"height: %d\n",
				_numbers, _colons, _date, _width, _height);
		return 0;
	}

	#ifdef _WIN32
	system("@ECHO OFF");
	system("cls");
	SetConsoleTitle("CLOCK");
	#else
	system("clear");
	#endif
	setConsoleSize(_width, _height);
	time_t rawtime;
	
	while(1){
		cursorVisible(0);
		time(&rawtime);
		struct tm *curr_time = localtime(&rawtime);
		gotoxy(0, 0);
		drawTime(curr_time);
	}

	return 0;
}