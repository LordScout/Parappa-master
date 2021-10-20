/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "dad.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../main.h"

//Dad character structure
enum
{
	Master_ArcMain_Idle0,
	Master_ArcMain_Idle1,
	Master_ArcMain_Idle2,
	Master_ArcMain_Left,
	Master_ArcMain_Left1,
	Master_ArcMain_Down,
	Master_ArcMain_Down1,
	Master_ArcMain_Up,
	Master_ArcMain_Up1,
	Master_ArcMain_Right,
	Master_ArcMain_Right1,
	
	Master_Arc_Max,
};

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[Master_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;
} Char_Dad;

//Dad character definitions
static const CharFrame char_master_frame[] = {
	{Master_ArcMain_Idle0, {  0,   0, 87, 187}, { 42, 183}}, //0 idle 1
	{Master_ArcMain_Idle0, { 88,   0, 89, 187}, { 42, 183}}, //1 idle 2
	{Master_ArcMain_Idle1, {  0,   0, 92, 186}, { 44, 181}}, //2 idle 3
	{Master_ArcMain_Idle1, { 95,   0, 89, 187}, { 42, 182}}, //3 idle 4
	{Master_ArcMain_Idle2, {  0,   0, 87, 188}, { 42, 183}}, //4 idle 4
	
	{Master_ArcMain_Left, {  0,   0,  90, 186}, { 48, 182}}, //5 left 1
	{Master_ArcMain_Left, { 91,   0,  93, 187}, { 49, 183}}, //6 left 2
	{Master_ArcMain_Left1, {  0,   0,  94, 187}, { 50, 183}}, //7 left 2
	
	{Master_ArcMain_Down, {  0,   0, 89, 179}, { 42, 175}}, //8 down 1
	{Master_ArcMain_Down, { 90,   0,  88, 180}, { 41, 176}}, //9 down 2
	{Master_ArcMain_Down1, {  0,   0,  88, 181}, { 41, 177}}, //10 down 2
	
	{Master_ArcMain_Up, {  0,   0, 110, 190}, { 62, 186}}, //11 up 1
	{Master_ArcMain_Up, {111,   0, 111, 189}, { 61, 185}}, //12 up 2
	{Master_ArcMain_Up1, {  0,   0, 111, 188}, { 61, 184}}, //13 up 2
	
	{Master_ArcMain_Right, {  0,   0,  93, 184}, { 44, 180}}, //14 right 1
	{Master_ArcMain_Right, { 94,   0,  92, 185}, { 42, 181}}, //15 right 2
	{Master_ArcMain_Right1, {  0,   0,  92, 185}, { 42, 181}}, //16 right 2
};

static const Animation char_master_anim[CharAnim_Max] = {
	{2, (const u8[]){ 1,  2,  3,  4,  0, ASCR_BACK, 1}}, //CharAnim_Idle
	{2, (const u8[]){ 5,  6,  7, ASCR_BACK, 1}},         //CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_LeftAlt
	{2, (const u8[]){ 8,  9,  10, ASCR_BACK, 1}},         //CharAnim_Down
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_DownAlt
	{2, (const u8[]){ 11,  12,  13, ASCR_BACK, 1}},         //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_UpAlt
	{2, (const u8[]){14, 15,  16, ASCR_BACK, 1}},         //CharAnim_Right
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_RightAlt
};

//Dad character functions
void Char_Master_SetFrame(void *user, u8 frame)
{
	Char_Dad *this = (Char_Dad*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_master_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_Master_Tick(Character *character)
{
	Char_Dad *this = (Char_Dad*)character;
	
	//Perform idle dance
	if ((character->pad_held & (INPUT_LEFT | INPUT_DOWN | INPUT_UP | INPUT_RIGHT)) == 0)
		Character_PerformIdle(character);
	
	//Animate and draw
	Animatable_Animate(&character->animatable, (void*)this, Char_Master_SetFrame);
	Character_Draw(character, &this->tex, &char_master_frame[this->frame]);
}

void Char_Master_SetAnim(Character *character, u8 anim)
{
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Character_CheckStartSing(character);
}

void Char_Master_Free(Character *character)
{
	Char_Dad *this = (Char_Dad*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_Master_New(fixed_t x, fixed_t y)
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
	this->character.tick = Char_Master_Tick;
	this->character.set_anim = Char_Master_SetAnim;
	this->character.free = Char_Master_Free;
	
	Animatable_Init(&this->character.animatable, char_master_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = 0;
	
	this->character.health_i = 11;
	
	this->character.focus_x = FIXED_DEC(65,1);
	this->character.focus_y = FIXED_DEC(-115,1);
	this->character.focus_zoom = FIXED_DEC(1,1);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\MASTER.ARC;1");
	
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
