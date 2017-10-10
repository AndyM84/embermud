/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <time.h>
#include "merc.h"
#include "magic.h"

/* attack table  -- not very organized :( */
const struct attack_type attack_table[] = {
    {"hit", -1},                /*  0 */
    {"slice", DAM_SLASH},
    {"stab", DAM_PIERCE},
    {"slash", DAM_SLASH},
    {"whip", DAM_SLASH},
    {"claw", DAM_SLASH},        /*  5 */
    {"blast", DAM_BASH},
    {"pound", DAM_BASH},
    {"crush", DAM_BASH},
    {"grep", DAM_SLASH},
    {"bite", DAM_PIERCE},       /* 10 */
    {"pierce", DAM_PIERCE},
    {"suction", DAM_BASH},
    {"beating", DAM_BASH},
    {"digestion", DAM_ACID},
    {"charge", DAM_BASH},       /* 15 */
    {"slap", DAM_BASH},
    {"punch", DAM_BASH},
    {"wrath", DAM_ENERGY},
    {"magic", DAM_ENERGY},
    {"divine power", DAM_HOLY}, /* 20 */
    {"cleave", DAM_SLASH},
    {"scratch", DAM_PIERCE},
    {"peck", DAM_PIERCE},
    {"peck", DAM_BASH},
    {"chop", DAM_SLASH},        /* 25 */
    {"sting", DAM_PIERCE},
    {"smash", DAM_BASH},
    {"shocking bite", DAM_LIGHTNING},
    {"flaming bite", DAM_FIRE},
    {"freezing bite", DAM_COLD},    /* 30 */
    {"acidic bite", DAM_ACID},
    {"chomp", DAM_PIERCE},
    {"weapon's bite", DAM_NEGATIVE},
    {"weapon's flame", DAM_FIRE},
    {"weapon's frost", DAM_COLD}    /* 35 */
};

/* race table */
const struct race_type race_table[] = {
/*
    {
	name,		pc_race?,
	act bits,	aff_by bits,	off bits,
	imm,		res,		vuln,
	form,		parts,          remort only 
    },
*/
    {"unique", FALSE, 0, 0, 0, 0, 0, 0, 0, 0},

    {
     "Dwarf", TRUE,
     0, AFF_INFRARED, 0,
     0, RES_MAGIC | RES_POISON | RES_DISEASE, VULN_DROWNING,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K, FALSE},

    {
     "Elf", TRUE,
     0, AFF_INFRARED, 0,
     0, RES_CHARM, VULN_IRON,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K, FALSE},

    {
     "Giant", TRUE,
     0, 0, 0,
     0, RES_FIRE | RES_COLD, VULN_MENTAL | VULN_LIGHTNING,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K, FALSE},

    {
     "Hobbit", TRUE,
     0, AFF_INFRARED, 0,
     0, RES_MAGIC | RES_POISON | RES_DISEASE, VULN_DROWNING,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K, FALSE},

    {
     "Human", TRUE,
     0, 0, 0,
     0, 0, 0,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K, FALSE},

    {
     "Troll", TRUE,
     0, AFF_REGENERATION | AFF_INFRARED,
     OFF_BERSERK,
     0, RES_CHARM | RES_BASH, VULN_FIRE | VULN_ACID,
     B | M | V, A | B | C | D | E | F | G | H | I | J | K | U | V, FALSE},

    {
     "Wolf", FALSE,
     0, AFF_DARK_VISION, OFF_FAST | OFF_DODGE,
     0, 0, 0,
     A | G | V, A | C | D | E | F | J | K | Q | V, FALSE},

    {
     "Wyvern", FALSE,
     0, AFF_FLYING | AFF_DETECT_INVIS | AFF_DETECT_HIDDEN,
     OFF_BASH | OFF_FAST | OFF_DODGE,
     IMM_POISON, 0, VULN_LIGHT,
     B | Z | cc, A | C | D | E | F | H | J | K | Q | U | V | X, FALSE},

    {
     "Drow Elf", FALSE,
     0, AFF_DARK_VISION, 0,
     0, RES_CHARM, VULN_IRON | VULN_LIGHT,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K},

    {
     "bat", FALSE,
     0, AFF_FLYING | AFF_DARK_VISION, OFF_DODGE | OFF_FAST,
     0, 0, VULN_LIGHT,
     A | G | W, A | C | D | E | F | H | J | K | P, FALSE},

    {
     "bear", FALSE,
     0, 0, OFF_CRUSH | OFF_DISARM | OFF_BERSERK,
     0, RES_BASH | RES_COLD, 0,
     A | G | V, A | B | C | D | E | F | H | J | K | U | V, FALSE},

    {
     "cat", FALSE,
     0, AFF_DARK_VISION, OFF_FAST | OFF_DODGE,
     0, 0, 0,
     A | G | V, A | C | D | E | F | H | J | K | Q | U | V, FALSE},

    {
     "centipede", FALSE,
     0, AFF_DARK_VISION, 0,
     0, RES_PIERCE | RES_COLD, VULN_BASH, FALSE},

    {
     "dog", FALSE,
     0, 0, OFF_FAST,
     0, 0, 0,
     A | G | V, A | C | D | E | F | H | J | K | U | V, FALSE},

    {
     "doll", FALSE,
     0, 0, 0,
     IMM_MAGIC, RES_BASH | RES_LIGHT,
     VULN_SLASH | VULN_FIRE | VULN_ACID | VULN_LIGHTNING | VULN_ENERGY,
     E | J | M | cc, A | B | C | G | H | K, FALSE},

    {
     "fido", FALSE,
     0, 0, OFF_DODGE | ASSIST_RACE,
     0, 0, VULN_MAGIC,
     B | G | V, A | C | D | E | F | H | J | K | Q | V, FALSE},

    {
     "fox", FALSE,
     0, AFF_DARK_VISION, OFF_FAST | OFF_DODGE,
     0, 0, 0,
     A | G | V, A | C | D | E | F | H | J | K | Q | V, FALSE},

    {
     "goblin", FALSE,
     0, AFF_INFRARED, 0,
     0, RES_DISEASE, VULN_MAGIC,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K, FALSE},

    {
     "hobgoblin", FALSE,
     0, AFF_INFRARED, 0,
     0, RES_DISEASE | RES_POISON, 0,
     0, 0, FALSE},

    {
     "kobold", FALSE,
     0, AFF_INFRARED, 0,
     0, RES_POISON, VULN_MAGIC,
     A | B | H | M | V, A | B | C | D | E | F | G | H | I | J | K | Q, FALSE},

    {
     "lizard", FALSE,
     0, 0, 0,
     0, RES_POISON, VULN_COLD,
     A | G | X | cc, A | C | D | E | F | H | K | Q | V, FALSE},

    {
     "modron", FALSE,
     0, AFF_INFRARED, ASSIST_RACE | ASSIST_ALIGN,
     IMM_CHARM | IMM_DISEASE | IMM_MENTAL | IMM_HOLY | IMM_NEGATIVE,
     RES_FIRE | RES_COLD | RES_ACID, 0,
     H, A | B | C | G | H | J | K, FALSE},

    {
     "orc", FALSE,
     0, AFF_INFRARED, 0,
     0, RES_DISEASE, VULN_LIGHT,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K, FALSE},

    {
     "pig", FALSE,
     0, 0, 0,
     0, 0, 0,
     A | G | V, A | C | D | E | F | H | J | K, FALSE},

    {
     "rabbit", FALSE,
     0, 0, OFF_DODGE | OFF_FAST,
     0, 0, 0,
     A | G | V, A | C | D | E | F | H | J | K, FALSE},

    {
     "sailor", FALSE,
     bb, 0, 0,
     0, 0, 0,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K, FALSE},

    {
     "school monster", FALSE,
     ACT_NOALIGN, 0, 0,
     IMM_CHARM | IMM_SUMMON, 0, VULN_MAGIC,
     A | M | V, A | B | C | D | E | F | H | J | K | Q | U, FALSE},

    {
     "shiriff", FALSE,
     T, 0, L | P,
     0, 0, 0,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K, FALSE},

    {
     "snake", FALSE,
     0, 0, 0,
     0, RES_POISON, VULN_COLD,
     A | G | R | X | Y | cc, A | D | E | F | K | L | Q | V | X, FALSE},

    {
     "song bird", FALSE,
     0, AFF_FLYING, OFF_FAST | OFF_DODGE,
     0, 0, 0,
     A | G | W, A | C | D | E | F | H | K | P, FALSE},

    {
     "thain", FALSE,
     T, 0, L | P,
     0, 0, 0,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K, FALSE},

    {
     "water fowl", FALSE,
     0, AFF_SWIM | AFF_FLYING, 0,
     0, RES_DROWNING, 0,
     A | G | W, A | C | D | E | F | H | K | P, FALSE},

    {
     NULL, 0, 0, 0, 0, 0, 0}
};

/* Race Table for stats of 25 */
#if MAX_ATTAINABLE_STATS == 25
const struct pc_race_type pc_race_table[] = {
    {"null race", "", 0, {100, 100, 100, 100},
     {""}, {13, 13, 13, 13, 13}, {18, 18, 18, 18, 18}, 0},

/*
    {
	"race name", 	short name, 	points,	{ class multipliers },
	{ bonus skills },
	{ base stats },		{ max stats },		size 
    },
*/

/*    { 	
	"drow elf",		"Drow ",	5,	{ 100, 125,  100, 120 }, 
	{ "sneak", "hide" },
	{ -1, 1, 0, 2, -2 },	{ 16, 20, 18, 21, 15 }, SIZE_MEDIUM
    },*/

    {
     "dwarf", "  Dwarf   ", 8, {150, 100, 125, 100},
     {"berserk"},
     {2, -2, 1, -4, 3}, {20, 16, 19, 14, 21}, SIZE_MEDIUM},

    {
     "elf", "   Elf    ", 5, {100, 125, 100, 120},
     {"sneak", "hide"},
     {-2, 2, 0, 3, -3}, {16, 20, 18, 21, 15}, SIZE_MEDIUM},

    {
     "giant", "  Giant   ", 6, {200, 150, 150, 100},
     {"bash", "fast healing"},
     {4, -3, 0, -3, 2}, {22, 15, 18, 15, 20}, SIZE_HUGE},

    {
     "hobbit", "  Hobbit  ", 5, {125, 125, 100, 150},
     {"sneak", "hide"},
     {-1, -2, -1, 4, 0}, {17, 16, 17, 22, 18}, SIZE_MEDIUM},

    {
     "human", "  Human   ", 0, {100, 100, 100, 100},
     {""},
     {0, 0, 0, 0, 0}, {18, 18, 18, 18, 18}, SIZE_MEDIUM},

    {
     "troll", "  Troll   ", 12, {175, 150, 125, 100},
     {""},
     {2, -1, 0, -2, 1}, {20, 17, 18, 16, 19}, SIZE_LARGE},

};
#endif

#if MAX_ATTAINABLE_STATS == 30
const struct pc_race_type pc_race_table[] = {
    {"null race", "", 0, {100, 100, 100, 100},
     {""}, {13, 13, 13, 13, 13}, {18, 18, 18, 18, 18}, 0},

/*
    {
	"race name", 	short name, 	points,	{ class multipliers },
	{ bonus skills },
	{ base stats },		{ max stats },		size 
    },
*/

/*    { 	
	"drow elf",		"Drow ",	5,	{ 100, 125,  100, 120 }, 
	{ "sneak", "hide" },
	{ -1, 1, 0, 2, -2 },	{ 16, 20, 18, 21, 15 }, SIZE_MEDIUM
    },*/

    {
     "dwarf", "  Dwarf   ", 8, {150, 100, 125, 100},
     {"berserk"},
     {1, -1, 1, -3, 2}, {20, 16, 19, 14, 24}, SIZE_MEDIUM},

    {
     "elf", "   Elf    ", 5, {100, 125, 100, 120},
     {"sneak", "hide"},
     {-1, 1, 0, 2, -2}, {16, 26, 19, 21, 15}, SIZE_MEDIUM},

    {
     "giant", "  Giant   ", 6, {200, 150, 150, 100},
     {"bash", "fast healing"},
     {3, -2, 0, -2, 1}, {26, 15, 18, 15, 20}, SIZE_HUGE},

    {
     "hobbit", "  Hobbit  ", 5, {125, 125, 100, 150},
     {"sneak", "hide"},
     {-1, -2, -1, 3, 1}, {17, 16, 17, 26, 18}, SIZE_MEDIUM},

    {
     "human", "  Human   ", 0, {100, 100, 100, 100},
     {""},
     {0, 0, 0, 0, 0}, {19, 19, 19, 19, 19}, SIZE_MEDIUM},

    {
     "troll", "  Troll   ", 12, {175, 150, 125, 100},
     {""},
     {2, -1, 0, -2, 1}, {20, 17, 18, 16, 26}, SIZE_LARGE},

};
#endif

/*
 * Class table.
 */
/* VERY IMPORTANT NOTE:
   If you insert a new class, PUT IT AT THE END!
   If you don't the mud won't crash but will certainly get
   VERY screwed up.  

   Another very important thing:
   Don't forget the console class!  If you add any more classes,
   you NEED to remember that the console class is the fifth one
   but you can probably just put zeroes for all its values.

   Kyle
 */
#if defined(cbuilder)
struct class_type class_table[MAX_CLASS + 1] =
#else
struct class_type class_table[MAX_CLASS] =
#endif
{
    {
     "Mage", "Mag", STAT_INT, OBJ_VNUM_SCHOOL_DAGGER,
     {3018, 9618}, 75, 18, 6, 6, 8, TRUE,
     "mage basics", "mage default", FALSE},

    {
     "Cleric", "Cle", STAT_WIS, OBJ_VNUM_SCHOOL_MACE,
     {3003, 9619}, 75, 18, 2, 7, 10, TRUE,
     "cleric basics", "cleric default", FALSE},

    {
     "Thief", "Thi", STAT_DEX, OBJ_VNUM_SCHOOL_DAGGER,
     {3028, 9639}, 75, 18, -4, 8, 13, FALSE,
     "thief basics", "thief default", FALSE},

    {
     "Warrior", "War", STAT_STR, OBJ_VNUM_SCHOOL_SWORD,
     {3022, 9633}, 75, 18, -10, 11, 15, FALSE,
     "warrior basics", "warrior default", FALSE}

#if defined(cbuilder)
    /* Class added solely for Win32 GUI Console Administration. -Zane */
    , {
       "Console", "Con", STAT_STR, OBJ_VNUM_SCHOOL_SWORD,
       {3022, 9633}, 75, 18, -10, 11, 15, FALSE,
       "warrior basics", "warrior default", FALSE}
#endif
};

/*
 * Attribute bonus tables. If you changed MAX_ATTAINABLE_STATS to a number
HIGHER then 30, you MUST edit these tables to make the higher stats affect
anything
 */
#if MAX_ATTAINABLE_STATS == 30
const struct str_app_type str_app[MAX_ATTAINABLE_STATS + 1] = {
    {-5, -4, 0, 0},             /* 0  */
    {-5, -4, 3, 1},             /* 1  */
    {-3, -2, 3, 2},
    {-3, -1, 10, 3},            /* 3  */
    {-2, -1, 25, 4},
    {-2, -1, 55, 5},            /* 5  */
    {-1, 0, 80, 6},
    {-1, 0, 90, 7},
    {0, 0, 100, 8},
    {0, 0, 100, 9},
    {0, 0, 115, 10},            /* 10  */
    {0, 0, 115, 11},
    {0, 0, 130, 12},
    {0, 0, 130, 13},            /* 13  */
    {0, 1, 140, 14},
    {1, 1, 150, 15},            /* 15  */
    {1, 2, 165, 16},
    {2, 3, 180, 22},
    {2, 3, 200, 25},            /* 18  */
    {3, 4, 225, 30},
    {3, 5, 250, 35},            /* 20  */
    {4, 6, 300, 40},
    {4, 6, 350, 45},
    {5, 7, 400, 50},
    {5, 8, 450, 55},
    {6, 9, 500, 60},            /* 25 Default Attainable  */
    {6, 9, 550, 65},
    {7, 10, 600, 70},
    {7, 10, 650, 80},
    {8, 11, 700, 85},
    {10, 13, 1000, 90}          /* 30 */

};
#else                           /* Stats table for MAX_ATTAINABLE_STATS = 25 */
const struct str_app_type str_app[MAX_ATTAINABLE_STATS + 1] = {
    {-5, -4, 0, 0},             /* 0  */
    {-5, -4, 3, 1},             /* 1  */
    {-3, -2, 3, 2},
    {-3, -1, 10, 3},            /* 3  */
    {-2, -1, 25, 4},
    {-2, -1, 55, 5},            /* 5  */
    {-1, 0, 80, 6},
    {-1, 0, 90, 7},
    {0, 0, 100, 8},
    {0, 0, 100, 9},
    {0, 0, 115, 10},            /* 10  */
    {0, 0, 115, 11},
    {0, 0, 130, 12},
    {0, 0, 130, 13},            /* 13  */
    {0, 1, 140, 14},
    {1, 1, 150, 15},            /* 15  */
    {1, 2, 165, 16},
    {2, 3, 180, 22},
    {2, 3, 200, 25},            /* 18  */
    {3, 4, 225, 30},
    {3, 5, 250, 35},            /* 20  */
    {4, 6, 300, 40},
    {4, 6, 350, 45},
    {5, 7, 400, 50},
    {5, 8, 450, 55},
    {6, 9, 500, 60}             /* 25 Default Attainable  */
};
#endif

#if MAX_ATTAINABLE_STATS == 30
const struct int_app_type int_app[MAX_ATTAINABLE_STATS + 1] = {
    {3},                        /*  0 */
    {5},                        /*  1 */
    {7},
    {8},                        /*  3 */
    {9},
    {10},                       /*  5 */
    {11},
    {12},
    {13},
    {15},
    {17},                       /* 10 */
    {19},
    {22},
    {25},
    {28},
    {31},                       /* 15 */
    {34},
    {37},
    {40},                       /* 18 */
    {44},
    {49},                       /* 20 */
    {55},
    {60},
    {70},
    {80},
    {85},                       /* 25 Default attainable */
    {87},
    {89},
    {91},
    {92},
    {95}                        /* 30 */
};
#else
const struct int_app_type int_app[MAX_ATTAINABLE_STATS + 1] = {
    {3},                        /*  0 */
    {5},                        /*  1 */
    {7},
    {8},                        /*  3 */
    {9},
    {10},                       /*  5 */
    {11},
    {12},
    {13},
    {15},
    {17},                       /* 10 */
    {19},
    {22},
    {25},
    {28},
    {31},                       /* 15 */
    {34},
    {37},
    {40},                       /* 18 */
    {44},
    {49},                       /* 20 */
    {55},
    {60},
    {70},
    {80},
    {85}                        /* 25 Default attainable */
};
#endif

#if MAX_ATTAINABLE_STATS == 30
const struct wis_app_type wis_app[MAX_ATTAINABLE_STATS + 1] = {
    {0},                        /*  0 */
    {0},                        /*  1 */
    {0},
    {0},                        /*  3 */
    {0},
    {1},                        /*  5 */
    {1},
    {1},
    {1},
    {1},
    {2},                        /* 10 */
    {2},
    {2},
    {3},
    {3},
    {3},                        /* 15 */
    {3},
    {3},
    {4},                        /* 18 */
    {4},
    {4},                        /* 20 */
    {4},
    {5},
    {5},
    {5},
    {6},                        /* 25 */
    {7},
    {8},
    {9},
    {10},
    {15}                        /* 30 */
};
#else
const struct wis_app_type wis_app[MAX_ATTAINABLE_STATS + 1] = {
    {0},                        /*  0 */
    {0},                        /*  1 */
    {0},
    {0},                        /*  3 */
    {0},
    {1},                        /*  5 */
    {1},
    {1},
    {1},
    {1},
    {2},                        /* 10 */
    {2},
    {2},
    {3},
    {3},
    {3},                        /* 15 */
    {3},
    {3},
    {4},                        /* 18 */
    {4},
    {4},                        /* 20 */
    {4},
    {5},
    {5},
    {5},
    {6}                         /* 25 */
};
#endif

#if MAX_ATTAINABLE_STATS == 30
const struct dex_app_type dex_app[MAX_ATTAINABLE_STATS + 1] = {
    {60},                       /* 0 */
    {50},                       /* 1 */
    {50},
    {40},
    {30},
    {20},                       /* 5 */
    {10},
    {0},
    {0},
    {0},
    {0},                        /* 10 */
    {0},
    {0},
    {0},
    {0},
    {-10},                      /* 15 */
    {-15},
    {-20},
    {-30},
    {-40},
    {-50},                      /* 20 */
    {-60},
    {-75},
    {-90},
    {-105},
    {-120},                     /* 25 Default Attainable */
    {-125},
    {-130},
    {-140},
    {-150},
    {-200}                      /* 30 */
};
#else
const struct dex_app_type dex_app[MAX_ATTAINABLE_STATS + 1] = {
    {60},                       /* 0 */
    {50},                       /* 1 */
    {50},
    {40},
    {30},
    {20},                       /* 5 */
    {10},
    {0},
    {0},
    {0},
    {0},                        /* 10 */
    {0},
    {0},
    {0},
    {0},
    {-10},                      /* 15 */
    {-15},
    {-20},
    {-30},
    {-40},
    {-50},                      /* 20 */
    {-60},
    {-75},
    {-90},
    {-105},
    {-120},                     /* 25 Default Attainable */
};
#endif

#if MAX_ATTAINABLE_STATS == 30
const struct con_app_type con_app[MAX_ATTAINABLE_STATS + 1] = {
    {-4, 20},                   /*  0 */
    {-3, 25},                   /*  1 */
    {-2, 30},
    {-2, 35},                   /*  3 */
    {-1, 40},
    {-1, 45},                   /*  5 */
    {-1, 50},
    {0, 55},
    {0, 60},
    {0, 65},
    {0, 70},                    /* 10 */
    {0, 75},
    {0, 80},
    {0, 85},
    {0, 88},
    {1, 90},                    /* 15 */
    {2, 95},
    {2, 97},
    {3, 99},                    /* 18 */
    {3, 99},
    {4, 99},                    /* 20 */
    {4, 99},
    {5, 99},
    {6, 99},
    {7, 99},
    {8, 99},                    /* 25 Default attainable */
    {9, 99},
    {10, 99},
    {11, 99},
    {12, 99},
    {18, 99}                    /* 30 */
};
#else
const struct con_app_type con_app[MAX_ATTAINABLE_STATS + 1] = {
    {-4, 20},                   /*  0 */
    {-3, 25},                   /*  1 */
    {-2, 30},
    {-2, 35},                   /*  3 */
    {-1, 40},
    {-1, 45},                   /*  5 */
    {-1, 50},
    {0, 55},
    {0, 60},
    {0, 65},
    {0, 70},                    /* 10 */
    {0, 75},
    {0, 80},
    {0, 85},
    {0, 88},
    {1, 90},                    /* 15 */
    {2, 95},
    {2, 97},
    {3, 99},                    /* 18 */
    {3, 99},
    {4, 99},                    /* 20 */
    {4, 99},
    {5, 99},
    {6, 99},
    {7, 99},
    {8, 99}                     /* 25 Default attainable */
};
#endif

/*
 * Liquid properties.
 * Used in world.obj.
 */
const struct liq_type liq_table[LIQ_MAX] = {
    {"water", "clear", {0, 1, 10}}, /*  0 */
    {"beer", "amber", {3, 2, 5}},
    {"wine", "rose", {5, 2, 5}},
    {"ale", "brown", {2, 2, 5}},
    {"dark ale", "dark", {1, 2, 5}},

    {"whisky", "golden", {6, 1, 4}},    /*  5 */
    {"lemonade", "pink", {0, 1, 8}},
    {"firebreather", "boiling", {10, 0, 0}},
    {"local specialty", "everclear", {3, 3, 3}},
    {"slime mold juice", "green", {0, 4, -8}},

    {"milk", "white", {0, 3, 6}},   /* 10 */
    {"tea", "tan", {0, 1, 6}},
    {"coffee", "black", {0, 1, 6}},
    {"blood", "red", {0, 2, -1}},
    {"salt water", "clear", {0, 1, -2}},

    {"cola", "cherry", {0, 1, 5}}   /* 15 */
};

/*
 * The skill and spell table.
 * Slot numbers must never be changed as they appear in #OBJECTS sections.
 */
#define SLOT(n)	n

struct skill_type skill_table[MAX_SKILL] = {

/*
 * Magic spells.
 */
/*
    {
	"reserved",		{ 999, 999, 999, 999 },	{ 99, 99, 99, 99},
	spell_null,			TAR_IGNORE, POS_STANDING,
	NULL,			SLOT( 0),	 0,	 0,
	"",			""
    },*/

    {
     "acid blast", {28, 53, 35, 32}, {1, 1, 2, 2},
     spell_acid_blast, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT( 70 ), 20, 12,
     "acid blast", "!Acid Blast!"},

    {
     "armor", {7, 2, 10, 5}, {1, 1, 2, 2},
     spell_armor, TAR_CHAR_DEFENSIVE, POS_STANDING,
     NULL, SLOT( 1 ), 5, 0,
     "", "You feel less protected."},

    {
     "bless", {50, 7, 50, 8}, {1, 1, 2, 2},
     spell_bless, TAR_CHAR_DEFENSIVE, POS_STANDING,
     NULL, SLOT( 3 ), 5, 0,
     "", "You feel less righteous."},

    {
     "blindness", {12, 8, 17, 15}, {1, 1, 2, 2},
     spell_blindness, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     &gsn_blindness, SLOT( 4 ), 5, 12,
     "", "You can see again."},

    {
     "burning hands", {7, 50, 10, 9}, {1, 1, 2, 2},
     spell_burning_hands, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT( 5 ), 15, 12,
     "burning hands", "!Burning Hands!"},

    {
     "call lightning", {26, 18, 31, 22}, {1, 1, 2, 2},
     spell_call_lightning, TAR_IGNORE, POS_FIGHTING,
     NULL, SLOT( 6 ), 15, 12,
     "lightning bolt", "!Call Lightning!"},

    {"calm", {50, 16, 50, 20}, {1, 1, 2, 2},
     spell_calm, TAR_IGNORE, POS_FIGHTING,
     NULL, SLOT( 509 ), 30, 4,
     "", "You have lost your peace of mind."},

    {
     "cancellation", {18, 26, 34, 34}, {1, 1, 2, 2},
     spell_cancellation, TAR_CHAR_DEFENSIVE, POS_FIGHTING,
     NULL, SLOT( 507 ), 20, 12,
     "" "!cancellation!",
     },

    {
     "cause critical", {50, 13, 50, 19}, {1, 1, 2, 2},
     spell_cause_critical, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT( 63 ), 20, 12,
     "spell", "!Cause Critical!"},

    {
     "cause light", {50, 1, 50, 3}, {1, 1, 2, 2},
     spell_cause_light, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT( 62 ), 15, 12,
     "spell", "!Cause Light!"},

    {
     "cause serious", {50, 7, 50, 10}, {1, 1, 2, 2},
     spell_cause_serious, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT( 64 ), 17, 12,
     "spell", "!Cause Serious!"},

    {
     "chain lightning", {33, 50, 39, 36}, {1, 1, 2, 2},
     spell_chain_lightning, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT( 500 ), 25, 12,
     "lightning", "!Chain Lightning!"},

    {
     "change sex", {53, 53, 53, 53}, {1, 1, 2, 2},
     spell_change_sex, TAR_CHAR_DEFENSIVE, POS_FIGHTING,
     NULL, SLOT( 82 ), 15, 0,
     "", "Your body feels familiar again."},

    {
     "charm person", {20, 50, 25, 50}, {1, 1, 2, 2},
     spell_charm_person, TAR_CHAR_OFFENSIVE, POS_STANDING,
     &gsn_charm_person, SLOT( 7 ), 5, 12,
     "", "You feel more self-confident."},

    {
     "chill touch", {4, 53, 6, 6}, {1, 1, 2, 2},
     spell_chill_touch, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT( 8 ), 15, 12,
     "chilling touch", "You feel less cold."},

    {
     "colour spray", {16, 50, 22, 20}, {1, 1, 2, 2},
     spell_colour_spray, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT( 10 ), 15, 12,
     "colour spray", "!Colour Spray!"},

    {
     "continual light", {6, 4, 6, 9}, {1, 1, 2, 2},
     spell_continual_light, TAR_IGNORE, POS_STANDING,
     NULL, SLOT( 57 ), 7, 0,
     "", "!Continual Light!"},

    {
     "control weather", {15, 19, 28, 22}, {1, 1, 2, 2},
     spell_control_weather, TAR_IGNORE, POS_STANDING,
     NULL, SLOT( 11 ), 25, 12,
     "", "!Control Weather!"},

    {
     "create food", {10, 5, 11, 12}, {1, 1, 2, 2},
     spell_create_food, TAR_IGNORE, POS_STANDING,
     NULL, SLOT( 12 ), 5, 0,
     "", "!Create Food!"},

    {
     "create spring", {14, 17, 23, 20}, {1, 1, 2, 2},
     spell_create_spring, TAR_IGNORE, POS_STANDING,
     NULL, SLOT( 80 ), 20, 0,
     "", "!Create Spring!"},

    {
     "create water", {8, 3, 12, 11}, {1, 1, 2, 2},
     spell_create_water, TAR_OBJ_INV, POS_STANDING,
     NULL, SLOT( 13 ), 5, 0,
     "", "!Create Water!"},

    {
     "cure blindness", {50, 6, 50, 8}, {1, 1, 2, 2},
     spell_cure_blindness, TAR_CHAR_DEFENSIVE, POS_FIGHTING,
     NULL, SLOT( 14 ), 5, 0,
     "", "!Cure Blindness!"},

    {
     "cure critical", {50, 13, 50, 19}, {1, 1, 2, 2},
     spell_cure_critical, TAR_CHAR_DEFENSIVE, POS_FIGHTING,
     NULL, SLOT( 15 ), 20, 0,
     "", "!Cure Critical!"},

    {
     "cure disease", {50, 13, 50, 14}, {1, 1, 2, 2},
     spell_cure_disease, TAR_CHAR_DEFENSIVE, POS_STANDING,
     NULL, SLOT( 501 ), 20, 0,
     "", "!Cure Disease!"},

    {
     "cure light", {50, 1, 50, 3}, {1, 1, 2, 2},
     spell_cure_light, TAR_CHAR_DEFENSIVE, POS_FIGHTING,
     NULL, SLOT( 16 ), 10, 0,
     "", "!Cure Light!"},

    {
     "cure poison", {50, 14, 50, 16}, {1, 1, 2, 2},
     spell_cure_poison, TAR_CHAR_DEFENSIVE, POS_STANDING,
     NULL, SLOT( 43 ), 5, 0,
     "", "!Cure Poison!"},

    {
     "cure serious", {50, 7, 50, 10}, {1, 1, 2, 2},
     spell_cure_serious, TAR_CHAR_DEFENSIVE, POS_FIGHTING,
     NULL, SLOT( 61 ), 15, 0,
     "", "!Cure Serious!"},

    {
     "curse", {18, 18, 26, 22}, {1, 1, 2, 2},
     spell_curse, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     &gsn_curse, SLOT( 17 ), 20, 12,
     "curse", "The curse wears off."},

    {
     "demonfire", {50, 34, 50, 45}, {1, 1, 2, 2},
     spell_demonfire, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT( 505 ), 20, 12,
     "torments", "!Demonfire!"},

    {
     "detect evil", {12, 4, 12, 53}, {1, 1, 2, 2},
     spell_detect_evil, TAR_CHAR_SELF, POS_STANDING,
     NULL, SLOT( 18 ), 5, 0,
     "", "The red in your vision disappears."},

    {
     "detect hidden", {15, 11, 12, 50}, {1, 1, 2, 2},
     spell_detect_hidden, TAR_CHAR_SELF, POS_STANDING,
     NULL, SLOT( 44 ), 5, 0,
     "", "You feel less aware of your suroundings."},

    {
     "detect invis", {3, 8, 6, 50}, {1, 1, 2, 2},
     spell_detect_invis, TAR_CHAR_SELF, POS_STANDING,
     NULL, SLOT( 19 ), 5, 0,
     "", "You no longer see invisible objects."},

    {
     "detect magic", {2, 6, 5, 50}, {1, 1, 2, 2},
     spell_detect_magic, TAR_CHAR_SELF, POS_STANDING,
     NULL, SLOT( 20 ), 5, 0,
     "", "The detect magic wears off."},

    {
     "detect poison", {15, 7, 9, 50}, {1, 1, 2, 2},
     spell_detect_poison, TAR_OBJ_INV, POS_STANDING,
     NULL, SLOT( 21 ), 5, 0,
     "", "!Detect Poison!"},

    {
     "dispel evil", {50, 15, 50, 21}, {1, 1, 2, 2},
     spell_dispel_evil, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT( 22 ), 15, 12,
     "dispel evil", "!Dispel Evil!"},

    {
     "dispel magic", {16, 24, 30, 30}, {1, 1, 2, 2},
     spell_dispel_magic, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT( 59 ), 15, 12,
     "", "!Dispel Magic!"},

    {
     "earthquake", {50, 10, 50, 14}, {1, 1, 2, 2},
     spell_earthquake, TAR_IGNORE, POS_FIGHTING,
     NULL, SLOT( 23 ), 15, 12,
     "earthquake", "!Earthquake!"},

    {
     "enchant armor", {16, 50, 50, 50}, {1, 1, 2, 2},
     spell_enchant_armor, TAR_OBJ_INV, POS_STANDING,
     NULL, SLOT( 510 ), 100, 0,
     "", "!Enchant Armor!"},

    {
     "enchant weapon", {17, 50, 50, 50}, {1, 1, 2, 2},
     spell_enchant_weapon, TAR_OBJ_INV, POS_STANDING,
     NULL, SLOT( 24 ), 100, 0,
     "", "!Enchant Weapon!"},

    {
     "energy drain", {19, 22, 26, 23}, {1, 1, 2, 2},
     spell_energy_drain, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT( 25 ), 35, 12,
     "energy drain", "!Energy Drain!"},

    {
     "faerie fire", {6, 3, 5, 8}, {1, 1, 2, 2},
     spell_faerie_fire, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT( 72 ), 5, 12,
     "faerie fire", "The pink aura around you fades away."},

    {
     "faerie fog", {14, 21, 16, 24}, {1, 1, 2, 2},
     spell_faerie_fog, TAR_IGNORE, POS_STANDING,
     NULL, SLOT( 73 ), 12, 12,
     "faerie fog", "!Faerie Fog!"},

    {
     "fireball", {22, 50, 30, 26}, {1, 1, 2, 2},
     spell_fireball, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT( 26 ), 15, 12,
     "fireball", "!Fireball!"},

    {
     "flamestrike", {50, 20, 50, 27}, {1, 1, 2, 2},
     spell_flamestrike, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT( 65 ), 20, 12,
     "flamestrike", "!Flamestrike!"},

    {
     "fly", {10, 18, 20, 22}, {1, 1, 2, 2},
     spell_fly, TAR_CHAR_DEFENSIVE, POS_STANDING,
     NULL, SLOT( 56 ), 10, 0,
     "", "You slowly float to the ground."},

    {
     "frenzy", {50, 24, 50, 26}, {1, 1, 2, 2},
     spell_frenzy, TAR_CHAR_DEFENSIVE, POS_STANDING,
     NULL, SLOT( 504 ), 30, 24,
     "", "Your rage ebbs."},

    {
     "gate", {27, 17, 32, 28}, {1, 1, 2, 2},
     spell_gate, TAR_IGNORE, POS_FIGHTING,
     NULL, SLOT( 83 ), 80, 0,
     "", "!Gate!"},

    {
     "nexus", {32, 22, 37, 33}, {1, 1, 2, 2},
     spell_nexus, TAR_IGNORE, POS_FIGHTING,
     NULL, SLOT( 84 ), 180, 0,
     "", "!Nexus!"},

    {
     "giant strength", {11, 19, 22, 20}, {1, 1, 2, 2},
     spell_giant_strength, TAR_CHAR_DEFENSIVE, POS_STANDING,
     NULL, SLOT( 39 ), 20, 0,
     "", "You feel weaker."},

    {
     "harm", {50, 23, 50, 28}, {1, 1, 2, 2},
     spell_harm, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT( 27 ), 35, 12,
     "harm spell", "!Harm!"},

    {
     "haste", {21, 29, 26, 29}, {1, 1, 2, 2},
     spell_haste, TAR_CHAR_DEFENSIVE, POS_FIGHTING,
     NULL, SLOT( 502 ), 30, 0,
     "", "You feel yourself slow down."},

    {
     "heal", {50, 21, 50, 30}, {1, 1, 2, 2},
     spell_heal, TAR_CHAR_DEFENSIVE, POS_FIGHTING,
     NULL, SLOT( 28 ), 50, 0,
     "", "!Heal!"},

    {
     "holy word", {50, 36, 50, 42}, {2, 2, 4, 4},
     spell_holy_word, TAR_IGNORE, POS_FIGHTING,
     NULL, SLOT( 506 ), 200, 24,
     "divine wrath", "!Holy Word!"},

    {
     "identify", {15, 16, 18, 50}, {1, 1, 2, 2},
     spell_identify, TAR_OBJ_INV, POS_STANDING,
     NULL, SLOT( 53 ), 12, 0,
     "", "!Identify!"},

    {
     "infravision", {9, 13, 10, 16}, {1, 1, 2, 2},
     spell_infravision, TAR_CHAR_DEFENSIVE, POS_STANDING,
     NULL, SLOT( 77 ), 5, 0,
     "", "You no longer see in the dark."},

    {
     "invis", {5, 50, 9, 50}, {1, 1, 2, 2},
     spell_invis, TAR_CHAR_DEFENSIVE, POS_STANDING,
     &gsn_invis, SLOT( 29 ), 5, 0,
     "", "You are no longer invisible."},

    {
     "know alignment", {12, 9, 20, 50}, {1, 1, 2, 2},
     spell_know_alignment, TAR_CHAR_DEFENSIVE, POS_FIGHTING,
     NULL, SLOT( 58 ), 9, 0,
     "", "!Know Alignment!"},

    {
     "lightning bolt", {13, 23, 18, 16}, {1, 1, 2, 2},
     spell_lightning_bolt, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT( 30 ), 15, 12,
     "lightning bolt", "!Lightning Bolt!"},

    {
     "locate object", {9, 15, 11, 50}, {1, 1, 2, 2},
     spell_locate_object, TAR_IGNORE, POS_STANDING,
     NULL, SLOT( 31 ), 20, 0,
     "", "!Locate Object!"},

    {
     "magic missile", {1, 50, 2, 2}, {1, 1, 2, 2},
     spell_magic_missile, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT( 32 ), 15, 12,
     "magic missile", "!Magic Missile!"},

    {
     "mass healing", {50, 38, 50, 46}, {2, 2, 4, 4},
     spell_mass_healing, TAR_IGNORE, POS_STANDING,
     NULL, SLOT( 508 ), 100, 0,
     "", "!Mass Healing!"},

    {
     "mass invis", {22, 25, 31, 50}, {1, 1, 2, 2},
     spell_mass_invis, TAR_IGNORE, POS_STANDING,
     &gsn_mass_invis, SLOT( 69 ), 20, 0,
     "", "!Mass Invis!"},

    {
     "pass door", {24, 32, 25, 37}, {1, 1, 2, 2},
     spell_pass_door, TAR_CHAR_SELF, POS_STANDING,
     NULL, SLOT( 74 ), 20, 0,
     "", "You feel solid again."},

    {
     "plague", {23, 17, 36, 26}, {1, 1, 2, 2},
     spell_plague, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     &gsn_plague, SLOT( 503 ), 20, 12,
     "sickness", "Your sores vanish."},

    {
     "poison", {17, 12, 15, 21}, {1, 1, 2, 2},
     spell_poison, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     &gsn_poison, SLOT( 33 ), 10, 12,
     "poison", "You feel less sick."},

    {
     "protection evil", {12, 9, 17, 11}, {1, 1, 2, 2},
     spell_protection, TAR_CHAR_SELF, POS_STANDING,
     NULL, SLOT( 34 ), 5, 0,
     "", "You feel less protected."},

    {
     "refresh", {8, 5, 12, 9}, {1, 1, 2, 2},
     spell_refresh, TAR_CHAR_DEFENSIVE, POS_STANDING,
     NULL, SLOT( 81 ), 12, 0,
     "refresh", "!Refresh!"},

    {
     "remove curse", {50, 18, 50, 22}, {1, 1, 2, 2},
     spell_remove_curse, TAR_CHAR_DEFENSIVE, POS_STANDING,
     NULL, SLOT( 35 ), 5, 0,
     "", "!Remove Curse!"},

    {
     "sanctuary", {36, 20, 42, 30}, {1, 1, 2, 2},
     spell_sanctuary, TAR_CHAR_DEFENSIVE, POS_STANDING,
     NULL, SLOT( 36 ), 75, 0,
     "", "The white aura around your body fades."},

    {
     "shield", {20, 35, 35, 40}, {1, 1, 2, 2},
     spell_shield, TAR_CHAR_DEFENSIVE, POS_STANDING,
     NULL, SLOT( 67 ), 12, 0,
     "", "Your force shield shimmers then fades away."},

    {
     "shocking grasp", {10, 50, 14, 13}, {1, 1, 2, 2},
     spell_shocking_grasp, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT( 54 ), 15, 12,
     "shocking grasp", "!Shocking Grasp!"},

    {
     "sleep", {10, 50, 11, 50}, {1, 1, 2, 2},
     spell_sleep, TAR_CHAR_OFFENSIVE, POS_STANDING,
     &gsn_sleep, SLOT( 38 ), 15, 12,
     "", "You feel less tired."},

    {
     "stone skin", {25, 40, 40, 45}, {1, 1, 2, 2},
     spell_stone_skin, TAR_CHAR_SELF, POS_STANDING,
     NULL, SLOT( 66 ), 12, 0,
     "", "Your skin feels soft again."},

    {
     "summon", {24, 12, 29, 22}, {1, 1, 2, 2},
     spell_summon, TAR_IGNORE, POS_STANDING,
     NULL, SLOT( 40 ), 50, 12,
     "", "!Summon!"},

    {
     "teleport", {13, 22, 25, 36}, {1, 1, 2, 2},
     spell_teleport, TAR_CHAR_SELF, POS_FIGHTING,
     NULL, SLOT( 2 ), 35, 0,
     "", "!Teleport!"},

    {
     "ventriloquate", {1, 50, 2, 50}, {1, 1, 2, 2},
     spell_ventriloquate, TAR_IGNORE, POS_STANDING,
     NULL, SLOT( 41 ), 5, 0,
     "", "!Ventriloquate!"},

    {
     "weaken", {11, 14, 16, 17}, {1, 1, 2, 2},
     spell_weaken, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT( 68 ), 20, 12,
     "spell", "You feel stronger."},

    {
     "word of recall", {32, 28, 40, 30}, {1, 1, 2, 2},
     spell_word_of_recall, TAR_CHAR_SELF, POS_RESTING,
     NULL, SLOT( 42 ), 5, 0,
     "", "!Word of Recall!"},

/*
 * Dragon breath
 */
    {
     "acid breath", {37, 50, 50, 50}, {1, 1, 2, 2},
     spell_acid_breath, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT( 200 ), 0, 4,
     "blast of acid", "!Acid Breath!"},

    {
     "fire breath", {39, 50, 50, 50}, {1, 1, 2, 2},
     spell_fire_breath, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT( 201 ), 0, 4,
     "blast of flame", "!Fire Breath!"},

    {
     "frost breath", {31, 50, 50, 50}, {1, 1, 2, 2},
     spell_frost_breath, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT( 202 ), 0, 4,
     "blast of frost", "!Frost Breath!"},

    {
     "gas breath", {40, 50, 50, 50}, {1, 1, 2, 2},
     spell_gas_breath, TAR_IGNORE, POS_FIGHTING,
     NULL, SLOT( 203 ), 0, 4,
     "blast of gas", "!Gas Breath!"},

    {
     "lightning breath", {34, 50, 50, 50}, {1, 1, 2, 2},
     spell_lightning_breath, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT( 204 ), 0, 4,
     "blast of lightning", "!Lightning Breath!"},

/*
 * Spells for mega1.are from Glop/Erkenbrand.
 */
    {
     "general purpose", {50, 50, 50, 50}, {0, 0, 0, 0},
     spell_general_purpose, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT( 401 ), 0, 12,
     "general purpose ammo", "!General Purpose Ammo!"},

    {
     "high explosive", {50, 50, 50, 50}, {0, 0, 0, 0},
     spell_high_explosive, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT( 402 ), 0, 12,
     "high explosive ammo", "!High Explosive Ammo!"},

/*
 * Spells added by Thexder
 */

    {
     "firewind", {45, 50, 50, 50}, {1, 1, 2, 2},
     spell_firewind, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT( 400 ), 33, 12,
     "flaming winds", "!Firewind"},
    {
     "meteor swarm", {38, 50, 50, 50}, {1, 1, 2, 2},
     spell_meteor_swarm, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT( 401 ), 25, 12,
     "fireball", "!Fireball"},
    {
     "multi missile", {9, 50, 50, 50}, {1, 1, 2, 2},
     spell_multi_missile, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT( 402 ), 25, 12,
     "magic missile", "!Missile"},
    {
     "disintegrate ", {38, 50, 50, 50}, {1, 1, 2, 2},
     spell_disintegrate, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT( 403 ), 25, 12,
     "energy blast", "!Disint"},
    {
     "ice ray", {33, 50, 50, 50}, {1, 1, 2, 2},
     spell_ice_ray, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT( 404 ), 20, 8,
     "ice ray", "The chill leaves your body."},
    {
     "holyfire", {14, 50, 50, 50}, {1, 1, 2, 2},
     spell_holyfire, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT( 405 ), 25, 12,
     "flames", "!Holyfire"},
    {
     "ice storm", {47, 50, 50, 50}, {1, 1, 2, 2},
     spell_ice_storm, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT( 406 ), 33, 8,
     "ice ray", "The chill leaves your body."},
    {
     "vision", {27, 17, 32, 28}, {1, 1, 2, 2},
     spell_vision, TAR_IGNORE, POS_FIGHTING,
     NULL, SLOT( 407 ), 40, 20,
     "", "!Vision!"},
    {
     "restoration", {50, 45, 50, 50}, {1, 1, 2, 2},
     spell_restoration, TAR_CHAR_DEFENSIVE, POS_FIGHTING,
     NULL, SLOT( 408 ), 75, 0,
     "", "!Restore!"},
    {
     "regeneration", {50, 25, 40, 40}, {1, 1, 2, 2},
     spell_regeneration, TAR_CHAR_DEFENSIVE, POS_STANDING,
     NULL, SLOT( 409 ), 50, 0,
     "", "Your body slows down."},
    {
     "test area", {14, 50, 50, 50}, {1, 1, 2, 2},
     spell_test_area, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT( 410 ), 25, 12,
     "breath", "!TEST"},
    {
     "web", {11, 19, 22, 20}, {1, 1, 2, 2},
     spell_web, TAR_CHAR_DEFENSIVE, POS_STANDING,
     NULL, SLOT( 511 ), 20, 0,
     "", "The webs disolve."},

/* Spells donated  by Slaythar (slaythar@earthlink.net) */

    {
     "life drain", {20, 30, 50, 50}, {1, 1, 2, 2},
     spell_life_drain, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT( 512 ), 75, 12,
     "life drain", "!Drain!"},

    {
     "energize", {30, 30, 50, 50}, {1, 1, 2, 2},
     spell_energize, TAR_CHAR_DEFENSIVE, POS_STANDING,
     NULL, SLOT( 513 ), 25, 0,
     "", "!Energize!"},

/* combat and weapons skills */

    {
     "axe", {1, 1, 1, 1}, {6, 6, 5, 4},
     spell_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_axe, SLOT( 0 ), 0, 0,
     "", "!Axe!"},

    {
     "dagger", {1, 1, 1, 1}, {2, 3, 2, 2},
     spell_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_dagger, SLOT( 0 ), 0, 0,
     "", "!Dagger!"},

    {
     "flail", {1, 1, 1, 1}, {6, 3, 6, 4},
     spell_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_flail, SLOT( 0 ), 0, 0,
     "", "!Flail!"},

    {
     "mace", {1, 1, 1, 1}, {5, 2, 3, 3},
     spell_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_mace, SLOT( 0 ), 0, 0,
     "", "!Mace!"},

    {
     "polearm", {1, 1, 1, 1}, {6, 6, 6, 4},
     spell_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_polearm, SLOT( 0 ), 0, 0,
     "", "!Polearm!"},

    {
     "shield block", {1, 1, 1, 1}, {6, 4, 6, 2},
     spell_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_shield_block, SLOT( 0 ), 0, 0,
     "", "!Shield!"},

    {
     "counter", {1, 1, 1, 1}, {0, 0, 0, 6},
     spell_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_counter, SLOT( 0 ), 0, 0,
     "", "!Counter!"},

    {
     "grip", {1, 1, 1, 1}, {0, 0, 0, 6},
     spell_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_grip, SLOT( 0 ), 0, 18,
     "", "!Grip!"},

    {
     "spear", {1, 1, 1, 1}, {4, 4, 4, 3},
     spell_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_spear, SLOT( 0 ), 0, 0,
     "", "!Spear!"},

    {
     "sword", {1, 1, 1, 1}, {5, 6, 3, 2},
     spell_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_sword, SLOT( 0 ), 0, 0,
     "", "!sword!"},

    {
     "whip", {1, 1, 1, 1}, {6, 5, 5, 4},
     spell_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_whip, SLOT( 0 ), 0, 0,
     "", "!Whip!"},

    {
     "backstab", {50, 50, 1, 50}, {0, 0, 5, 0},
     spell_null, TAR_IGNORE, POS_STANDING,
     &gsn_backstab, SLOT( 0 ), 0, 36,
     "backstab", "!Backstab!"},

    {
     "blind fighting", {50, 50, 1, 50}, {0, 0, 0, 5},
     spell_null, TAR_IGNORE, POS_STANDING,
     &gsn_blind_fighting, SLOT( 0 ), 0, 36,
     "", "!Blind_Fighting!"},

    {
     "shield cleave", {50, 50, 1, 50}, {0, 0, 0, 3},
     spell_null, TAR_IGNORE, POS_STANDING,
     &gsn_shield_cleave, SLOT( 0 ), 0, 5,
     "", "!Shield_Cleave!"},

    {
     "weapon cleave", {50, 50, 1, 50}, {0, 0, 0, 7},
     spell_null, TAR_IGNORE, POS_STANDING,
     &gsn_weapon_cleave, SLOT( 0 ), 0, 25,
     "", "!Weapon_Cleave!"},

    {
     "bash", {50, 50, 50, 1}, {0, 0, 0, 4},
     spell_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_bash, SLOT( 0 ), 0, 24,
     "bash", "!Bash!"},

    {
     "berserk", {50, 50, 50, 18}, {0, 0, 0, 5},
     spell_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_berserk, SLOT( 0 ), 0, 24,
     "", "You feel your pulse slow down."},

    {
     "blackjack", {50, 50, 3, 3}, {0, 0, 4, 4},
     spell_null, TAR_IGNORE, POS_STANDING,
     &gsn_blackjack, SLOT( 0 ), 0, 24,
     "blackjack", "Man what a headache, good thing its over."},

    {
     "dirt kicking", {50, 50, 3, 3}, {0, 0, 4, 4},
     spell_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_dirt, SLOT( 0 ), 0, 24,
     "kicked dirt", "You rub the dirt out of your eyes."},

    {
     "disarm", {50, 50, 12, 11}, {0, 0, 6, 4},
     spell_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_disarm, SLOT( 0 ), 0, 24,
     "", "!Disarm!"},

    {
     "dodge", {20, 22, 1, 13}, {8, 8, 4, 6},
     spell_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_dodge, SLOT( 0 ), 0, 0,
     "", "!Dodge!"},

    {
     "enhanced damage", {50, 30, 25, 1}, {0, 9, 5, 3},
     spell_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_enhanced_damage, SLOT( 0 ), 0, 0,
     "", "!Enhanced Damage!"},

    {
     "hand to hand", {25, 10, 15, 6}, {8, 5, 6, 4},
     spell_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_hand_to_hand, SLOT( 0 ), 0, 0,
     "", "!Hand to Hand!"},

    {
     "kick", {50, 12, 14, 8}, {0, 4, 6, 3},
     spell_null, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     &gsn_kick, SLOT( 0 ), 0, 12,
     "kick", "!Kick!"},

    {
     "parry", {22, 20, 13, 1}, {8, 8, 6, 4},
     spell_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_parry, SLOT( 0 ), 0, 0,
     "", "!Parry!"},

    {
     "rescue", {50, 50, 50, 1}, {0, 0, 0, 4},
     spell_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_rescue, SLOT( 0 ), 0, 12,
     "", "!Rescue!"},

    {
     "trip", {50, 50, 1, 15}, {0, 0, 4, 8},
     spell_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_trip, SLOT( 0 ), 0, 24,
     "trip", "!Trip!"},

    {
     "second attack", {30, 24, 12, 5}, {10, 8, 5, 3},
     spell_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_second_attack, SLOT( 0 ), 0, 0,
     "", "!Second Attack!"},

    {
     "third attack", {50, 50, 24, 12}, {0, 0, 10, 4},
     spell_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_third_attack, SLOT( 0 ), 0, 0,
     "", "!Third Attack!"},

/* non-combat skills */

    {
     "fast healing", {15, 9, 16, 6}, {8, 5, 6, 4},
     spell_null, TAR_IGNORE, POS_SLEEPING,
     &gsn_fast_healing, SLOT( 0 ), 0, 0,
     "", "!Fast Healing!"},

    {
     "haggle", {7, 18, 1, 14}, {5, 8, 3, 6},
     spell_null, TAR_IGNORE, POS_RESTING,
     &gsn_haggle, SLOT( 0 ), 0, 0,
     "", "!Haggle!"},

    {
     "hide", {50, 50, 1, 12}, {0, 0, 4, 6},
     spell_null, TAR_IGNORE, POS_RESTING,
     &gsn_hide, SLOT( 0 ), 0, 0,
     "", "!Hide!"},

    {
     "lore", {10, 10, 6, 20}, {6, 6, 4, 8},
     spell_null, TAR_IGNORE, POS_RESTING,
     &gsn_lore, SLOT( 0 ), 0, 0,
     "", "!Lore!"},

    {
     "meditation", {6, 6, 15, 15}, {5, 5, 8, 8},
     spell_null, TAR_IGNORE, POS_SLEEPING,
     &gsn_meditation, SLOT( 0 ), 0, 0,
     "", "Meditation"},

    {
     "peek", {8, 21, 1, 14}, {5, 7, 3, 6},
     spell_null, TAR_IGNORE, POS_STANDING,
     &gsn_peek, SLOT( 0 ), 0, 0,
     "", "!Peek!"},

    {
     "pick lock", {25, 25, 7, 25}, {8, 8, 4, 8},
     spell_null, TAR_IGNORE, POS_STANDING,
     &gsn_pick_lock, SLOT( 0 ), 0, 0,
     "", "!Pick!"},

    {
     "sneak", {50, 50, 4, 10}, {0, 0, 4, 6},
     spell_null, TAR_IGNORE, POS_STANDING,
     &gsn_sneak, SLOT( 0 ), 0, 0,
     "", "You no longer feel stealthy."},

    {
     "steal", {50, 50, 5, 50}, {0, 0, 4, 0},
     spell_null, TAR_IGNORE, POS_STANDING,
     &gsn_steal, SLOT( 0 ), 0, 24,
     "", "!Steal!"},

    {
     "scrolls", {1, 1, 1, 1}, {2, 3, 5, 8},
     spell_null, TAR_IGNORE, POS_STANDING,
     &gsn_scrolls, SLOT( 0 ), 0, 0,
     "", "!Scrolls!"},

    {
     "staves", {1, 1, 1, 1}, {2, 3, 5, 8},
     spell_null, TAR_IGNORE, POS_STANDING,
     &gsn_staves, SLOT( 0 ), 0, 0,
     "", "!Staves!"},

    {
     "wands", {1, 1, 1, 1}, {2, 3, 5, 8},
     spell_null, TAR_IGNORE, POS_STANDING,
     &gsn_wands, SLOT( 0 ), 0, 0,
     "", "!Wands!"},

    {
     "recall", {1, 1, 1, 1}, {2, 2, 2, 2},
     spell_null, TAR_IGNORE, POS_STANDING,
     &gsn_recall, SLOT( 0 ), 0, 0,
     "", "!Recall!"},
#ifdef TRACK_IS_SKILL
    {
     "track", {80, 80, 25, 35}, {8, 5, 3, 2},
     spell_null, TAR_IGNORE, POS_STANDING,
     &gsn_track, SLOT( 0 ), 0, 0,
     "", "!track!"},
#endif

    {
     "brew", {25, 30, 80, 80}, {2, 3, 5, 8},
     spell_null, TAR_IGNORE, POS_STANDING,
     &gsn_brew, SLOT( 0 ), 0, 24,
     "", "!Brew!"},

    {
     "scribe", {25, 35, 80, 80}, {2, 3, 5, 8},
     spell_null, TAR_IGNORE, POS_STANDING,
     &gsn_scribe, SLOT( 0 ), 0, 24,
     "", "!Scribe!"}

};

const struct group_type group_table[MAX_GROUP] = {

    {
     "rom basics", {0, 0, 0, 0},
     {"scrolls", "staves", "wands", "recall"}
     },

    {
     "mage basics", {0, -1, -1, -1},
     {"dagger"}
     },

    {
     "cleric basics", {-1, 0, -1, -1},
     {"mace"}
     },

    {
     "thief basics", {-1, -1, 0, -1},
     {"dagger", "steal"}
     },

    {
     "warrior basics", {-1, -1, -1, 0},
     {"sword", "second attack"}
     },

    {
     "mage default", {40, -1, -1, -1},
     {"lore", "beguiling", "combat", "detection", "enhancement", "illusion",
      "maladictions", "protective", "transportation", "weather"}
     },

    {
     "cleric default", {-1, 40, -1, -1},
     {"flail", "attack", "creation", "curative", "benedictions",
      "detection", "healing", "maladictions", "protective", "shield block",
      "transportation", "weather"}
     },

    {
     "thief default", {-1, -1, 40, -1},
     {"whip", "sword", "backstab", "disarm", "dodge", "second attack",
      "trip", "hide", "peek", "pick lock", "sneak"}
     },

    {
     "warrior default", {-1, -1, -1, 40},
     {"weaponsmaster", "shield block", "bash", "disarm", "enhanced damage",
      "parry", "rescue", "third attack"}
     },

    {
     "weaponsmaster", {40, 40, 40, 20},
     {"axe", "dagger", "flail", "mace", "polearm", "spear", "sword", "whip"}
     },

    {
     "attack", {-1, 5, -1, -1},
     {"demonfire", "dispel evil", "earthquake", "flamestrike"}
     },

    {
     "beguiling", {4, -1, -1, -1},
     {"charm person", "sleep"}
     },

    {
     "benedictions", {-1, 4, -1, -1},
     {"bless", "calm", "frenzy", "holy word", "remove curse"}
     },
    {
     "combat", {6, -1, -1, -1},
     {"acid blast", "burning hands", "chain lightning", "chill touch",
      "colour spray", "fireball", "lightning bolt", "magic missile",
      "shocking grasp"}
     },

    {
     "creation", {4, 4, -1, -1},
     {"continual light", "create food", "create water", "create spring"}
     },

    {
     "curative", {-1, 4, -1, -1},
     {"cure blindness", "cure disease", "cure poison"}
     },

    {
     "detection", {4, 3, -1, -1},
     {"detect evil", "detect invis", "detect magic",
      "detect poison", "identify", "know alignment", "locate object"}
     },

    {
     "draconian", {8, -1, -1, -1},
     {"acid breath", "fire breath", "frost breath", "gas breath",
      "lightning breath"}
     },

    {
     "enchantment", {6, -1, -1, -1},
     {"enchant armor", "enchant weapon"}
     },

    {
     "enhancement", {5, -1, -1, -1},
     {"giant strength", "haste", "infravision", "refresh"}
     },

    {
     "harmful", {-1, 3, -1, -1},
     {"cause critical", "cause light", "cause serious", "harm"}
     },

    {
     "healing", {-1, 3, -1, -1},
     {"cure critical", "cure light", "cure serious", "heal",
      "mass healing", "refresh", "energize"}
     },

    {
     "illusion", {4, -1, 3, -1},
     {"invis", "mass invis", "ventriloquate"}
     },

    {
     "maladictions", {5, 5, -1, -1},
     {"blindness", "change sex", "curse", "energy drain", "plague",
      "poison", "weaken", "web", "life drain"}
     },

    {
     "protective", {4, 4, -1, -1},
     {"armor", "cancellation", "dispel magic", "protection evil",
      "sanctuary", "shield", "stone skin"}
     },

    {
     "transportation", {4, 4, -1, -1},
     {"fly", "gate", "nexus", "pass door", "summon", "teleport",
      "word of recall"}
     },

    {
     "weather", {4, 4, -1, -1},
     {"call lightning", "control weather", "faerie fire", "faerie fog",
      "lightning bolt"}
     }

};
