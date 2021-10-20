/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "mom.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../main.h"
#include "../timer.h"

//Mom character structure
enum
{
	Mom_ArcMain_Idle0,
	Mom_ArcMain_Idle1,
	Mom_ArcMain_Idle2,
	Mom_ArcMain_Left,
	Mom_ArcMain_Left1,
	Mom_ArcMain_Down,
	Mom_ArcMain_Down1,
	Mom_ArcMain_Up,
	Mom_ArcMain_Up1,
	Mom_ArcMain_Right,
	Mom_ArcMain_Right1,

	Mom_Arc_Max,
};

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[Mom_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;

} Char_Mom;

//Mom character definitions
static const CharFrame char_mom_frame[] = {
	{Mom_ArcMain_Idle0, {  0,   0,  94, 154}, { 35, 165}}, //0 idle 1
	{Mom_ArcMain_Idle0, {95,   0,  95, 154}, { 36, 165}}, //1 idle 2
	{Mom_ArcMain_Idle1, {  0,   0,  97, 154}, { 38, 165}}, //2 idle 3
	{Mom_ArcMain_Idle1, { 98,   0,  94, 154}, { 38, 165}}, //3 idle 4
	{Mom_ArcMain_Idle2, {  0,   0,  90, 154}, { 41, 165}}, //4 idle 4}
	{Mom_ArcMain_Idle2, { 92,   0,  90, 154}, { 41, 165}}, //5 idle 4}

	{Mom_ArcMain_Left, {  0,   0, 121, 148}, { 62, 158}}, //6 left 1
	{Mom_ArcMain_Left, {122,   0, 123, 153}, { 65, 163}}, //7 left 1
	{Mom_ArcMain_Left1, {  0,   0, 122, 152}, { 64, 162}}, //8 left 1
	
	{Mom_ArcMain_Down, {  0,   0, 98, 155}, { 42, 167}}, //9 down 1
	{Mom_ArcMain_Down, { 99,   0, 104, 155}, { 48, 167}}, //10 down 2
	{Mom_ArcMain_Down1, {  0,   0, 103, 155}, { 47, 167}}, //11 down 2
	
	{Mom_ArcMain_Up, {  0,   0, 109, 159}, { 43, 170}}, //12 up 1
	{Mom_ArcMain_Up, {111,   0, 110, 163}, { 44, 173}}, //13 up 2
	{Mom_ArcMain_Up1, {  0,   0, 111, 162}, { 45, 172}}, //14 up 2
	
	{Mom_ArcMain_Right, {  0,   0,  119, 153}, { 26, 165}}, //14 right 1
	{Mom_ArcMain_Right, { 120,   0,  119, 152}, { 25, 164}}, //15 right 2
	{Mom_ArcMain_Right1, {  0,   0,  120, 152}, { 25, 164}}, //16 right 2
};

static const Animation char_mom_anim[CharAnim_Max] = {
	{2, (const u8[]){ 2,  1,  0,  0,   1, 2,  3,  4,  5,  5,  5,  4,  3, ASCR_BACK, 1}}, //CharAnim_Idle
	{2, (const u8[]){ 6,  7,  8, ASCR_BACK, 1}},         //CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_LeftAlt
	{2, (const u8[]){ 9,  10,  11, ASCR_BACK, 1}},         //CharAnim_Down
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_DownAlt
	{2, (const u8[]){ 12,  13,  14, ASCR_BACK, 1}},         //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_UpAlt
	{2, (const u8[]){15, 16, 17, ASCR_BACK, 1}},         //CharAnim_Right
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_RightAlt
};

//Mom character functions
void Char_Mom_SetFrame(void *user, u8 frame)
{
	Char_Mom *this = (Char_Mom*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_mom_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_Mom_Tick(Character *character)
{
	Char_Mom *this = (Char_Mom*)character;
	
	//Perform idle dance
	if ((character->pad_held & (INPUT_LEFT | INPUT_DOWN | INPUT_UP | INPUT_RIGHT)) == 0)
		Character_PerformIdle(character);
	
	//Animate and draw
	Animatable_Animate(&character->animatable, (void*)this, Char_Mom_SetFrame);
	Character_Draw(character, &this->tex, &char_mom_frame[this->frame]);
	
}

void Char_Mom_SetAnim(Character *character, u8 anim)
{
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Character_CheckStartSing(character);
}

void Char_Mom_Free(Character *character)
{
	Char_Mom *this = (Char_Mom*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_Mom_New(fixed_t x, fixed_t y)
{
	//Allocate mom object
	Char_Mom *this = Mem_Alloc(sizeof(Char_Mom));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_Mom_New] Failed to allocate mom object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_Mom_Tick;
	this->character.set_anim = Char_Mom_SetAnim;
	this->character.free = Char_Mom_Free;
	
	Animatable_Init(&this->character.animatable, char_mom_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = 0;
	
	this->character.health_i = 8;
	
	this->character.focus_x = FIXED_DEC(65,1);
	this->character.focus_y = FIXED_DEC(-115,1);
	this->character.focus_zoom = FIXED_DEC(1,1);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\MOM.ARC;1");
	
	const char **pathp = (const char *[]){
		"idle0.tim", //Mom_ArcMain_Idle0
		"idle1.tim", //Mom_ArcMain_Idle1
		"idle2.tim",
		"left.tim",  //Mom_ArcMain_Left
		"left1.tim",
		"down.tim",  //Mom_ArcMain_Down
		"down1.tim",
		"up.tim",    //Mom_ArcMain_Up
		"up1.tim",
		"right.tim", //Mom_ArcMain_Right
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
