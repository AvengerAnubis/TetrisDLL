#include <iostream>
#include <fcntl.h>
#include <io.h>
#include <Windows.h>

typedef unsigned char byte;

typedef void* TETRIS_HANDLE;

typedef void* (*INIT_FP)(short, short);
typedef void (*DESTROY_FP)(TETRIS_HANDLE);
typedef byte(*GET_FP)(TETRIS_HANDLE);
typedef bool(*GET_CS_FP)(TETRIS_HANDLE, short, short);
typedef bool(*TICK_FP)(TETRIS_HANDLE, byte);
#ifdef _DEBUG
typedef void(*SET_FP)(TETRIS_HANDLE, bool, short, short);
#endif

HANDLE console;

bool KEY[256];

void GetKEY()
{
	int i = 0;
	while (i < 256)
	{
		if (GetAsyncKeyState(i)) KEY[i] = 1; else KEY[i] = 0;
		i++;
	}
}

void print_field(TETRIS_HANDLE game, short w, short h, GET_CS_FP get_cell_state)
{
	CHAR_INFO* buf = new CHAR_INFO[w * h];
	buf->Attributes = 0x00FF;
	buf->Char.UnicodeChar = L'#';

}

int main()
{
	_setmode(_fileno(stdout), _O_U16TEXT);
	console = GetStdHandle(STD_OUTPUT_HANDLE);
	HMODULE tetris_dll;

	INIT_FP init;
	DESTROY_FP destroy;
	GET_FP get_gs, get_nf;
	GET_CS_FP get_cell_state;
	TICK_FP tick;

	TETRIS_HANDLE game;

	tetris_dll = LoadLibrary(L"TetrisDLL");
	if (!tetris_dll)
	{
		std::cout << "Unable to load TetrisDLL.dll" << std::endl;
		return 1;
	}

	init = (INIT_FP)GetProcAddress(tetris_dll, "init");
	destroy = (DESTROY_FP)GetProcAddress(tetris_dll, "destroy");
	get_gs = (GET_FP)GetProcAddress(tetris_dll, "get_gs");
	get_nf = (GET_FP)GetProcAddress(tetris_dll, "get_nf");
	get_cell_state = (GET_CS_FP)GetProcAddress(tetris_dll, "get_cell_state");
	tick = (TICK_FP)GetProcAddress(tetris_dll, "tick");
#ifdef _DEBUG
	set_data = (SET_FP)GetProcAddress(tetris_dll, "set_data");
#endif


	if (!(init && destroy && get_gs && get_nf && get_cell_state && tick))
	{
		std::cout << 
			"DLL library is incorrect or corrupted!\n" <<
			"Make sure you have downloaded dll file from official source!" 
		<< std::endl;
		std::cout << init << " "
			<< destroy << " "
			<< get_gs << " "
			<< get_nf << " "
			<< get_cell_state << " "
			<< tick << " "
			<< std::endl;
		return 2;
	}

	short w = 8, h = 12;
	byte control;
	game = init(8, 12);
	
	do
	{
		GetKEY();
		if (KEY[0x57])
		{
			tick(game, 3);
		}
		else if (KEY[0x58])
		{
			tick(game, 4);
		}
		else if (KEY[0x5A])
		{
			tick(game, 8);
		}
		else if (KEY[0x44])
		{
			tick(game, 2);
		}
		else if (KEY[0x53])
		{
			tick(game, 0);
		}
		else if (KEY[0x41])
		{
			tick(game, 1);
		}
		else 
		{
			tick(game, 0);
		}
		print_field(game, w, h, get_cell_state);
	} while (!KEY[VK_ESCAPE]);


	destroy(game);
	FreeLibrary(tetris_dll);
}