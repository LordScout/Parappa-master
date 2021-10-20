/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "week4.h"

#include "../archive.h"
#include "../mem.h"
#include "../stage.h"
#include "../random.h"
#include "../timer.h"
#include "../animation.h"

//Week 4 background structure
typedef struct
{
	//Stage background base structure
	StageBack back;
	
	//Textures
	IO_Data arc_hench, arc_hench_ptr[2];
	
	Gfx_Tex tex_back0; //Foreground limo
	Gfx_Tex tex_back1; //Background limo
	Gfx_Tex tex_back2; //Sunset
	Gfx_Tex tex_back3; //Car
	
	//Car state
	fixed_t car_x;
	fixed_t car_timer;
	
	//Henchmen state
	Gfx_Tex tex_hench;
	u8 hench_frame, hench_tex_id;
	
	Animatable hench_animatable;
} Back_Week4;

//Henchmen animation and rects
static const CharFrame henchmen_frame[10] = {
	{0, {  0,   0,  41,  58}, { 71,  97}}, //0 left 1
	{0, { 42,   0,  42,  60}, { 71,  99}}, //1 left 2
	{0, { 42,   0,  42,  60}, { 71,  99}}, //2 left 2
	{0, { 86,   0,  43,  61}, { 71, 100}}, //3 left 4
	{0, { 86,   0,  43,  61}, { 71, 100}}, //4 left 4
	
	{1, {  0,   0, 43, 61}, { 71, 100}}, //5 right 1
	{1, {  0,   0, 43, 61}, { 71, 100}}, //5 right 1
	{1, {  0,   0, 43, 61}, { 71, 100}}, //5 right 1
	{1, {  0,   0, 43, 61}, { 71, 100}}, //5 right 1
	{1, {  0,   0, 43, 61}, { 71, 100}}, //5 right 1
};

static const Animation henchmen_anim[2] = {
	{2, (const u8[]){0, 1, 2, 3, 4, ASCR_BACK, 1}}, //Left
	{2, (const u8[]){5, 6, 6, 7, 7, 8, 9, ASCR_BACK, 1}}, //Right
};

//Henchmen functions
void Week4_Henchmen_SetFrame(void *user, u8 frame)
{
	Back_Week4 *this = (Back_Week4*)user;
	
	//Check if this is a new frame
	if (frame != this->hench_frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &henchmen_frame[this->hench_frame = frame];
		if (cframe->tex != this->hench_tex_id)
			Gfx_LoadTex(&this->tex_hench, this->arc_hench_ptr[this->hench_tex_id = cframe->tex], 0);
	}
}

void Week4_Henchmen_Draw(Back_Week4 *this, fixed_t x, fixed_t y)
{
	//Draw character
	const CharFrame *cframe = &henchmen_frame[this->hench_frame];
	
	fixed_t ox = x - ((fixed_t)cframe->off[0] << FIXED_SHIFT);
	fixed_t oy = y - ((fixed_t)cframe->off[1] << FIXED_SHIFT);
	
	RECT src = {cframe->src[0], cframe->src[1], cframe->src[2], cframe->src[3]};
	RECT_FIXED dst = {ox, oy, src.w << FIXED_SHIFT, src.h << FIXED_SHIFT};
	Stage_DrawTex(&this->tex_hench, &src, &dst, stage.camera.bzoom);
}

//Week 4 background functions
#define CAR_START_X FIXED_DEC(-500,1)
#define CAR_END_X    FIXED_DEC(500,1)
#define CAR_TIME_A FIXED_DEC(5,1)
#define CAR_TIME_B FIXED_DEC(14,1)

void Back_Week4_DrawFG(StageBack *back)
{
	Back_Week4 *this = (Back_Week4*)back;
	
	fixed_t fx, fy;
	
	//Move car
	this->car_timer -= timer_dt;
	if (this->car_timer <= 0)
	{
		this->car_timer = RandomRange(CAR_TIME_A, CAR_TIME_B);
		this->car_x = CAR_START_X;
	}
	
	if (this->car_x < CAR_END_X)
		this->car_x += timer_dt * 2700;
	
	//Draw car
	fx = stage.camera.x * 4 / 3;
	fy = stage.camera.y * 4 / 3;
	
	RECT car_src = {0, 0, 256, 128};
	RECT_FIXED car_dst = {
		this->car_x - fx,
		FIXED_DEC(1000,1) - fy,
		FIXED_DEC(400,1),
		FIXED_DEC(200,1)
	};
	
	Stage_DrawTex(&this->tex_back3, &car_src, &car_dst, stage.camera.bzoom);
}

void Back_Week4_DrawMD(StageBack *back)
{
	Back_Week4 *this = (Back_Week4*)back;
	
	fixed_t fx, fy;
	
	//Animate and draw henchmen
	fx = stage.camera.x >> 1;
	fy = stage.camera.y >> 1;

	if (stage.flag & STAGE_FLAG_JUST_STEP)
	{
		switch (stage.song_step & 3)
		{
		case 0:
			Animatable_SetAnim(&this->hench_animatable, 0);
			break;
		}
	}
	if (stage.note_scroll >= 0)
	{
		switch (stage.stage_id)
		{
		case StageId_4_1: //Tutorial peace
			if (stage.song_step == 40)
				Animatable_SetAnim(&this->hench_animatable, 1);
			if (stage.song_step > 108 && stage.song_step < 244 && (stage.song_step & 0xF) == 4)
				Animatable_SetAnim(&this->hench_animatable, 1);
			if (stage.song_step > 272 && stage.song_step < 390 && (stage.song_step & 0xF) == 4)
				Animatable_SetAnim(&this->hench_animatable, 1);
			if (stage.song_step > 428 && stage.song_step < 690 && (stage.song_step & 0x1E) == 24)
				Animatable_SetAnim(&this->hench_animatable, 1);
			if (stage.song_step > 760 && stage.song_step < 1012 && (stage.song_step & 0x1E) == 8)
				Animatable_SetAnim(&this->hench_animatable, 1);
			break;
		default:
			break;
		}
	}

	Animatable_Animate(&this->hench_animatable, (void*)this, Week4_Henchmen_SetFrame);

	fx = stage.camera.x;
	fy = stage.camera.y;

	Week4_Henchmen_Draw(this, FIXED_DEC(-108, 1) - fx, FIXED_DEC(80, 1) - fy);
	Week4_Henchmen_Draw(this, FIXED_DEC(-80, 1) - fx, FIXED_DEC(65, 1) - fy);
	Week4_Henchmen_Draw(this, FIXED_DEC(-52, 1) - fx, FIXED_DEC(50, 1) - fy);
	Week4_Henchmen_Draw(this, FIXED_DEC(281, 1) - fx, FIXED_DEC(85, 1) - fy);
	Week4_Henchmen_Draw(this, FIXED_DEC(260, 1) - fx, FIXED_DEC(70, 1) - fy);
	Week4_Henchmen_Draw(this, FIXED_DEC(229, 1) - fx, FIXED_DEC(51, 1) - fy);

	fx = stage.camera.x >> 1;
	fy = stage.camera.y >> 1;

	Week4_Henchmen_Draw(this, FIXED_DEC(20, 1) - fx, FIXED_DEC(60, 1) - fy);
	Week4_Henchmen_Draw(this, FIXED_DEC(65, 1) - fx, FIXED_DEC(60, 1) - fy);
	Week4_Henchmen_Draw(this, FIXED_DEC(110, 1) - fx, FIXED_DEC(60, 1) - fy);

	//Draw foreground limo
	fx = stage.camera.x;
	fy = stage.camera.y;
	
	RECT fglimo_src = {0, 0, 253, 38};
	RECT_FIXED fglimo_dst = {
		FIXED_DEC(-225,1) - fx,
		FIXED_DEC(5,1) - fy,
		FIXED_DEC(525,1),
		FIXED_DEC(76,1)
	};
	
	Stage_DrawTex(&this->tex_back0, &fglimo_src, &fglimo_dst, stage.camera.bzoom);
	fglimo_dst.x += fglimo_dst.w;
	fglimo_dst.y -= (fglimo_dst.h * 22) >> 7;
	fglimo_src.y += 128;
	Stage_DrawTex(&this->tex_back0, &fglimo_src, &fglimo_dst, stage.camera.bzoom);
}

void Back_Week4_DrawBG(StageBack *back)
{
	Back_Week4 *this = (Back_Week4*)back;
	
	fixed_t fx, fy;
	
	//Draw background limo
	//Use same scroll as henchmen
	RECT bglimo_src = {0, 0, 255, 128};
	RECT_FIXED bglimo_dst = {
		FIXED_DEC(-1000,1),
		FIXED_DEC(30,1),
		FIXED_DEC(256,1),
		FIXED_DEC(128,1)
	};
	
	Stage_DrawTex(&this->tex_back1, &bglimo_src, &bglimo_dst, stage.camera.bzoom);
	bglimo_dst.x += bglimo_dst.w;
	bglimo_src.y += 128;
	Stage_DrawTex(&this->tex_back1, &bglimo_src, &bglimo_dst, stage.camera.bzoom);
	
	//Draw sunset
	fx = stage.camera.x >> 1;
	fy = stage.camera.y >> 1;
	
	RECT sunset_src = {0, 0, 256, 168};
	RECT_FIXED sunset_dst = {
		FIXED_DEC(-167 - SCREEN_WIDEOADD2,1) - fx,
		FIXED_DEC(-155,1) - fy,
		FIXED_DEC(380 + SCREEN_WIDEOADD,1),
		FIXED_DEC(320,1)
	};
	
	Stage_DrawTex(&this->tex_back2, &sunset_src, &sunset_dst, stage.camera.bzoom);
}

void Back_Week4_Free(StageBack *back)
{
	Back_Week4 *this = (Back_Week4*)back;
	
	//Free henchmen archive
	Mem_Free(this->arc_hench);
	
	//Free structure
	Mem_Free(this);
}

StageBack *Back_Week4_New(void)
{
	//Allocate background structure
	Back_Week4 *this = (Back_Week4*)Mem_Alloc(sizeof(Back_Week4));
	if (this == NULL)
		return NULL;
	
	//Set background functions
	this->back.draw_fg = Back_Week4_DrawFG;
	this->back.draw_md = Back_Week4_DrawMD;
	this->back.draw_bg = Back_Week4_DrawBG;
	this->back.free = Back_Week4_Free;
	
	//Load background textures
	IO_Data arc_back = IO_Read("\\WEEK4\\BACK.ARC;1");
	Gfx_LoadTex(&this->tex_back0, Archive_Find(arc_back, "back0.tim"), 0);
	Gfx_LoadTex(&this->tex_back1, Archive_Find(arc_back, "back1.tim"), 0);
	Gfx_LoadTex(&this->tex_back2, Archive_Find(arc_back, "back2.tim"), 0);
	Gfx_LoadTex(&this->tex_back3, Archive_Find(arc_back, "back3.tim"), 0);
	Mem_Free(arc_back);
	
	//Load henchmen textures
	this->arc_hench = IO_Read("\\WEEK4\\HENCH.ARC;1");
	this->arc_hench_ptr[0] = Archive_Find(this->arc_hench, "hench0.tim");
	this->arc_hench_ptr[1] = Archive_Find(this->arc_hench, "hench1.tim");
	
	//Initialize car state
	this->car_x = CAR_END_X;
	this->car_timer = RandomRange(CAR_TIME_A, CAR_TIME_B);
	
	//Initialize henchmen state
	Animatable_Init(&this->hench_animatable, henchmen_anim);
	Animatable_SetAnim(&this->hench_animatable, 0);
	this->hench_frame = this->hench_tex_id = 0xFF; //Force art load
	
	return (StageBack*)this;
}
