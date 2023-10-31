#pragma once
#include "pch.h"

#ifdef TETRISDLL_EXPORTS
#define TETRISDLL_API __declspec(dllexport)
#else
#define TETRISDLL_API __declspec(dllimport)
#endif

typedef unsigned char byte;

//creates and returns game instance
extern "C" TETRISDLL_API void* init(short w, short h);

//destroys game instance (deletes all used data)
extern "C" TETRISDLL_API void destroy(void* game_ptr);

//returns current game state
extern "C" TETRISDLL_API byte get_gs(void* game_ptr);

//returns next figure type
extern "C" TETRISDLL_API byte get_nf(void* game_ptr);

//returns cell state by coordinate
extern "C" TETRISDLL_API bool get_cell_state(void* game_ptr, short x, short y);

//does a tick
extern "C" TETRISDLL_API bool tick(void* game_ptr, byte input);

#ifdef _DEBUG
extern "C" TETRISDLL_API void set_data(void* game_ptr, bool data, short x, short y);
#endif
