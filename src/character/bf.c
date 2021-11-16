/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "bf.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../random.h"
#include "../main.h"

//Boyfriend skull fragments
static SkullFragment char_bf_skull[15] = {
	{ 1 * 8, -87 * 8, -13, -13},
	{ 9 * 8, -88 * 8,   5, -22},
	{18 * 8, -87 * 8,   9, -22},
	{26 * 8, -85 * 8,  13, -13},
	
	{-3 * 8, -82 * 8, -13, -11},
	{ 8 * 8, -85 * 8,  -9, -15},
	{20 * 8, -82 * 8,   9, -15},
	{30 * 8, -79 * 8,  13, -11},
	
	{-1 * 8, -74 * 8, -13, -5},
	{ 8 * 8, -77 * 8,  -9, -9},
	{19 * 8, -75 * 8,   9, -9},
	{26 * 8, -74 * 8,  13, -5},
	
	{ 5 * 8, -73 * 8, -5, -3},
	{14 * 8, -76 * 8,  9, -6},
	{26 * 8, -67 * 8, 15, -3},
};

//Boyfriend player types
enum
{
	BF_ArcMain_BF0,
	BF_ArcMain_BF1,
	BF_ArcMain_BF2,
	BF_ArcMain_BF3,
	BF_ArcMain_BF4,
	BF_ArcMain_BF5,
	BF_ArcMain_BF6,
	BF_ArcDead_Dead0, //Mic Drop

	BF_ArcScene_0, //ugh0 good0
	BF_ArcScene_1, //ugh1 good1

	BF_ArcMain_Max,
};

enum
{
	BF_ArcDead_Dead1, //Twitch
	BF_ArcDead_Dead2, //Retry prompt
	
	BF_ArcDead_Max,
};

#define BF_Arc_Max BF_ArcMain_Max

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main, arc_scene, arc_dead;
	CdlFILE file_dead_arc; //dead.arc file position
	IO_Data arc_ptr[BF_Arc_Max];
	
	Gfx_Tex tex, tex_retry;
	u8 frame, tex_id;
	
	u8 retry_bump;
	
	SkullFragment skull[COUNT_OF(char_bf_skull)];
	u8 skull_scale;
} Char_BF;

//Boyfriend player definitions
static const CharFrame char_bf_frame[] = {
	{BF_ArcMain_BF0, {  0,   0, 87,  104}, { 54,  90}}, //0 idle 1
	{BF_ArcMain_BF0, {88,   0, 89,  102}, { 56,  89}}, //1 idle 2
	{BF_ArcMain_BF0, {  0, 105, 90, 105}, { 58,  91}}, //2 idle 3
	{BF_ArcMain_BF0, {91, 105, 91, 105}, { 59,  91}}, //3 idle 4
	{BF_ArcMain_BF1, {  0,   1, 90, 106}, { 58,  92}}, //4 idle 5
	
	{BF_ArcMain_BF1, {92,   0,  75, 108}, { 48,  93}}, //5 left 1
	{BF_ArcMain_BF1, {  0, 108,  77, 106}, { 50,  91}}, //6 left 2
	
	{BF_ArcMain_BF1, { 78, 110,  83, 101}, { 47,  86}}, //7 down 1
	{BF_ArcMain_BF2, {  0,   0,  84, 102}, { 48,  87}}, //8 down 2
	
	{BF_ArcMain_BF2, { 85,   0,  75, 108}, { 39, 94}}, //9 up 1
	{BF_ArcMain_BF2, {  1,  104,  74, 107}, { 40, 93}}, //10 up 2
	
	{BF_ArcMain_BF2, { 76, 109, 80, 102}, { 41,  88}}, //11 right 1
	{BF_ArcMain_BF3, {  0,   0, 80, 104}, { 41,  90}}, //12 right 2
	
	{BF_ArcMain_BF3, {81,   0,  82, 101}, { 52,  87}}, //13 peace 1
	{BF_ArcMain_BF3, {  0, 106, 95, 107}, { 51,  93}}, //14 peace 2
	{BF_ArcMain_BF3, {96, 106, 98, 105}, { 52,  91}}, //15 peace 3
	
	{BF_ArcMain_BF4, {  0,   0,  75, 104}, { 43,  90}}, //16 sweat 1
	{BF_ArcMain_BF4, { 76,   0,  75, 103}, { 43,  89}}, //17 sweat 2
	{BF_ArcMain_BF4, {  0,   0,  75, 104}, { 43,  90}}, //18 sweat 1
	{BF_ArcMain_BF4, { 76,   0,  75, 103}, { 43,  89}}, //19 sweat 2
	
	{BF_ArcMain_BF5, {  0,   1,  76, 107}, { 48, 92}}, //20 left miss 1
	{BF_ArcMain_BF5, { 77,   0,  77, 108}, { 49, 93}}, //21 left miss 2
	
	{BF_ArcMain_BF5, {  0, 110,  79,  96}, { 48,  81}}, //22 down miss 1
	{BF_ArcMain_BF5, { 82, 109,  95,  97}, { 48,  81}}, //23 down miss 2
	
	{BF_ArcMain_BF6, {  0,   0,  75, 102}, { 40,  89}}, //24 up miss 1
	{BF_ArcMain_BF6, { 76,   0,  75, 105}, { 40, 92}}, //25 up miss 2
	
	{BF_ArcMain_BF6, {  0, 104,  78, 104}, { 39, 90}}, //26 right miss 1
	{BF_ArcMain_BF6, {79, 105, 81, 104}, { 39, 91}}, //27 right miss 2

	{BF_ArcScene_0, {  0,   0, 101, 103}, { 52,  95}}, //28 ugh
	{BF_ArcScene_0, {105,   0, 102, 104}, { 50,  96}}, //29 ugh
	{BF_ArcScene_1, {  0,   0, 101, 103}, { 52,  95}}, //30 ugh
	{BF_ArcScene_1, {103,   0, 101, 103}, { 52,  95}}, //31 ugh

	{BF_ArcDead_Dead0, {  0,   0, 208, 139}, { 160,  140}}, //32 dead0 0
	{BF_ArcDead_Dead0, {  0,   0, 208, 139}, { 160,  140}}, //32 dead0 1
	{BF_ArcDead_Dead0, {  0,   0, 208, 139}, { 160,  140}}, //32 dead0 2
	{BF_ArcDead_Dead0, {  0,   0, 208, 139}, { 160,  140}}, //32 dead0 0

	{BF_ArcDead_Dead1, {  0,   0, 208, 139}, { 160,  140}}, //36 dead1 0
	{BF_ArcDead_Dead1, {  0,   0, 208, 139}, { 160,  140}}, //36 dead1 0
	{BF_ArcDead_Dead1, {  0,   0, 208, 139}, { 160,  140}}, //36 dead1 0
	{BF_ArcDead_Dead1, {  0,   0, 208, 139}, { 160,  140}}, //36 dead1 0

	{BF_ArcDead_Dead2, {  0,   0, 208, 139}, { 160,  140}}, //40 dead2 body twitch 0
	{BF_ArcDead_Dead2, {  0,   0, 208, 139}, { 160,  140}}, //40 dead2 body twitch 0
	{BF_ArcDead_Dead2, {  0,   0, 208, 139}, { 160,  140}}, //40 dead2 body twitch 0
	{BF_ArcDead_Dead2, {  0,   0, 208, 139}, { 160,  140}}, //40 dead2 body twitch 0
};

static const Animation char_bf_anim[PlayerAnim_Max] = {
	{2, (const u8[]){ 0,  1,  2,  3,  4, ASCR_BACK, 1}}, //CharAnim_Idle
	{2, (const u8[]){ 5,  6, ASCR_BACK, 1}},             //CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //CharAnim_LeftAlt
	{2, (const u8[]){ 7,  8, ASCR_BACK, 1}},             //CharAnim_Down
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //CharAnim_DownAlt
	{2, (const u8[]){ 9, 10, ASCR_BACK, 1}},             //CharAnim_Up
	{2, (const u8[]){16, 17, 18, 19, ASCR_BACK, 1}}, //CharAnim_UpAlt
	{2, (const u8[]){11, 12, ASCR_BACK, 1}},             //CharAnim_Right
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //CharAnim_RightAlt
	
	{1, (const u8[]){ 5, 20, 20, 21, ASCR_BACK, 1}},     //PlayerAnim_LeftMiss
	{1, (const u8[]){ 7, 22, 22, 23, ASCR_BACK, 1}},     //PlayerAnim_DownMiss
	{1, (const u8[]){ 9, 24, 24, 25, ASCR_BACK, 1}},     //PlayerAnim_UpMiss
	{1, (const u8[]){11, 26, 26, 27, ASCR_BACK, 1}},     //PlayerAnim_RightMiss
	
	{2, (const u8[]){13, 14, 15, ASCR_BACK, 1}},         //PlayerAnim_Peace
	{2, (const u8[]){16, 17, 18, 19, ASCR_REPEAT}},      //PlayerAnim_Sweat
	
	{5, (const u8[]) { 32, 33, 34, 35, 35, 35, 35, 35, 35, 35, ASCR_CHGANI, PlayerAnim_Dead1 }}, //PlayerAnim_Dead0
	{5, (const u8[]) { 35, ASCR_REPEAT }},                                                       //PlayerAnim_Dead1
	{3, (const u8[]) {39, 39, 39, 39, 39, 39, 39, ASCR_CHGANI, PlayerAnim_Dead3 }}, //PlayerAnim_Dead2
	{3, (const u8[]) { 39, ASCR_REPEAT }},                                                       //PlayerAnim_Dead3
	{3, (const u8[]) {39, 39, 39, 39, 39, ASCR_CHGANI, PlayerAnim_Dead3 }},             //PlayerAnim_Dead4
	{3, (const u8[]) {39, 39, 39, 39, 39, ASCR_CHGANI, PlayerAnim_Dead3 }},             //PlayerAnim_Dead5

	{10, (const u8[]) { 39, 39, 39, ASCR_BACK, 1 }}, //PlayerAnim_Dead4
	{ 3, (const u8[]) { 42, 43, 39, ASCR_REPEAT }},  //PlayerAnim_Dead5
};

//Boyfriend player functions
void Char_BF_SetFrame(void *user, u8 frame)
{
	Char_BF *this = (Char_BF*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_bf_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_BF_Tick(Character *character)
{
	Char_BF *this = (Char_BF*)character;
	
	if ((stage.stage_id == StageId_4_1))
	{
		this->character.focus_x = FIXED_DEC(-65, 1);
		this->character.focus_y = FIXED_DEC(-105, 1);
		this->character.focus_zoom = FIXED_DEC(1, 1);
	}

	if ((stage.stage_id == StageId_4_3))
	{
		this->character.focus_x = FIXED_DEC(-65, 1);
		this->character.focus_y = FIXED_DEC(-65, 1);
		this->character.focus_zoom = FIXED_DEC(1, 1);
	}

	if ((stage.stage_id == StageId_5_1))
	{
		this->character.focus_x = FIXED_DEC(-55, 1);
		this->character.focus_y = FIXED_DEC(-75, 1);
		this->character.focus_zoom = FIXED_DEC(10, 10);
	}

	if ((stage.stage_id == StageId_5_1) && stage.song_step >= 296)
	{
		this->character.focus_x = FIXED_DEC(-55, 1);
		this->character.focus_y = FIXED_DEC(-65, 1);
		this->character.focus_zoom = FIXED_DEC(12, 10);
	}

	if ((stage.stage_id == StageId_5_1) && stage.song_step >= 310)
	{
		this->character.focus_x = FIXED_DEC(-45, 1);
		this->character.focus_y = FIXED_DEC(-65, 1);
		this->character.focus_zoom = FIXED_DEC(14, 10);
	}

	if ((stage.stage_id == StageId_5_1) && stage.song_step >= 316)
	{
		this->character.focus_x = FIXED_DEC(-38, 1);
		this->character.focus_y = FIXED_DEC(-65, 1);
		this->character.focus_zoom = FIXED_DEC(16, 10);
	}

	if ((stage.stage_id == StageId_5_1) && stage.song_step >= 324)
	{
		this->character.focus_x = FIXED_DEC(-32, 1);
		this->character.focus_y = FIXED_DEC(-65, 1);
		this->character.focus_zoom = FIXED_DEC(20, 10);
	}

	if ((stage.stage_id == StageId_5_1) && stage.song_step >= 328)
	{
		this->character.focus_x = FIXED_DEC(-55, 1);
		this->character.focus_y = FIXED_DEC(-75, 1);
		this->character.focus_zoom = FIXED_DEC(10, 10);
	}

	//Handle animation updates
	if ((character->pad_held & (INPUT_LEFT | INPUT_DOWN | INPUT_UP | INPUT_RIGHT)) == 0 ||
	    (character->animatable.anim != CharAnim_Left &&
	     character->animatable.anim != CharAnim_LeftAlt &&
	     character->animatable.anim != CharAnim_Down &&
	     character->animatable.anim != CharAnim_DownAlt &&
	     character->animatable.anim != CharAnim_Up &&
	     character->animatable.anim != CharAnim_UpAlt &&
	     character->animatable.anim != CharAnim_Right &&
	     character->animatable.anim != CharAnim_RightAlt))
		Character_CheckEndSing(character);
	
	if (stage.flag & STAGE_FLAG_JUST_STEP)
	{
		//Perform idle dance
		if (Animatable_Ended(&character->animatable) &&
			(character->animatable.anim != CharAnim_Left &&
		     character->animatable.anim != CharAnim_LeftAlt &&
		     character->animatable.anim != PlayerAnim_LeftMiss &&
		     character->animatable.anim != CharAnim_Down &&
		     character->animatable.anim != CharAnim_DownAlt &&
		     character->animatable.anim != PlayerAnim_DownMiss &&
		     character->animatable.anim != CharAnim_Up &&
		     character->animatable.anim != CharAnim_UpAlt &&
		     character->animatable.anim != PlayerAnim_UpMiss &&
		     character->animatable.anim != CharAnim_Right &&
		     character->animatable.anim != CharAnim_RightAlt &&
		     character->animatable.anim != PlayerAnim_RightMiss) &&
			(stage.song_step & 0x7) == 0)
			character->set_anim(character, CharAnim_Idle);
		
		//Stage specific animations
		if (stage.note_scroll >= 0)
		{
			switch (stage.stage_id)
			{
				case StageId_1_4: //Tutorial peace
					if (stage.song_step > 64 && stage.song_step < 192 && (stage.song_step & 0x3F) == 60)
						character->set_anim(character, PlayerAnim_Peace);
					break;
				case StageId_1_1: //Bopeebo peace
					if ((stage.song_step & 0x1F) == 28)
						character->set_anim(character, PlayerAnim_Peace);
					break;
				case StageId_4_2: //Car Rap
					if ((stage.song_step) == 348)
						character->set_anim(character, PlayerAnim_Peace);
					if ((stage.song_step) == 412)
						character->set_anim(character, PlayerAnim_Peace);
					if ((stage.song_step) == 600)
						character->set_anim(character, PlayerAnim_Peace);
					break;
				case StageId_5_1: //Court Rap
					if ((stage.song_step) == 302)
						character->set_anim(character, CharAnim_UpAlt);
					if ((stage.song_step) == 304)
						character->set_anim(character, CharAnim_UpAlt);
					if ((stage.song_step) == 308)
						character->set_anim(character, CharAnim_UpAlt);
					if ((stage.song_step) == 312)
						character->set_anim(character, CharAnim_UpAlt);
					if ((stage.song_step) == 316)
						character->set_anim(character, CharAnim_UpAlt);
					if ((stage.song_step) == 320)
						character->set_anim(character, CharAnim_UpAlt);
					if ((stage.song_step) == 324)
						character->set_anim(character, CharAnim_UpAlt);
					if ((stage.song_step) == 328)
						character->set_anim(character, CharAnim_UpAlt);
					if ((stage.song_step) == 474)
						character->set_anim(character, PlayerAnim_Peace);
					break;
				default:
					break;
			}
		}
	}
	
	//Retry screen
	if (character->animatable.anim >= PlayerAnim_Dead3)
	{
		//Tick skull fragments
		if (this->skull_scale)
		{
			SkullFragment *frag = this->skull;
			for (size_t i = 0; i < COUNT_OF_MEMBER(Char_BF, skull); i++, frag++)
			{
				//Draw fragment
				RECT frag_src = {
					(i & 1) ? 112 : 96,
					(i >> 1) << 4,
					16,
					16
				};
				fixed_t skull_dim = (FIXED_DEC(16,1) * this->skull_scale) >> 6;
				fixed_t skull_rad = skull_dim >> 1;
				RECT_FIXED frag_dst = {
					character->x + (((fixed_t)frag->x << FIXED_SHIFT) >> 3) - skull_rad - stage.camera.x,
					character->y + (((fixed_t)frag->y << FIXED_SHIFT) >> 3) - skull_rad - stage.camera.y,
					skull_dim,
					skull_dim,
				};
				Stage_DrawTex(&this->tex_retry, &frag_src, &frag_dst, FIXED_MUL(stage.camera.zoom, stage.bump));
				
				//Move fragment
				frag->x += frag->xsp;
				frag->y += ++frag->ysp;
			}
			
			//Decrease scale
			this->skull_scale--;
		}
		
		//Draw input options
		u8 input_scale = 16 - this->skull_scale;
		if (input_scale > 16)
			input_scale = 0;
		
		RECT button_src = {
			 0, 96,
			16, 16
		};
		RECT_FIXED button_dst = {
			character->x - FIXED_DEC(32,1) - stage.camera.x,
			character->y - FIXED_DEC(88,1) - stage.camera.y,
			(FIXED_DEC(16,1) * input_scale) >> 4,
			FIXED_DEC(16,1),
		};
		
		//Cross - Retry
		Stage_DrawTex(&this->tex_retry, &button_src, &button_dst, FIXED_MUL(stage.camera.zoom, stage.bump));
		
		//Circle - Blueball
		button_src.x = 16;
		button_dst.y += FIXED_DEC(56,1);
		Stage_DrawTex(&this->tex_retry, &button_src, &button_dst, FIXED_MUL(stage.camera.zoom, stage.bump));
		
		//Draw 'RETRY'
		u8 retry_frame;
		
		if (character->animatable.anim == PlayerAnim_Dead6)
		{
			//Selected retry
			retry_frame = 2 - (this->retry_bump >> 3);
			if (retry_frame >= 3)
				retry_frame = 0;
			if (this->retry_bump & 2)
				retry_frame += 3;
			
			if (++this->retry_bump == 0xFF)
				this->retry_bump = 0xFD;
		}
		else
		{
			//Idle
			retry_frame = 1 +  (this->retry_bump >> 2);
			if (retry_frame >= 3)
				retry_frame = 0;
			
			if (++this->retry_bump >= 55)
				this->retry_bump = 0;
		}
		
		RECT retry_src = {
			(retry_frame & 1) ? 48 : 0,
			(retry_frame >> 1) << 5,
			48,
			32
		};
		RECT_FIXED retry_dst = {
			character->x -  FIXED_DEC(7,1) - stage.camera.x,
			character->y - FIXED_DEC(92,1) - stage.camera.y,
			FIXED_DEC(48,1),
			FIXED_DEC(32,1),
		};
		Stage_DrawTex(&this->tex_retry, &retry_src, &retry_dst, FIXED_MUL(stage.camera.zoom, stage.bump));
	}
	
	//Animate and draw character
	Animatable_Animate(&character->animatable, (void*)this, Char_BF_SetFrame);
	Character_Draw(character, &this->tex, &char_bf_frame[this->frame]);
}

void Char_BF_SetAnim(Character *character, u8 anim)
{
	Char_BF *this = (Char_BF*)character;
	
	//Perform animation checks
	switch (anim)
	{
		case PlayerAnim_Dead0:
			//Begin reading dead.arc and adjust focus
			this->arc_dead = IO_AsyncReadFile(&this->file_dead_arc);
			this->character.focus_x = FIXED_DEC(-55, 1);
			this->character.focus_y = FIXED_DEC(-65, 1);
			this->character.focus_zoom = FIXED_DEC(1, 1);
			break;
		case PlayerAnim_Dead2:
			//Unload main.arc
			Mem_Free(this->arc_main);
			Mem_Free(this->arc_scene);
			this->arc_main = this->arc_dead;
			this->arc_dead = NULL;
			this->arc_scene = NULL;
			
			//Find dead.arc files
			const char **pathp = (const char *[]){
				"dead1.tim", //BF_ArcDead_Dead2
				"retry.tim", //BF_ArcDead_Retry
				NULL
			};
			IO_Data *arc_ptr = this->arc_ptr;
			for (; *pathp != NULL; pathp++)
				*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
			
			//Load retry art
			Gfx_LoadTex(&this->tex_retry, this->arc_ptr[BF_ArcDead_Dead2], 0);
			break;
	}
	
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Character_CheckStartSing(character);
}

void Char_BF_Free(Character *character)
{
	Char_BF *this = (Char_BF*)character;
	
	//Free art
	Mem_Free(this->arc_main);
	Mem_Free(this->arc_dead);
	Mem_Free(this->arc_scene);
}

Character *Char_BF_New(fixed_t x, fixed_t y)
{
	//Allocate boyfriend object
	Char_BF *this = Mem_Alloc(sizeof(Char_BF));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_BF_New] Failed to allocate boyfriend object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_BF_Tick;
	this->character.set_anim = Char_BF_SetAnim;
	this->character.free = Char_BF_Free;
	
	Animatable_Init(&this->character.animatable, char_bf_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = CHAR_SPEC_MISSANIM;
	
	this->character.health_i = 0;
	
	this->character.focus_x = FIXED_DEC(-55,1);
	this->character.focus_y = FIXED_DEC(-65,1);
	this->character.focus_zoom = FIXED_DEC(1,1);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\BF.ARC;1");
	this->arc_dead = NULL;
	IO_FindFile(&this->file_dead_arc, "\\CHAR\\BFDEAD.ARC;1");
	
	const char **pathp = (const char *[]){
		"bf0.tim",   //BF_ArcMain_BF0
		"bf1.tim",   //BF_ArcMain_BF1
		"bf2.tim",   //BF_ArcMain_BF2
		"bf3.tim",   //BF_ArcMain_BF3
		"bf4.tim",   //BF_ArcMain_BF4
		"bf5.tim",   //BF_ArcMain_BF5
		"bf6.tim",   //BF_ArcMain_BF6
		"dead0.tim", //BF_ArcMain_Dead0
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	//Initialize player state
	this->retry_bump = 0;
	
	//Copy skull fragments
	memcpy(this->skull, char_bf_skull, sizeof(char_bf_skull));
	this->skull_scale = 64;
	
	SkullFragment *frag = this->skull;
	for (size_t i = 0; i < COUNT_OF_MEMBER(Char_BF, skull); i++, frag++)
	{
		//Randomize trajectory
		frag->xsp += RandomRange(-4, 4);
		frag->ysp += RandomRange(-2, 2);
	}
	
	return (Character*)this;
}
