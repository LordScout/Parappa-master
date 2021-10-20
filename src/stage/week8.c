/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "week8.h"

#include "../mem.h"
#include "../archive.h"
#include "../random.h"
#include "../timer.h"

//Week 3 background structure
typedef struct
{
	//Stage background base structure
	StageBack back;
	
	//Textures
	Gfx_Tex tex_back0; //Buildings
	Gfx_Tex tex_back1; //Lights
	Gfx_Tex tex_back2; //Rooftop
	Gfx_Tex tex_back3; //Background train arc
	Gfx_Tex tex_back4; //Train
	Gfx_Tex tex_back5; //Sky
	
	//Window state
	u8 win_r, win_g, win_b;
	fixed_t win_time;
	
	//Train state
	fixed_t building_x;
} Back_Week3;

//Week 3 background functions
static const u8 win_cols[][3] = {
	{ 49, 162, 253},
	{ 49, 253, 140},
	{251,  51, 245},
	{253,  69,  49},
	{251, 166,  51},
};

void Back_Week8_Window(Back_Week3 *this)
{
	const u8 *win_col = win_cols[RandomRange(0, COUNT_OF(win_cols) - 1)];
	this->win_r = win_col[0];
	this->win_g = win_col[1];
	this->win_b = win_col[2];
	this->win_time = FIXED_DEC(3,1);
}

void Back_Week8_DrawBG(StageBack *back)
{
	Back_Week3 *this = (Back_Week3*)back;

	
	fixed_t fx, fy;

	fx = stage.camera.x;
	fy = stage.camera.y;

	//Draw arc
	RECT arcl_src = { 0, 0, 251, 44};
	RECT_FIXED arcl_dst = {
		FIXED_DEC(-250,1) - fx,
		FIXED_DEC(43,1) - fy,
		FIXED_DEC(675,1),
		FIXED_DEC(130,1)
	};

	Stage_DrawTex(&this->tex_back2, &arcl_src, &arcl_dst, stage.camera.bzoom);

	static const struct Back_Week3_RoofPiece
	{
		RECT src;
		fixed_t scale;
	} roof_piece[] = {
		{{  0, 0,  16, 256}, FIXED_MUL(FIXED_DEC(3,1) * 7 / 10, FIXED_UNIT + FIXED_DEC(SCREEN_WIDEOADD,2) * 10 / 336)},
		{{ 16, 0,  55, 256}, FIXED_DEC(1,1) * 9 / 10},
		{{ 71, 0, 128, 256}, FIXED_DEC(265,100) * 7 / 10},
		{{199, 0,  55, 256}, FIXED_DEC(1,1) * 9 / 10},
		{{255, 0,   0, 256}, FIXED_DEC(16,1) + FIXED_DEC(SCREEN_WIDEOADD2,1)}
	};

	RECT_FIXED roof_dst = {
		FIXED_DEC(-210,1) - FIXED_DEC(SCREEN_WIDEOADD,2) - fx,
		FIXED_DEC(-1000,1) - fy,
		0,
		FIXED_DEC(700,1)
	};

	const struct Back_Week3_RoofPiece* roof_p = roof_piece;
	for (size_t i = 0; i < COUNT_OF(roof_piece); i++, roof_p++)
	{
		roof_dst.w = roof_p->src.w ? (roof_p->src.w * roof_p->scale) : roof_p->scale;
		Stage_DrawTex(&this->tex_back2, &roof_p->src, &roof_dst, stage.camera.bzoom);
		roof_dst.x += roof_dst.w;
	}
	
	fx = stage.camera.x;
	fy = stage.camera.y;

	RECT building_src = { 0, 0, 255, 67, };
	RECT_FIXED building_dst = {
		FIXED_DEC(-1200,1) - this->building_x,
		FIXED_DEC(-200,1) - fy,
		FIXED_DEC(1000,1),
		FIXED_DEC(300,1)
	};
	while (building_dst.x < FIXED_DEC(320, 1))
	{
		Stage_DrawTex(&this->tex_back4, &building_src, &building_dst, stage.camera.bzoom);
		building_dst.x += building_dst.w;
	}
	this->building_x = (this->building_x - (timer_dt << 8)) % building_dst.w;

	fx = stage.camera.x >> 3;
	fy = stage.camera.y >> 3;

	RECT arcr_src = { 0, 0, 256, 110};
	RECT_FIXED arcr_dst = {
		FIXED_DEC(-250,1) - fx,
		FIXED_DEC(-120,1) - fy,
		FIXED_DEC(512,1),
		FIXED_DEC(220,1)
	};

	Stage_DrawTex(&this->tex_back0, &arcr_src, &arcr_dst, stage.camera.bzoom);
	
	//Draw sky
	fx = stage.camera.x >> 3;
	fy = stage.camera.y >> 3;
	
	RECT sky_src = {0, 0, 256, 171};
	RECT_FIXED sky_dst = {
		FIXED_DEC(-175,1) - FIXED_DEC(SCREEN_WIDEOADD,2) - fx,
		FIXED_DEC(-140,1) - FIXED_DEC(SCREEN_WIDEOADD,4) - fy,
		FIXED_DEC(350,1) + FIXED_DEC(SCREEN_WIDEOADD,1),
		FIXED_DEC(250,1) + FIXED_DEC(SCREEN_WIDEOADD,2)
	};
	
	Stage_DrawTex(&this->tex_back5, &sky_src, &sky_dst, stage.camera.bzoom);
}

void Back_Week8_Free(StageBack *back)
{
	Back_Week3 *this = (Back_Week3*)back;
	
	//Free structure
	Mem_Free(this);
}

StageBack *Back_Week8_New(void)
{
	//Allocate background structure
	Back_Week3 *this = (Back_Week3*)Mem_Alloc(sizeof(Back_Week3));
	if (this == NULL)
		return NULL;
	
	//Set background functions
	this->back.draw_fg = NULL;
	this->back.draw_md = NULL;
	this->back.draw_bg = Back_Week8_DrawBG;
	this->back.free = Back_Week8_Free;
	
	//Load background textures
	IO_Data arc_back = IO_Read("\\WEEK8\\BACK.ARC;1");
	Gfx_LoadTex(&this->tex_back0, Archive_Find(arc_back, "back0.tim"), 0);
	Gfx_LoadTex(&this->tex_back1, Archive_Find(arc_back, "back1.tim"), 0);
	Gfx_LoadTex(&this->tex_back2, Archive_Find(arc_back, "back2.tim"), 0);
	Gfx_LoadTex(&this->tex_back3, Archive_Find(arc_back, "back3.tim"), 0);
	Gfx_LoadTex(&this->tex_back4, Archive_Find(arc_back, "back4.tim"), 0);
	Gfx_LoadTex(&this->tex_back5, Archive_Find(arc_back, "back5.tim"), 0);
	Mem_Free(arc_back);
	
	//Initialize window state
	this->win_time = -1;
	
	//Initialize train state
	this->building_x = -0;
	
	return (StageBack*)this;
}
