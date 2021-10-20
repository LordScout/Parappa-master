/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "flea.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../main.h"

//Dad character structure
enum
{
	Flea_ArcMain_Idle0,
	Flea_ArcMain_Idle1,
	Flea_ArcMain_Idle2,
	Flea_ArcMain_Left,
	Flea_ArcMain_Left1,
	Flea_ArcMain_Down,
	Flea_ArcMain_Down1,
	Flea_ArcMain_Up,
	Flea_ArcMain_Up1,
	Flea_ArcMain_Right,
	Flea_ArcMain_Right1,
	
	Flea_Arc_Max,
};

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[Flea_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;
} Char_Dad;

//Dad character definitions
static const CharFrame char_flea_frame[] = {
	{Flea_ArcMain_Idle0, {  0,   0, 93, 169}, { 45, 166}}, //0 idle 1
	{Flea_ArcMain_Idle0, { 94,   0, 92, 168}, { 44, 166}}, //1 idle 2
	{Flea_ArcMain_Idle1, {  0,   0, 93, 161}, { 45, 158}}, //2 idle 3
	{Flea_ArcMain_Idle1, { 94,   0, 92, 164}, { 44, 160}}, //3 idle 4
	{Flea_ArcMain_Idle2, {  0,   0, 93, 166}, { 45, 162}}, //4 idle 3
	{Flea_ArcMain_Idle2, { 94,   0, 92, 168}, { 44, 164}}, //5 idle 4
	
	{Flea_ArcMain_Left, {  0,   0, 105, 167}, { 52, 163}}, //6 left 1
	{Flea_ArcMain_Left, { 106,   0, 104, 166}, { 51, 162}}, //7 left 2
	{Flea_ArcMain_Left1, {  0,   0, 105, 166}, { 52, 163}}, //8 left 1
	{Flea_ArcMain_Left1, { 106,   0, 104, 166}, { 51, 162}}, //9 left 2
	
	{Flea_ArcMain_Down, {  0,   0, 106, 149}, { 54, 139}}, //10 down 1
	{Flea_ArcMain_Down, {107,   0, 106, 151}, { 54, 141}}, //11 down 2
	{Flea_ArcMain_Down1, {  0,   0, 106, 149}, { 54, 139}}, //12 down 1
	{Flea_ArcMain_Down1, {107,   0, 106, 151}, { 54, 141}}, //13 down 2
	
	{Flea_ArcMain_Up, {  0,   0, 99, 177}, { 46, 173}}, //14 up 1
	{Flea_ArcMain_Up, {100,   0, 100, 176}, { 46, 172}}, //15 up 2
	{Flea_ArcMain_Up1, {  0,   0, 99, 176}, { 46, 173}}, //16 up 1
	{Flea_ArcMain_Up1, {100,   0, 100, 175}, { 46, 172}}, //17 up 2
	
	{Flea_ArcMain_Right, {  0,   0, 95, 162}, { 44, 159}}, //18 right 1
	{Flea_ArcMain_Right, { 96,   0, 93, 163}, { 44, 160}}, //19 right 2
	{Flea_ArcMain_Right1, {  0,   0, 94, 162}, { 44, 159}}, //20 right 1
	{Flea_ArcMain_Right1, {95,   0,  92, 163}, { 44, 160}}, //21 right 2
};

static const Animation char_flea_anim[CharAnim_Max] = {
	{2, (const u8[]){ 1,  2,  3,  4,  5,  0, ASCR_BACK, 1}}, //CharAnim_Idle
	{2, (const u8[]){ 6,  7, ASCR_BACK, 1}},         //CharAnim_Left
	{2, (const u8[]){ 8,  9, ASCR_BACK, 1}},   //CharAnim_LeftAlt
	{2, (const u8[]){ 10,  11, ASCR_BACK, 1}},         //CharAnim_Down
	{2, (const u8[]){ 12,  13, ASCR_BACK, 1}},   //CharAnim_DownAlt
	{2, (const u8[]){ 14,  15, ASCR_BACK, 1}},         //CharAnim_Up
	{2, (const u8[]){ 16,  17, ASCR_BACK, 1}},   //CharAnim_UpAlt
	{2, (const u8[]){ 18, 19, ASCR_BACK, 1}},         //CharAnim_Right
	{2, (const u8[]){ 20, 21, ASCR_BACK, 1}},   //CharAnim_RightAlt
};

//Dad character functions
void Char_Flea_SetFrame(void *user, u8 frame)
{
	Char_Dad *this = (Char_Dad*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_flea_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_Flea_Tick(Character *character)
{
	Char_Dad *this = (Char_Dad*)character;
	
	//Perform idle dance
	if ((character->pad_held & (INPUT_LEFT | INPUT_DOWN | INPUT_UP | INPUT_RIGHT)) == 0)
		Character_PerformIdle(character);
	
	//Animate and draw
	Animatable_Animate(&character->animatable, (void*)this, Char_Flea_SetFrame);
	Character_Draw(character, &this->tex, &char_flea_frame[this->frame]);
}

void Char_Flea_SetAnim(Character *character, u8 anim)
{
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Character_CheckStartSing(character);
}

void Char_Flea_Free(Character *character)
{
	Char_Dad *this = (Char_Dad*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_Flea_New(fixed_t x, fixed_t y)
{
	//Allocate dad object
	Char_Dad *this = Mem_Alloc(sizeof(Char_Dad));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_Dad_New] Failed to allocate dad object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_Flea_Tick;
	this->character.set_anim = Char_Flea_SetAnim;
	this->character.free = Char_Flea_Free;
	
	Animatable_Init(&this->character.animatable, char_flea_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = 0;
	
	this->character.health_i = 9;
	
	this->character.focus_x = FIXED_DEC(65,1);
	this->character.focus_y = FIXED_DEC(-85,1);
	this->character.focus_zoom = FIXED_DEC(1,1);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\FLEA.ARC;1");
	
	const char **pathp = (const char *[]){
		"idle0.tim", //Dad_ArcMain_Idle0
		"idle1.tim", //Dad_ArcMain_Idle1
		"idle2.tim",
		"left.tim",  //Dad_ArcMain_Left
		"left1.tim",
		"down.tim",  //Dad_ArcMain_Down
		"down1.tim",
		"up.tim",    //Dad_ArcMain_Up
		"up1.tim",
		"right.tim", //Dad_ArcMain_Right
		"right1.tim",
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	return (Character*)this;
}
