/*
    C-Dogs SDL
    A port of the legendary (and fun) action/arcade cdogs.
    Copyright (C) 1995 Ronny Wester
    Copyright (C) 2003 Jeremy Chin 
    Copyright (C) 2003-2007 Lucas Martin-King 

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

-------------------------------------------------------------------------------

 prep.c - preparation stuff 
 
 Author: $Author: lmartinking $
 Rev:    $Revision: 265 $
 URL:    $HeadURL: svn://svn.icculus.org/cdogs-sdl/trunk/src/prep.c $
 ID:     $Id: prep.c 265 2008-02-10 09:53:42Z lmartinking $
 
*/


#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "prep.h"
#include "input.h"
#include "grafx.h"
#include "blit.h"
#include "actors.h"
#include "pics.h"
#include "text.h"
#include "sounds.h"
#include "keyboard.h"
#include "menu.h"
#include "files.h"
#include "utils.h"


#define MODE_MAIN           0
#define MODE_SELECTNAME     1
#define MODE_SELECTFACE     2
#define MODE_SELECTSKIN     3
#define MODE_SELECTHAIR     4
#define MODE_SELECTARMS     5
#define MODE_SELECTBODY     6
#define MODE_SELECTLEGS     7
#define MODE_LOADTEMPLATE   8
#define MODE_SAVETEMPLATE   9
#define MODE_DONE           10

#define MENU_COUNT  11

#define PLAYER_FACE_COUNT   7
#define PLAYER_BODY_COUNT   9
#define PLAYER_SKIN_COUNT   3
#define PLAYER_HAIR_COUNT   8


#define AVAILABLE_FACES PLAYER_FACE_COUNT


static const char *faceNames[PLAYER_FACE_COUNT] = {
	"Jones",
	"Ice",
	"WarBaby",
	"Dragon",
	"Smith",
	"Lady",
	"Wolf"
};


static const char *shadeNames[PLAYER_BODY_COUNT] = {
	"Blue",
	"Green",
	"Red",
	"Silver",
	"Brown",
	"Purple",
	"Black",
	"Yellow",
	"White"
};

static const char *skinNames[PLAYER_SKIN_COUNT] = {
	"Cacausian",
	"Asian",
	"Black"
};

static const char *hairNames[PLAYER_HAIR_COUNT] = {
	"Red",
	"Silver",
	"Brown",
	"Gray",
	"Blonde",
	"White",
	"Golden",
	"Black"
};

static const char *mainMenu[MENU_COUNT] = {
	"",
	"Change name",
	"Select face",
	"Select skin",
	"Select hair",
	"Select arms",
	"Select body",
	"Select legs",
	"Use template",
	"Save template",
	"Done"
};

const char *endChoice = "(End)";


struct PlayerTemplate {
	char name[20];
	int head;
	int body;
	int arms;
	int legs;
	int skin;
	int hair;
};

#define MAX_TEMPLATE  10
struct PlayerTemplate templates[MAX_TEMPLATE] = {
	{"-- empty --"},
	{"-- empty --"},
	{"-- empty --"},
	{"-- empty --"},
	{"-- empty --"},
	{"-- empty --"},
	{"-- empty --"},
	{"-- empty --"},
	{"-- empty --"},
	{"-- empty --"}
};

void LoadTemplates(void)
{
	FILE *f;
	int i, count;

	f = fopen(GetConfigFilePath("players.cnf"), "r");
	if (f) {
		i = 0;
		fscanf(f, "%d\n", &count);
		while (i < MAX_TEMPLATE && i < count) {
			fscanf(f, "[%[^]]] %d %d %d %d %d %d\n",
			       templates[i].name,
			       &templates[i].head,
			       &templates[i].body,
			       &templates[i].arms,
			       &templates[i].legs,
			       &templates[i].skin, &templates[i].hair);
			i++;
		}
		fclose(f);
	}
}

void SaveTemplates(void)
{
	FILE *f;
	int i;

	debug(D_NORMAL, "begin\n");

	f = fopen(GetConfigFilePath("players.cnf"), "w");
	if (f) {
		fprintf(f, "%d\n", MAX_TEMPLATE);
		for (i = 0; i < MAX_TEMPLATE; i++) {
			fprintf(f, "[%s] %d %d %d %d %d %d\n",
				templates[i].name,
				templates[i].head,
				templates[i].body,
				templates[i].arms,
				templates[i].legs,
				templates[i].skin, templates[i].hair);
		}
		fclose(f);

		debug(D_NORMAL, "saved templates\n");
	}
}

void DisplayPlayer(int x, struct PlayerData *data, int character,
		   int editingName)
{
	struct CharacterDescription *cd;
	TOffsetPic body, head;
	char s[22];
	int y;

	y = (SCREEN_HEIGHT / 10);

	cd = &characterDesc[character];

	if (editingName) {
		sprintf(s, "%c%s%c", '\020', data->name, '\021');
		TextStringAt(x, y, s);
	} else
		TextStringAt(x, y, data->name);

	body.dx = cBodyOffset[cd->unarmedBodyPic][DIRECTION_DOWN].dx;
	body.dy = cBodyOffset[cd->unarmedBodyPic][DIRECTION_DOWN].dy;
	body.picIndex =
	    cBodyPic[cd->unarmedBodyPic][DIRECTION_DOWN][STATE_IDLE];

	head.dx =
	    cNeckOffset[cd->unarmedBodyPic][DIRECTION_DOWN].dx +
	    cHeadOffset[cd->facePic][DIRECTION_DOWN].dx;
	head.dy =
	    cNeckOffset[cd->unarmedBodyPic][DIRECTION_DOWN].dy +
	    cHeadOffset[cd->facePic][DIRECTION_DOWN].dy;
	head.picIndex = cHeadPic[cd->facePic][DIRECTION_DOWN][STATE_IDLE];

	DrawTTPic(x + 20 + body.dx, y + 36 + body.dy, gPics[body.picIndex],
		  cd->table, gRLEPics[body.picIndex]);
	DrawTTPic(x + 20 + head.dx, y + 36 + head.dy, gPics[head.picIndex],
		  cd->table, gRLEPics[head.picIndex]);
}

static void ShowPlayerControls(int x, struct PlayerData *data)
{
	char s[256];
	int y;

	y = SCREEN_HEIGHT - (SCREEN_HEIGHT / 6);

	if (data->controls == JOYSTICK_ONE)
		TextStringAt(x, y, "(joystick one)");
	else if (data->controls == JOYSTICK_TWO)
		TextStringAt(x, y, "(joystick two)");
	else {
		sprintf(s, "(%s, %s, %s, %s, %s and %s)",
			SDL_GetKeyName(data->keys[0]),
			SDL_GetKeyName(data->keys[1]),
			SDL_GetKeyName(data->keys[2]),
			SDL_GetKeyName(data->keys[3]),
			SDL_GetKeyName(data->keys[4]),
			SDL_GetKeyName(data->keys[5]));
		if (TextWidth(s) < 125)
			TextStringAt(x, y, s);
		else {
			sprintf(s, "(%s, %s, %s,",
				SDL_GetKeyName(data->keys[0]),
				SDL_GetKeyName(data->keys[1]),
				SDL_GetKeyName(data->keys[2]));
			TextStringAt(x, y - 10, s);
			sprintf(s, "%s, %s and %s)",
				SDL_GetKeyName(data->keys[3]),
				SDL_GetKeyName(data->keys[4]),
				SDL_GetKeyName(data->keys[5]));
			TextStringAt(x, y, s);
		}
	}
}

static void ShowSelection(int x, struct PlayerData *data, int character)
{
	int i;

	DisplayPlayer(x, data, character, 0);

	if (data->weaponCount == 0) {
			TextStringAt(x + 40, (SCREEN_HEIGHT / 10) + 20, "None selected...");
	} else {
		for (i = 0; i < data->weaponCount; i++)
			TextStringAt(x + 40, (SCREEN_HEIGHT / 10) + 20 + i * TextHeight(),
				     gunDesc[data->weapons[i]].gunName);
	}
}

static int NameSelection(int x, int index, struct PlayerData *data,
			 int cmd)
{
	int i;
	int y;

	char s[2];
	static char letters[] =
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ !#?:.-0123456789";
	static char smallLetters[] =
		"abcdefghijklmnopqrstuvwxyz !#?:.-0123456789";
	static int selection[2] = { -1, -1 };

	// Kludge since Watcom won't let me initialize selection with a strlen()
	if (selection[0] < 0)
		selection[0] = selection[1] = strlen(letters);

	if (cmd & CMD_BUTTON1) {
		if (selection[index] == strlen(letters)) {
			PlaySound(SND_LAUNCH, 0, 255);
			return 0;
		}

		if (strlen(data->name) < sizeof(data->name) - 1) {
			int l = strlen(data->name);
			data->name[l + 1] = 0;
			if (l > 0 && data->name[l - 1] != ' ')
				data->name[l] =
				    smallLetters[selection[index]];
			else
				data->name[l] = letters[selection[index]];
			PlaySound(SND_MACHINEGUN, 0, 255);
		} else
			PlaySound(SND_KILL, 0, 255);
	} else if (cmd & CMD_BUTTON2) {
		if (data->name[0]) {
			data->name[strlen(data->name) - 1] = 0;
			PlaySound(SND_BANG, 0, 255);
		} else
			PlaySound(SND_KILL, 0, 255);
	} else if (cmd & CMD_LEFT) {
		if (selection[index] > 0) {
			selection[index]--;
			PlaySound(SND_DOOR, 0, 255);
		}
	} else if (cmd & CMD_RIGHT) {
		if (selection[index] < strlen(letters)) {
			selection[index]++;
			PlaySound(SND_DOOR, 0, 255);
		}
	} else if (cmd & CMD_UP) {
		if (selection[index] > 9) {
			selection[index] -= 10;
			PlaySound(SND_DOOR, 0, 255);
		}
	} else if (cmd & CMD_DOWN) {
		if (selection[index] < strlen(letters) - 9) {
			selection[index] += 10;
			PlaySound(SND_DOOR, 0, 255);
		} else if (selection[index] < strlen(letters)) {
			selection[index] = strlen(letters);
			PlaySound(SND_DOOR, 0, 255);
		}
	}

	#define ENTRY_COLS	10
	#define	ENTRY_SPACING	12
	
	y = CenterY(((TextHeight() * ((strlen(letters) - 1) / ENTRY_COLS) )));

	if (gOptions.twoPlayers && index == CHARACTER_PLAYER1)
		x = CenterOf(0, (SCREEN_WIDTH / 2), ((ENTRY_SPACING * (ENTRY_COLS - 1)) + TextCharWidth('a')));
	else if (gOptions.twoPlayers && index == CHARACTER_PLAYER2)
		x = CenterOf((SCREEN_WIDTH / 2), (SCREEN_WIDTH), ((ENTRY_SPACING * (ENTRY_COLS - 1)) + TextCharWidth('a')));
	else
		x = CenterX(((ENTRY_SPACING * (ENTRY_COLS - 1)) + TextCharWidth('a')));

	// Draw selection

	s[1] = 0;
	for (i = 0; i < strlen(letters); i++) {
		s[0] = letters[i];

		TextGoto(x + (i % ENTRY_COLS) * ENTRY_SPACING,
			 y + (i / ENTRY_COLS) * TextHeight());

		if (i == selection[index])
			TextCharWithTable(letters[i], &tableFlamed);
		else
			TextChar(letters[i]);
/*
		DisplayMenuItem(x + (i % 10) * 12,
				80 + (i / 10) * TextHeight(), s,
				i == selection[index]);
*/
	}

	DisplayMenuItem(x + (i % ENTRY_COLS) * ENTRY_SPACING,
			y + (i / ENTRY_COLS) * TextHeight(),
			endChoice, i == selection[index]);

	return 1;
}

static int IndexToHead(int index)
{
	switch (index) {
	case 0:
		return FACE_JONES;
	case 1:
		return FACE_ICE;
	case 2:
		return FACE_WARBABY;
	case 3:
		return FACE_HAN;
	case 4:
		return FACE_BLONDIE;
	case 5:
		return FACE_LADY;
	case 6:
		return FACE_PIRAT2;
	}
	return FACE_BLONDIE;
}

static int IndexToSkin(int index)
{
	switch (index) {
	case 0:
		return SHADE_SKIN;
	case 1:
		return SHADE_ASIANSKIN;
	case 2:
		return SHADE_DARKSKIN;
	}
	return SHADE_SKIN;
}

static int IndexToHair(int index)
{
	switch (index) {
	case 0:
		return SHADE_RED;
	case 1:
		return SHADE_GRAY;
	case 2:
		return SHADE_BROWN;
	case 3:
		return SHADE_DKGRAY;
	case 4:
		return SHADE_YELLOW;
	case 5:
		return SHADE_LTGRAY;
	case 6:
		return SHADE_GOLDEN;
	case 7:
		return SHADE_BLACK;
	}
	return SHADE_SKIN;
}

static int IndexToShade(int index)
{
	switch (index) {
	case 0:
		return SHADE_BLUE;
	case 1:
		return SHADE_GREEN;
	case 2:
		return SHADE_RED;
	case 3:
		return SHADE_GRAY;
	case 4:
		return SHADE_BROWN;
	case 5:
		return SHADE_PURPLE;
	case 6:
		return SHADE_DKGRAY;
	case 7:
		return SHADE_YELLOW;
	case 8:
		return SHADE_LTGRAY;
	}
	return SHADE_BLUE;
};

static void SetPlayer(int character, struct PlayerData *data)
{
	int face, skin, hair;

	face = IndexToHead(data->head);
	skin = IndexToSkin(data->skin);
	hair = IndexToHair(data->hair);
	characterDesc[character].armedBodyPic = BODY_ARMED;
	characterDesc[character].unarmedBodyPic = BODY_UNARMED;
	characterDesc[character].speed = 256;
	characterDesc[character].maxHealth = 200;
	SetCharacter(character, face, skin, hair, data->body, data->arms,
		     data->legs);
}

static int AppearanceSelection(const char **menu, int menuCount,
			       int x, int index,
			       struct PlayerData *data, int *property,
			       int cmd, int *selection)
{
	int y;
	int i;

	debug(D_NORMAL, "\n");

	if (cmd & (CMD_BUTTON1 | CMD_BUTTON2)) {
		PlaySound(SND_MACHINEGUN, 0, 255);
		return 0;
	} else if (cmd & (CMD_LEFT | CMD_UP)) {
		if (selection[index] > 0) {
			selection[index]--;
			*property = selection[index];
			PlaySound(SND_SWITCH, 0, 255);
		} else if (selection[index] == 0) {
			selection[index] = menuCount - 1;
			*property = selection[index];
			PlaySound(SND_SWITCH, 0, 255);
		}
	} else if (cmd & (CMD_RIGHT | CMD_DOWN)) {
		if (selection[index] < menuCount - 1) {
			selection[index]++;
			*property = selection[index];
			PlaySound(SND_SWITCH, 0, 255);
		} else if (selection[index] == menuCount - 1) {
			selection[index] = 0;
			*property = selection[index];
			PlaySound(SND_SWITCH, 0, 255);
		}
	}

	SetPlayer(index, data);

	y = CenterY((menuCount * TextHeight()));

	for (i = 0; i < menuCount; i++)
		DisplayMenuItem(x, y + i * TextHeight(), menu[i],
				i == selection[index]);

	return 1;
}

static int FaceSelection(int x, int index, struct PlayerData *data,
			 int cmd)
{
	static int selection[2] = { 0, 1 };

	return AppearanceSelection(faceNames, AVAILABLE_FACES, x, index,
				   data, &data->head, cmd, selection);
}

static int SkinSelection(int x, int index, struct PlayerData *data,
			 int cmd)
{
	static int selection[2] = { 0, 1 };

	return AppearanceSelection(skinNames, PLAYER_SKIN_COUNT, x, index,
				   data, &data->skin, cmd, selection);
}

static int HairSelection(int x, int index, struct PlayerData *data,
			 int cmd)
{
	static int selection[2] = { 0, 1 };

	return AppearanceSelection(hairNames, PLAYER_HAIR_COUNT, x, index,
				   data, &data->hair, cmd, selection);
}

static int BodyPartSelection(int x, int index, struct PlayerData *data,
			     int cmd, int *property, int *selection)
{
	int i;
	int y;

	if (cmd & (CMD_BUTTON1 | CMD_BUTTON2)) {
		PlaySound(SND_POWERGUN, 0, 255);
		return 0;
	} else if (cmd & (CMD_LEFT | CMD_UP)) {
		if (*selection > 0) {
			(*selection)--;
			*property = IndexToShade(*selection);
			PlaySound(SND_SWITCH, 0, 255);
		} else if (*selection == 0) {
			(*selection) = PLAYER_BODY_COUNT - 1;
			*property = IndexToShade(*selection);
			PlaySound(SND_SWITCH, 0, 255);
		}
	} else if (cmd & (CMD_RIGHT | CMD_DOWN)) {
		if (*selection < PLAYER_BODY_COUNT - 1) {
			(*selection)++;
			*property = IndexToShade(*selection);
			PlaySound(SND_SWITCH, 0, 255);
		} else if (*selection == PLAYER_BODY_COUNT - 1) {
			(*selection) = 0;
			*property = IndexToShade(*selection);
			PlaySound(SND_SWITCH, 0, 255);
		}
	}

	y = CenterY((PLAYER_BODY_COUNT * TextHeight()));

	SetPlayer(index, data);
	for (i = 0; i < PLAYER_BODY_COUNT; i++)
		DisplayMenuItem(x, y + i * TextHeight(), shadeNames[i],
				i == *selection);

	return 1;
}

static int ArmSelection(int x, int index, struct PlayerData *data, int cmd)
{
	static int selection[2] = { 0, 0 };

	return BodyPartSelection(x, index, data, cmd, &data->arms,
				 &selection[index]);
}

static int BodySelection(int x, int index, struct PlayerData *data,
			 int cmd)
{
	static int selection[2] = { 0, 0 };

	return BodyPartSelection(x, index, data, cmd, &data->body,
				 &selection[index]);
}

static int LegSelection(int x, int index, struct PlayerData *data, int cmd)
{
	static int selection[2] = { 0, 0 };

	return BodyPartSelection(x, index, data, cmd, &data->legs,
				 &selection[index]);
}

static int WeaponSelection(int x, int index, struct PlayerData *data,
			   int cmd, int done)
{
	int i;
	int y;
	static int selection[2] = { 0, 0 };

	debug(D_NORMAL, "\n");

	if (selection[index] > gMission.weaponCount)
		selection[index] = gMission.weaponCount;

	if (cmd & CMD_BUTTON1) {
		if (selection[index] == gMission.weaponCount) {
			PlaySound(SND_KILL2, 0, 255);
			return data->weaponCount > 0 ? 0 : 1;
		}

		if (data->weaponCount < MAX_WEAPONS) {
			for (i = 0; i < data->weaponCount; i++)
				if (data->weapons[i] == gMission.availableWeapons[selection[index]])
					return 1;

			data->weapons[data->weaponCount] = gMission.availableWeapons[selection[index]];
			data->weaponCount++;

			PlaySound(SND_SHOTGUN, 0, 255);
		} else {
			PlaySound(SND_KILL, 0, 255);
		}
	} else if (cmd & CMD_BUTTON2) {
		if (data->weaponCount) {
			data->weaponCount--;
			PlaySound(SND_PICKUP, 0, 255);
			done = 0;
		}
	} else if (cmd & (CMD_LEFT | CMD_UP)) {
		if (selection[index] > 0) {
			selection[index]--;
			PlaySound(SND_SWITCH, 0, 255);
		} else if (selection[index] == 0) {
			selection[index] = gMission.weaponCount;
			PlaySound(SND_SWITCH, 0, 255);
		}
		done = 0;
	} else if (cmd & (CMD_RIGHT | CMD_DOWN)) {
		if (selection[index] < gMission.weaponCount) {
			selection[index]++;
			PlaySound(SND_SWITCH, 0, 255);
		} else if (selection[index] == gMission.weaponCount) {
			selection[index] = 0;
			PlaySound(SND_SWITCH, 0, 255);
		}
		done = 0;
	}

	if (!done) {
		y = CenterY((TextHeight() * gMission.weaponCount));

		for (i = 0; i < gMission.weaponCount; i++)
			DisplayMenuItem(x, y + i * TextHeight(),
					gunDesc[gMission.availableWeapons[i]].
					gunName, i == selection[index]);

		DisplayMenuItem(x, y + i * TextHeight(), endChoice, i == selection[index]);
	}

	return !done;
}

void UseTemplate(int character, struct PlayerData *data,
		 struct PlayerTemplate *t)
{
	memset(data->name, 0, sizeof(data->name));
	strncpy(data->name, t->name, sizeof(data->name) - 1);

	data->head = (t->head < AVAILABLE_FACES ? t->head : 0);
	data->body = t->body;
	data->arms = t->arms;
	data->legs = t->legs;
	data->skin = t->skin;
	data->hair = t->hair;

	SetPlayer(character, data);
}

void SaveTemplate(struct PlayerData *data, struct PlayerTemplate *t)
{
	memset(t->name, 0, sizeof(t->name));
	strncpy(t->name, data->name, sizeof(t->name) - 1);

	t->head = data->head;
	t->body = data->body;
	t->arms = data->arms;
	t->legs = data->legs;
	t->skin = data->skin;
	t->hair = data->hair;
}

static int TemplateSelection(int loadFlag, int x, int index,
			     struct PlayerData *data, int cmd)
{
	int i;
	int y;
	static int selection[2] = { 0, 0 };

	if (cmd & CMD_BUTTON1) {
		if (loadFlag)
			UseTemplate(index, data,
				    &templates[selection[index]]);
		else
			SaveTemplate(data, &templates[selection[index]]);
		PlaySound(rand() % SND_COUNT, 0, 255);
		return 0;
	} else if (cmd & CMD_BUTTON2) {
		PlaySound(rand() % SND_COUNT, 0, 255);
		return 0;
	} else if (cmd & (CMD_LEFT | CMD_UP)) {
		if (selection[index] > 0) {
			selection[index]--;
			PlaySound(SND_SWITCH, 0, 255);
		} else if (selection[index] == 0) {
			selection[index] = MAX_TEMPLATE - 1;
			PlaySound(SND_SWITCH, 0, 255);
		}
	} else if (cmd & (CMD_RIGHT | CMD_DOWN)) {
		if (selection[index] < MAX_TEMPLATE - 1) {
			selection[index]++;
			PlaySound(SND_SWITCH, 0, 255);
		} else if (selection[index] == MAX_TEMPLATE - 1) {
			selection[index] = 0;
			PlaySound(SND_SWITCH, 0, 255);
		}
	}

	y = CenterY((TextHeight() * MAX_TEMPLATE));

	if (!loadFlag) {
		TextStringAt(x, y - 4 - TextHeight(), "Save ");
		TextString(data->name);
		TextString("...");
	}

	for (i = 0; i < MAX_TEMPLATE; i++)
		DisplayMenuItem(x, y + i * TextHeight(),
				templates[i].name, i == selection[index]);

	return 1;
}

static int MainMenu(int x, int index, int cmd)
{
	int i;
	int y;
	static int selection[2] = { MODE_DONE, MODE_DONE };

	if (cmd & (CMD_BUTTON1 | CMD_BUTTON2)) {
		PlaySound(SND_BANG, 0, 255);
		return selection[index];
	} else if (cmd & (CMD_LEFT | CMD_UP)) {
		if (selection[index] > MODE_SELECTNAME) {
			selection[index]--;
			PlaySound(SND_SWITCH, 0, 255);
		} else if (selection[index] == MODE_SELECTNAME) {
			selection[index] = MODE_DONE;
			PlaySound(SND_SWITCH, 0, 255);
		}
	} else if (cmd & (CMD_RIGHT | CMD_DOWN)) {
		if (selection[index] < MODE_DONE) {
			selection[index]++;
			PlaySound(SND_SWITCH, 0, 255);
		} else if (selection[index] == MODE_DONE) {
			selection[index] = MODE_SELECTNAME;
			PlaySound(SND_SWITCH, 0, 255);				
		}
	}

	y = CenterY((TextHeight() * MENU_COUNT));

	for (i = 1; i < MENU_COUNT; i++)
		DisplayMenuItem(x, y + i * TextHeight(), mainMenu[i],
				selection[index] == i);

	return MODE_MAIN;
}

static int MakeSelection(int mode, int x, int character,
			 struct PlayerData *data, int cmd)
{
	switch (mode) {
		case MODE_MAIN:
			mode = MainMenu(x, character, cmd);
			break;

		case MODE_SELECTNAME:
			if (!NameSelection(x, character, data, cmd))
				mode = MODE_MAIN;
			break;

		case MODE_SELECTFACE:
			if (!FaceSelection(x, character, data, cmd))
				mode = MODE_MAIN;
			break;

		case MODE_SELECTSKIN:
			if (!SkinSelection(x, character, data, cmd))
				mode = MODE_MAIN;
			break;

		case MODE_SELECTHAIR:
			if (!HairSelection(x, character, data, cmd))
				mode = MODE_MAIN;
			break;

		case MODE_SELECTARMS:
			if (!ArmSelection(x, character, data, cmd))
				mode = MODE_MAIN;
			break;

		case MODE_SELECTBODY:
			if (!BodySelection(x, character, data, cmd))
				mode = MODE_MAIN;
			break;

		case MODE_SELECTLEGS:
			if (!LegSelection(x, character, data, cmd))
				mode = MODE_MAIN;
			break;

		case MODE_LOADTEMPLATE:
			if (!TemplateSelection(1, x, character, data, cmd))
				mode = MODE_MAIN;
			break;

		case MODE_SAVETEMPLATE:
			if (!TemplateSelection(0, x, character, data, cmd))
				mode = MODE_MAIN;
			break;
	}
	DisplayPlayer(x, data, character, mode == MODE_SELECTNAME);
//      ShowPlayerControls(x, data);

	return mode;
}

int PlayerSelection(int twoPlayers, void *bkg)
{
	int cmd1, cmd2, prev1 = 0, prev2 = 0;
	int mode1, mode2;

	mode1 = MODE_MAIN;
	mode2 = twoPlayers ? MODE_MAIN : MODE_DONE;

	SetPlayer(0, &gPlayer1Data);
	SetPlayer(1, &gPlayer2Data);

	while (mode1 != MODE_DONE || mode2 != MODE_DONE) {
		memcpy(GetDstScreen(), bkg, SCREEN_MEMSIZE);
		GetPlayerCmd(&cmd1, &cmd2);
		
		if (KeyDown(keyEsc)) return 0; // hack to allow exit
		
		if (twoPlayers) {
			if (cmd1 == prev1)
				cmd1 = 0;
			else
				prev1 = cmd1;

			mode1 = MakeSelection(mode1, CenterOfLeft(50), CHARACTER_PLAYER1, &gPlayer1Data, cmd1);

			if (cmd2 == prev2)
				cmd2 = 0;
			else
				prev2 = cmd2;

			mode2 = MakeSelection(mode2, CenterOfRight(50), CHARACTER_PLAYER2, &gPlayer2Data, cmd2);
		} else {
			if (cmd1 == prev1)
				cmd1 = 0;
			else
				prev1 = cmd1;

			mode1 = MakeSelection(mode1, CenterX(50), CHARACTER_PLAYER1, &gPlayer1Data, cmd1);
		}

		CopyToScreen();
	}

	WaitForRelease();

	return 1;
}

int PlayerEquip(void *bkg)
{
	int cmd1, cmd2, prev1 = 0, prev2 = 0;
	int done1 = 0, done2;

	debug(D_NORMAL, "\n");

	done2 = gOptions.twoPlayers ? 0 : 1;
	while (!done1 || !done2) {
		memcpy(GetDstScreen(), bkg, SCREEN_MEMSIZE);
		GetPlayerCmd(&cmd1, &cmd2);
		
		if (KeyDown(keyEsc)) return 0; // hack to exit from menu
		
		if (gOptions.twoPlayers) {
			if (cmd1 == prev1)
				cmd1 = 0;
			else
				prev1 = cmd1;
//      if (!done1) // || !gPlayer1Data.weaponCount < MAX_WEAPONS)
			done1 = !WeaponSelection(CenterOfLeft(50), CHARACTER_PLAYER1, &gPlayer1Data, cmd1, done1);
			ShowSelection(CenterOfLeft(50), &gPlayer1Data,CHARACTER_PLAYER1);
			ShowPlayerControls(CenterOfLeft(100), &gPlayer1Data);

			if (cmd2 == prev2)
				cmd2 = 0;
			else
				prev2 = cmd2;
//      if (!done2) // || gPlayer2Data.weaponCount < MAX_WEAPONS)
			done2 = !WeaponSelection(CenterOfRight(50), CHARACTER_PLAYER2, &gPlayer2Data, cmd2, done2);
			ShowSelection(CenterOfRight(50), &gPlayer2Data, CHARACTER_PLAYER2);
			ShowPlayerControls(CenterOfRight(100), &gPlayer2Data);
		} else {
			if (cmd1 == prev1)
				cmd1 = 0;
			else
				prev1 = cmd1;
			if (!done1)	// || gPlayer1Data.weaponCount <= 0)
				done1 =
				    !WeaponSelection(CenterX(80),
						     CHARACTER_PLAYER1,
						     &gPlayer1Data, cmd1,
						     done1);
			ShowSelection(CenterX(80), &gPlayer1Data,
				      CHARACTER_PLAYER1);
			ShowPlayerControls(CenterX(100), &gPlayer1Data);
		}

		CopyToScreen();
	}

	WaitForRelease();

	return 1;
}
