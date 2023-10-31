#include "pch.h"
#include "Tetris.h"

typedef unsigned char byte;
typedef char sbyte;

enum GAME_STATE
{
	START = 0,
	FIGURE_FALLING = 1,
	NEXT_FIGURE = 2,
	LINE_FULL = 3,
	GAME_OVER = 4
};

enum FIGURE_TYPES
{
	I_TYPE = 0,
	T_TYPE = 1,
	S_TYPE = 2,
	L_TYPE = 3
};

//w and h
int figure_sizes[] =
{
	65540, 131075, 131075, 131075
};
short figure_fillings[] =
{
	0b11110000'00000000,
	/*
	*	1
	*	1
	*	1
	*	1
	*/
	0b10111000'00000000,
	/*
	*	1
	*	11
	*	1
	*/
	0b10110100'00000000,
	/*
	*	1
	*	11
	*	 1
	*/
	0b10101100'00000000
	/*
	*	1
	*	1
	*	11
	*/
};

typedef struct 
{
	short w;
	short h;
	byte* data;
} FIELD;

typedef struct 
{
	short w, h, x, y;
	byte data;
} FIGURE;

typedef struct 
{
	FIELD* field;
	FIGURE* current_figure;
	byte data; //0b GGGG NNNN
	byte tick;
	short fullline_y;
} TETRIS;


void set_data(FIELD* field, bool data, short x, short y)
{
	byte* data_el = field->data + ((field->w * y + x) / 8);
	byte index = 7 - ((field->w * y + x) % 8);
	*data_el = (data) ? (*data_el | (1 << index)) : (*data_el & ~(1 << index));
}
bool get_data(FIELD* field, short x, short y)
{
	byte data_el = field->data[(field->w * y + x) / 8];
	byte index = 7 - ((field->w * y + x) % 8);
	return (bool)((data_el >> index) & 1);
}
void set_data(FIGURE* figure, bool data, short x, short y)
{
	byte index = 7 - ((figure->w * y + x) % 8);
	byte new_data = (data) ? (figure->data | (1 << index)) : (figure->data & ~(1 << index));
	figure->data = new_data;
}
bool get_data(FIGURE* figure, short x, short y)
{
	byte index = 7 - ((figure->w * y + x) % 8);
	return (bool)((figure->data >> index) & 1);
}
void set_data(byte* data_ptr, short w, bool data, short x, short y)
{
	byte* data_el = data_ptr + ((w * y + x) / 8);
	byte index = 7 - ((w * y + x) % 8);
	*data_el = (data) ? (*data_el | (1 << index)) : (*data_el & ~(1 << index));
}
bool get_data(byte* data_ptr, short w, short x, short y)
{
	byte data_el = data_ptr[(w * y + x) / 8];
	byte index = 7 - ((w * y + x) % 8);
	return (bool)((data_el >> index) & 1);
}
void set_data(byte data, short w, bool n_data, short x, short y)
{
	byte index = 7 - ((w * y + x) % 8);
	data = (n_data) ? (data | (1 << index)) : (data & ~(1 << index));
}
bool get_data(byte data, short w, short x, short y)
{
	byte index = 7 - ((w * y + x) % 8);
	return (bool)((data >> index) & 1);
}

#ifdef _DEBUG
void set_data(void* game_ptr, bool data, short x, short y)
{
	TETRIS* game = (TETRIS*)game_ptr;
	set_data(game->field, data, x, y);
}
#endif

bool check_collision(TETRIS* game)
{
	for (short i = 0; i < game->current_figure->h; ++i)
	{
		for (short j = 0; j < game->current_figure->w; ++j)
		{
			if (
				get_data(game->current_figure, j, i)
				&&
				get_data(game->field, game->current_figure->x + j, game->current_figure->y + i)
				)
			{
				return true;
			}
		}
	}
	return false;
}
void figure_to_field(TETRIS* game)
{
	for (short i = 0; i < game->current_figure->h; ++i)
	{
		for (short j = 0; j < game->current_figure->w; ++j)
		{
			if (get_data(game->current_figure, j, i))
			{
				set_data(game->field, 1, game->current_figure->x + j, game->current_figure->y + i);
			}
		}
	}
}
void shift_data(byte* data, short size)
{
	short counter = size;
	bool has_been_shifted = false;
	for (short j = 0; j < size; ++j)
	{
		counter -= !(get_data(data, size, j, 0));
	}
	if (!counter)
	{
		for (short i = 0; i < size - 1; ++i)
		{
			for (short j = 0; j < size; ++j)
			{
				set_data(data, size, get_data(data, size, j, i + 1), j, i);
			}
		}
		has_been_shifted = true;
	}
	counter = size;

	for (short i = 0; i < size; ++i)
	{
		counter -= !(get_data(data, size, 0, i));
	}
	if (!counter)
	{
		for (short i = 0; i < size; ++i)
		{
			for (short j = 0; j < size - 1; ++j)
			{
				set_data(data, size, get_data(data, size, j + 1, i), j, i);
			}
		}
		has_been_shifted = true;
	}
	
	if (has_been_shifted)
	{
		shift_data(data, size);
	}
}
void rotate(TETRIS* game, bool to_left)
{
	short size = (game->current_figure->w > game->current_figure->h) ? game->current_figure->w : game->current_figure->h;
	byte* data_r = new byte[size * size / 8 + 1];

	if (to_left)
	{
		for (short i = 0; i < size; ++i)
		{
			for (short j = 0; j < size; ++j)
			{
				if ((size - 1 - i) < game->current_figure->w && j < game->current_figure->h)
				{
					set_data(data_r, size, (get_data(game->current_figure, size - 1 - i, j)), j, i);
				}
				else
				{
					set_data(data_r, size, 0, j, i);
				}
			}
		}
	}
	else
	{
		for (short i = 0; i < size; ++i)
		{
			for (short j = 0; j < size; ++j)
			{
				if (i < game->current_figure->w && (size - 1 - j) < game->current_figure->h)
				{
					set_data(data_r, size, (get_data(game->current_figure, i, size - 1 - j)), j, i);
				}
				else
				{
					set_data(data_r, size, 0, j, i);
				}
			}
		}
	}
	
	shift_data(data_r, size);
	
	bool isOk = 
		(
			(game->current_figure->x + game->current_figure->h <= game->field->w)
			&&
			(game->current_figure->y + game->current_figure->w <= game->field->h)
		);

	for (short i = 0; i < game->current_figure->h && isOk; ++i)
	{
		for (short j = 0; j < game->current_figure->w && isOk; ++j)
		{
			isOk = !(
				get_data(game->field, j + game->current_figure->x, i + game->current_figure->y) 
				&& 
				get_data(data_r, size, j, i)
			);
		}
	}
	if (isOk)
	{
		short t = game->current_figure->w;
		game->current_figure->w = game->current_figure->h;
		game->current_figure->h = t;
		for (short i = 0; i < game->current_figure->h; ++i)
		{
			for (short j = 0; j < game->current_figure->w; ++j)
			{
				set_data(game->current_figure, get_data(data_r, size, j, i), j, i);
			}
		}
	}
	delete[] data_r;
}
bool move(TETRIS* game, byte control)
{
	/*
	*	Input: 0bxxxxxxAA (Arrows, or wasd or smhg)
	*		0	00	-	falling
	*		1	01	-	move left
	*		2	10	-	move right
	*		3	11	-	full falling
	*/
	bool can_move = true;
	do 
	{
		sbyte move_direction = (((control & 3) == 0) || ((control & 3) == 3)) ? 0 : (((control & 3) == 1) ? -1 : 1);

		if (move_direction)
		{
			if	(
					game->current_figure->x + move_direction < 0 
					||
					game->current_figure->x + game->current_figure->w + move_direction > game->field->w
				)
			{
				can_move = false;
			}
			else
			{
				game->current_figure->x += move_direction;
				if (check_collision(game))
				{
					game->current_figure->x -= move_direction;
					can_move = false;
				}
			}
		}
		else
		{
			if (game->current_figure->y + game->current_figure->h + 1 > game->field->h)
			{
				can_move = false;
			}
			else
			{
				++(game->current_figure->y);
				if (check_collision(game))
				{
					--(game->current_figure->y);
					can_move = false;
				}
			}
		}

	} while (can_move && ((control & 3) == 3));

	return can_move;
}
short check_fullline(TETRIS* game)
{
	short counter = game->field->w;
	for (short i = game->field->h; i >= 0; --i)
	{
		for (short j = 0; j < game->field->w; ++j)
		{
			if (get_data(game->field, j, i))
			{
				--counter;
			}
		}
		if (counter)
		{
			counter = game->field->w;
			continue;
		}
		return i;
	}
	return -1;
}
void remove_line(TETRIS* game, short y)
{
	for (short j = 0; j < game->field->w; ++j)
	{
		set_data(game->field, 0, j, y);
	}
}
void shift_down(TETRIS* game, short y)
{
	for (short i = y; i > 0; --i)
	{
		for (short j = 0; j < game->field->w; ++j)
		{
			set_data(game->field, get_data(game->field, j, i - 1), j, i);
		}
	}
}


void* init(short w, short h)
{
	TETRIS* game = new TETRIS;

	game->field = new FIELD;
	game->field->data = new byte[w * h / 8 + 1];
	for (int i = 0; i < w * h / 8 + 1; ++i)
	{
		game->field->data[i] = 0;
	}
	game->field->h = h;
	game->field->w = w;
	
	SYSTEMTIME time;
	GetSystemTime(&time);
	srand(time.wMilliseconds);
	byte figure = rand() % 4;
	game->data = rand() % 4;
	game->data += 0b00010000;
	game->tick = 0;

	game->current_figure = new FIGURE;
	game->current_figure->x = w / 2;
	game->current_figure->y = 0;
	game->current_figure->w = (figure_sizes[figure] >> 16) & 196607;
	game->current_figure->h = figure_sizes[figure] & 196607;
	game->current_figure->data = 0;
	for (short i = 0; i < game->current_figure->h; ++i)
	{
		for (short j = 0; j < game->current_figure->w; ++j)
		{
			bool data = (figure_fillings[figure] >> (15 - (j + i * game->current_figure->w))) & 1;
			set_data(game->current_figure, data, j, i);
		}
	}
	return game;
}

void destroy(void* game_ptr)
{
	TETRIS* game = (TETRIS*)game_ptr;
	delete[] game->field->data;

	delete game->field;
	delete game->current_figure;
}
byte get_gs(void* game_ptr)
{
	return ((((TETRIS*)game_ptr)->data >> 4) & 15);
}
byte get_nf(void* game_ptr)
{
	return (((TETRIS*)game_ptr)->data & 15);
}
bool get_cell_state(void* game_ptr, short x, short y)
{
	TETRIS* game = (TETRIS*)game_ptr;
	bool data = false;
	bool figure_data = false;
	if (game->current_figure != nullptr)
	{
		if ((
				x >= game->current_figure->x
				&&
				x < game->current_figure->x + game->current_figure->w
			)
			&&
			(
				y >= game->current_figure->y
				&&
				y < game->current_figure->y + game->current_figure->h
			))
		{
			figure_data = get_data(game->current_figure, x - game->current_figure->x, y - game->current_figure->y);
		}
	}
	data = (get_data(game->field, x, y)) || figure_data;
		
	return data;
}


bool tick(void* game_ptr, byte input)
{
	TETRIS* game = (TETRIS*)game_ptr;
	switch (game->data >> 4)
	{
		case START:
		{
			for (short i = 0; i < game->field->w * game->field->h / 8 + 1; ++i)
			{
				game->field->data[i] = 0;
			}

			SYSTEMTIME time;
			GetSystemTime(&time);
			srand(time.wMilliseconds);
			byte figure = rand() % 4;
			game->data = rand() % 4;
			game->data += 0b00010000;
			game->tick = 0;

			short prev_x = game->current_figure->x;
			delete game->current_figure;
			game->current_figure = nullptr;
			game->current_figure = new FIGURE;
			game->current_figure->x = prev_x;
			game->current_figure->y = 0;
			game->current_figure->w = (figure_sizes[figure] >> 16) & 0x0000FFFF;
			game->current_figure->h = figure_sizes[figure] & 0x0000FFFF;
			game->current_figure->data = 0;
			for (short i = 0; i < game->current_figure->h; ++i)
			{
				for (short j = 0; j < game->current_figure->w; ++j)
				{
					bool data = (figure_fillings[figure] >> (15 - (j + i * game->current_figure->w))) & 1;
					set_data(game->current_figure, data, j, i);
				}
			}
			break;
		}
		case FIGURE_FALLING:
		{
			if ((input >> 2) != 0)
			{
				rotate(game, input >> 3);
				if (game->current_figure->x + game->current_figure->w > game->field->w)
				{
					rotate(game, (~(input >> 3)) & 1);
				}
				if (game->current_figure->y + game->current_figure->h > game->field->h)
				{
					rotate(game, (~(input >> 3)) & 1);
				}
			}
			else
			{
				bool can_move = move(game, input);
				if (!can_move && (input == 0 || input == 3))
				{
					figure_to_field(game);
					for (short i = 0; i < game->current_figure->h; ++i)
					{
						for (short j = 0; j < game->current_figure->w; ++j)
						{
							set_data(game->current_figure, 0, j, i);
						}
					}
					if (check_fullline(game) != -1)
					{
						game->data += 0b00010000;
					}
					game->data += 0b00010000;
				}
			}

			break;
		}
		case NEXT_FIGURE:
		{
			byte figure = game->data & 0x0F;
			game->data = rand() % 4;
			game->data += 0b00010000;

			short prev_x = game->current_figure->x;
			delete game->current_figure;
			game->current_figure = new FIGURE;
			game->current_figure->y = 0;
			game->current_figure->w = (figure_sizes[figure] >> 16) & 0x0000FFFF;
			game->current_figure->h = figure_sizes[figure] & 0x0000FFFF;
			if (prev_x + game->current_figure->w > game->field->w)
			{
				prev_x = game->field->w - game->current_figure->w;
			}
			game->current_figure->x = prev_x;
			game->current_figure->data = 0;
			for (short i = 0; i < game->current_figure->h; ++i)
			{
				for (short j = 0; j < game->current_figure->w; ++j)
				{
					bool data = (figure_fillings[figure] >> (15 - (j + i * game->current_figure->w))) & 1;
					set_data(game->current_figure, data, j, i);
				}
			}
			if (check_collision(game))
			{
				game->data += 0b00110000;
			}

			break;
		}
		case LINE_FULL:
		{
			if (game->tick == 0)
			{
				game->fullline_y = check_fullline(game);
				remove_line(game, game->fullline_y);
				++game->tick;
			}
			else if ((game->tick) == 1)
			{
				shift_down(game, game->fullline_y);
				if (check_fullline(game) == -1)
				{
					game->data -= 0b00010000;
				}
				--game->tick;
			}

			break;
		}
		case GAME_OVER:
		{
			game->data = 0b00000000;

			break;
		}
	}
	return true;
}