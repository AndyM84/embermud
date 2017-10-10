
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
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "olc.h"

/* command procedures needed */
DECLARE_DO_FUN( do_return );

AFFECT_DATA *affect_free;
NEWAFFECT_DATA *newaffect_free;

char thedate[10];
char thetime[7];

/*
 * Local functions.
 */
void affect_modify args( ( CHAR_DATA * ch, AFFECT_DATA * paf, bool fAdd ) );

/* get_date() - Return current date in mm/dd/yy format */
char *get_curdate(  )
{
    time_t tm;
    struct tm now;

    thedate[9] = '\0';
    time( &tm );
    now = *localtime( &tm );
    sprintf( thedate, "%02d/%02d/%02d", now.tm_mon + 1, now.tm_mday,
             now.tm_year );

    return thedate;
}

/* get_curtime() - Return current time in hh:mm format */
char *get_curtime(  )
{
    time_t tm;
    struct tm now;

    thetime[6] = '\0';
    time( &tm );
    now = *localtime( &tm );

    if ( now.tm_hour == 0 )
    {
        now.tm_hour = 12;
        sprintf( thetime, "%02d:%02dam", now.tm_hour, now.tm_min );
    }
    else if ( now.tm_hour > 12 )
    {
        now.tm_hour = now.tm_hour - 12;
        sprintf( thetime, "%02d:%02dpm", now.tm_hour, now.tm_min );
    }
    else if ( now.tm_hour == 12 )
    {
        sprintf( thetime, "%02d:%02dpm", now.tm_hour, now.tm_min );
    }
    else if ( now.tm_hour < 12 )
    {
        sprintf( thetime, "%02d:%02dam", now.tm_hour, now.tm_min );
    }
    return thetime;
}

char *get_date( time_t tm )
{
    struct tm then;

    thedate[9] = '\0';
    then = *localtime( &tm );
    sprintf( thedate, "%02d/%02d/%02d", then.tm_mon + 1, then.tm_mday,
             then.tm_year % 100 );

    return thedate;
}

char *get_time( time_t tm )
{
    struct tm stuff;

    thetime[6] = '\0';
    stuff = *localtime( &tm );
    sprintf( thetime, "%02d:%02d", stuff.tm_hour, stuff.tm_min );

    return thetime;
}

/* returns number of people on an object */
int count_users( OBJ_DATA * obj )
{
    CHAR_DATA *fch;
    int count = 0;

    if ( obj->in_room == NULL )
        return 0;

    for ( fch = obj->in_room->people; fch != NULL; fch = fch->next_in_room )
        if ( fch->on == obj )
            count++;

    return count;
}

/* returns material number */
int material_lookup( const char *name )
{
    return 0;
}

/* returns material name -- ROM OLC temp patch */
char *material_name( sh_int num )
{
    return "unknown";
}

/* returns race number */
int race_lookup( const char *name )
{
    int race;

    for ( race = 0; race_table[race].name != NULL; race++ )
    {
        if ( LOWER( name[0] ) == LOWER( race_table[race].name[0] )
             && !str_prefix( name, race_table[race].name ) )
            return race;
    }

    return 0;
}

/* returns class number */
int class_lookup( const char *name )
{
    int Class;

    for ( Class = 0; Class < MAX_CLASS; Class++ )
    {
        if ( !str_prefix( name, class_table[Class].name ) )
            return Class;
    }

    return -1;
}

/* for immunity, vulnerabiltiy, and resistant
 * the 'globals' (magic and weapons) may be overriden
 * three other cases -- wood, silver, and iron -- are checked in fight.c */

int check_immune( CHAR_DATA * ch, int dam_type )
{
    int immune;
    int bit;

    immune = IS_NORMAL;

    if ( dam_type == DAM_NONE )
        return immune;

    if ( dam_type <= 3 )
    {
        if ( IS_SET( ch->imm_flags, IMM_WEAPON ) )
            immune = IS_IMMUNE;
        else if ( IS_SET( ch->res_flags, RES_WEAPON ) )
            immune = IS_RESISTANT;
        else if ( IS_SET( ch->vuln_flags, VULN_WEAPON ) )
            immune = IS_VULNERABLE;
    }
    else                        /* magical attack */
    {
        if ( IS_SET( ch->imm_flags, IMM_MAGIC ) )
            immune = IS_IMMUNE;
        else if ( IS_SET( ch->res_flags, RES_MAGIC ) )
            immune = IS_RESISTANT;
        else if ( IS_SET( ch->vuln_flags, VULN_MAGIC ) )
            immune = IS_VULNERABLE;
    }

    /* set bits to check -- VULN etc. must ALL be the same or this will fail */
    switch ( dam_type )
    {
    case ( DAM_BASH ):
        bit = IMM_BASH;
        break;
    case ( DAM_PIERCE ):
        bit = IMM_PIERCE;
        break;
    case ( DAM_SLASH ):
        bit = IMM_SLASH;
        break;
    case ( DAM_FIRE ):
        bit = IMM_FIRE;
        break;
    case ( DAM_COLD ):
        bit = IMM_COLD;
        break;
    case ( DAM_LIGHTNING ):
        bit = IMM_LIGHTNING;
        break;
    case ( DAM_ACID ):
        bit = IMM_ACID;
        break;
    case ( DAM_POISON ):
        bit = IMM_POISON;
        break;
    case ( DAM_NEGATIVE ):
        bit = IMM_NEGATIVE;
        break;
    case ( DAM_HOLY ):
        bit = IMM_HOLY;
        break;
    case ( DAM_ENERGY ):
        bit = IMM_ENERGY;
        break;
    case ( DAM_MENTAL ):
        bit = IMM_MENTAL;
        break;
    case ( DAM_DISEASE ):
        bit = IMM_DISEASE;
        break;
    case ( DAM_DROWNING ):
        bit = IMM_DROWNING;
        break;
    case ( DAM_LIGHT ):
        bit = IMM_LIGHT;
        break;
    default:
        return immune;
    }

    if ( IS_SET( ch->imm_flags, bit ) )
        immune = IS_IMMUNE;
    else if ( IS_SET( ch->res_flags, bit ) )
        immune = IS_RESISTANT;
    else if ( IS_SET( ch->vuln_flags, bit ) )
        immune = IS_VULNERABLE;

    return immune;
}

/* for returning skill information */
int get_skill( CHAR_DATA * ch, int sn )
{
    int skill;

    if ( sn == -1 )             /* shorthand for level based skills */
    {
        skill = ch->level * 5 / 2;
    }

    else if ( sn < -1 || sn > MAX_SKILL )
    {
        bug( "Bad sn %d in get_skill.", sn );
        skill = 0;
    }

    else if ( !IS_NPC( ch ) )
    {
        if ( ch->level < skill_table[sn].skill_level[ch->Class] )
            skill = 0;
        else
            skill = ch->pcdata->learned[sn];
    }

    else                        /* mobiles */
    {

        if ( sn == gsn_sneak )
            skill = ch->level * 2 + 20;

        if ( sn == gsn_second_attack
             && ( IS_SET( ch->act, ACT_WARRIOR )
                  || IS_SET( ch->act, ACT_THIEF ) ) )
            skill = 10 + 3 * ch->level;

        else if ( sn == gsn_third_attack && IS_SET( ch->act, ACT_WARRIOR ) )
            skill = 4 * ch->level - 40;

        else if ( sn == gsn_fourth_attack && IS_SET( ch->act, ACT_WARRIOR ) )
            skill = 4 * ch->level - 50;

        else if ( sn == gsn_hand_to_hand )
            skill = 40 + 2 * ch->level;

        else if ( sn == gsn_trip && IS_SET( ch->off_flags, OFF_TRIP ) )
            skill = 10 + 3 * ch->level;

        else if ( sn == gsn_bash && IS_SET( ch->off_flags, OFF_BASH ) )
            skill = 10 + 3 * ch->level;

        else if ( sn == gsn_disarm
                  && ( IS_SET( ch->off_flags, OFF_DISARM )
                       || IS_SET( ch->off_flags, ACT_WARRIOR )
                       || IS_SET( ch->off_flags, ACT_THIEF ) ) )
            skill = 20 + 3 * ch->level;

        else if ( sn == gsn_grip
                  && ( IS_SET( ch->act, ACT_WARRIOR )
                       || IS_SET( ch->act, ACT_THIEF ) ) )
            skill = ch->level;

        else if ( sn == gsn_berserk && IS_SET( ch->off_flags, OFF_BERSERK ) )
            skill = 3 * ch->level;

        else if ( sn == gsn_sword
                  || sn == gsn_dagger
                  || sn == gsn_spear
                  || sn == gsn_mace
                  || sn == gsn_axe
                  || sn == gsn_flail || sn == gsn_whip || sn == gsn_polearm )
            skill = 40 + 5 * ch->level / 2;

        else
            skill = 0;
    }

    if ( IS_AFFECTED( ch, AFF_BERSERK ) )
        skill -= ch->level / 2;

    return URANGE( 0, skill, 100 );
}

/* for returning weapon information */
int get_weapon_sn( CHAR_DATA * ch )
{
    OBJ_DATA *wield;
    int sn;

    wield = get_eq_char( ch, WEAR_WIELD );
    if ( wield == NULL || wield->item_type != ITEM_WEAPON )
        sn = gsn_hand_to_hand;
    else
        switch ( wield->value[0] )
        {
        default:
            sn = -1;
            break;
        case ( WEAPON_SWORD ):
            sn = gsn_sword;
            break;
        case ( WEAPON_DAGGER ):
            sn = gsn_dagger;
            break;
        case ( WEAPON_SPEAR ):
            sn = gsn_spear;
            break;
        case ( WEAPON_MACE ):
            sn = gsn_mace;
            break;
        case ( WEAPON_AXE ):
            sn = gsn_axe;
            break;
        case ( WEAPON_FLAIL ):
            sn = gsn_flail;
            break;
        case ( WEAPON_WHIP ):
            sn = gsn_whip;
            break;
        case ( WEAPON_POLEARM ):
            sn = gsn_polearm;
            break;
        }
    return sn;
}

int get_second_weapon_sn( CHAR_DATA * ch )
{
    OBJ_DATA *wield;
    int sn;

    wield = get_eq_char( ch, WEAR_SECOND_WIELD );
    if ( wield == NULL || wield->item_type != ITEM_WEAPON )
        sn = gsn_hand_to_hand;
    else
        switch ( wield->value[0] )
        {
        default:
            sn = -1;
            break;
        case ( WEAPON_SWORD ):
            sn = gsn_sword;
            break;
        case ( WEAPON_DAGGER ):
            sn = gsn_dagger;
            break;
        case ( WEAPON_SPEAR ):
            sn = gsn_spear;
            break;
        case ( WEAPON_MACE ):
            sn = gsn_mace;
            break;
        case ( WEAPON_AXE ):
            sn = gsn_axe;
            break;
        case ( WEAPON_FLAIL ):
            sn = gsn_flail;
            break;
        case ( WEAPON_WHIP ):
            sn = gsn_whip;
            break;
        case ( WEAPON_POLEARM ):
            sn = gsn_polearm;
            break;
        }
    return sn;
}

int get_weapon_skill( CHAR_DATA * ch, int sn )
{
    int skill;

    /* -1 is exotic */
    if ( IS_NPC( ch ) )
    {
        if ( sn == -1 )
            skill = 3 * ch->level;
        else if ( sn == gsn_hand_to_hand )
            skill = 40 + 2 * ch->level;
        else
            skill = 40 + 5 * ch->level / 2;
    }

    else
    {
        if ( sn == -1 )
            skill = 3 * ch->level;
        else
            skill = ch->pcdata->learned[sn];
    }

    return URANGE( 0, skill, 100 );
}

/* used to de-screw characters */
void reset_char( CHAR_DATA * ch )
{
    int loc, mod, stat;
    OBJ_DATA *obj;
    AFFECT_DATA *af;
    NEWAFFECT_DATA *naf;
    int i;

    if ( IS_NPC( ch ) )
        return;

    if ( ch->pcdata->perm_hit == 0
         || ch->pcdata->perm_mana == 0
         || ch->pcdata->perm_move == 0 || ch->pcdata->last_level == 0 )
    {
        /* do a FULL reset */
        for ( loc = 0; loc < MAX_WEAR; loc++ )
        {
            obj = get_eq_char( ch, loc );
            if ( obj == NULL )
                continue;
            if ( !obj->enchanted )
                for ( af = obj->pIndexData->affected; af != NULL;
                      af = af->next )
                {
                    mod = af->modifier;
                    switch ( af->location )
                    {
                    case APPLY_SEX:
                        ch->sex -= mod;
                        if ( ch->sex < 0 || ch->sex > 2 )
                            ch->sex = IS_NPC( ch ) ? 0 : ch->pcdata->true_sex;
                        break;
                    case APPLY_MANA:
                        ch->max_mana -= mod;
                        break;
                    case APPLY_HIT:
                        ch->max_hit -= mod;
                        break;
                    case APPLY_MOVE:
                        ch->max_move -= mod;
                        break;
                    }
                }

            for ( af = obj->affected; af != NULL; af = af->next )
            {
                mod = af->modifier;
                switch ( af->location )
                {
                case APPLY_SEX:
                    ch->sex -= mod;
                    break;
                case APPLY_MANA:
                    ch->max_mana -= mod;
                    break;
                case APPLY_HIT:
                    ch->max_hit -= mod;
                    break;
                case APPLY_MOVE:
                    ch->max_move -= mod;
                    break;
                }
            }
        }
        /* now reset the permanent stats */
        ch->pcdata->perm_hit = ch->max_hit;
        ch->pcdata->perm_mana = ch->max_mana;
        ch->pcdata->perm_move = ch->max_move;
        ch->pcdata->last_level = ch->played / 3600;
        if ( ch->pcdata->true_sex < 0 || ch->pcdata->true_sex > 2 )
        {
            if ( ch->sex > 0 && ch->sex < 3 )
                ch->pcdata->true_sex = ch->sex;
            else
                ch->pcdata->true_sex = 0;
        }

    }

    /* now restore the character to his/her true condition */
    for ( stat = 0; stat < MAX_STATS; stat++ )
        ch->mod_stat[stat] = 0;

    if ( ch->pcdata->true_sex < 0 || ch->pcdata->true_sex > 2 )
        ch->pcdata->true_sex = 0;
    ch->sex = ch->pcdata->true_sex;
    ch->max_hit = ch->pcdata->perm_hit;
    ch->max_mana = ch->pcdata->perm_mana;
    ch->max_move = ch->pcdata->perm_move;

    for ( i = 0; i < 4; i++ )
        ch->armor[i] = 100;

    ch->hitroll = 0;
    ch->damroll = 0;
    ch->saving_throw = 0;

    /* now start adding back the effects */
    for ( loc = 0; loc < MAX_WEAR; loc++ )
    {
        obj = get_eq_char( ch, loc );
        if ( obj == NULL )
            continue;
        for ( i = 0; i < 4; i++ )
            ch->armor[i] -= apply_ac( obj, loc, i );

        if ( !obj->enchanted )
            for ( af = obj->pIndexData->affected; af != NULL; af = af->next )
            {
                mod = af->modifier;
                switch ( af->location )
                {
                case APPLY_STR:
                    ch->mod_stat[STAT_STR] += mod;
                    break;
                case APPLY_DEX:
                    ch->mod_stat[STAT_DEX] += mod;
                    break;
                case APPLY_INT:
                    ch->mod_stat[STAT_INT] += mod;
                    break;
                case APPLY_WIS:
                    ch->mod_stat[STAT_WIS] += mod;
                    break;
                case APPLY_CON:
                    ch->mod_stat[STAT_CON] += mod;
                    break;

                case APPLY_SEX:
                    ch->sex += mod;
                    break;
                case APPLY_MANA:
                    ch->max_mana += mod;
                    break;
                case APPLY_HIT:
                    ch->max_hit += mod;
                    break;
                case APPLY_MOVE:
                    ch->max_move += mod;
                    break;

                case APPLY_AC:
                    for ( i = 0; i < 4; i++ )
                        ch->armor[i] += mod;
                    break;
                case APPLY_HITROLL:
                    ch->hitroll += mod;
                    break;
                case APPLY_DAMROLL:
                    ch->damroll += mod;
                    break;

                case APPLY_SAVING_PARA:
                    ch->saving_throw += mod;
                    break;
                case APPLY_SAVING_ROD:
                    ch->saving_throw += mod;
                    break;
                case APPLY_SAVING_PETRI:
                    ch->saving_throw += mod;
                    break;
                case APPLY_SAVING_BREATH:
                    ch->saving_throw += mod;
                    break;
                case APPLY_SAVING_SPELL:
                    ch->saving_throw += mod;
                    break;
                case APPLY_ALIGN:
                    ch->alignment += mod;
                    break;
                }
            }

        for ( af = obj->affected; af != NULL; af = af->next )
        {
            mod = af->modifier;
            switch ( af->location )
            {
            case APPLY_STR:
                ch->mod_stat[STAT_STR] += mod;
                break;
            case APPLY_DEX:
                ch->mod_stat[STAT_DEX] += mod;
                break;
            case APPLY_INT:
                ch->mod_stat[STAT_INT] += mod;
                break;
            case APPLY_WIS:
                ch->mod_stat[STAT_WIS] += mod;
                break;
            case APPLY_CON:
                ch->mod_stat[STAT_CON] += mod;
                break;

            case APPLY_SEX:
                ch->sex += mod;
                break;
            case APPLY_MANA:
                ch->max_mana += mod;
                break;
            case APPLY_HIT:
                ch->max_hit += mod;
                break;
            case APPLY_MOVE:
                ch->max_move += mod;
                break;

            case APPLY_AC:
                for ( i = 0; i < 4; i++ )
                    ch->armor[i] += mod;
                break;
            case APPLY_HITROLL:
                ch->hitroll += mod;
                break;
            case APPLY_DAMROLL:
                ch->damroll += mod;
                break;

            case APPLY_SAVING_PARA:
                ch->saving_throw += mod;
                break;
            case APPLY_SAVING_ROD:
                ch->saving_throw += mod;
                break;
            case APPLY_SAVING_PETRI:
                ch->saving_throw += mod;
                break;
            case APPLY_SAVING_BREATH:
                ch->saving_throw += mod;
                break;
            case APPLY_SAVING_SPELL:
                ch->saving_throw += mod;
                break;
            case APPLY_ALIGN:
                ch->alignment += mod;
                break;
            }
        }
    }

    /* now add back spell effects */
    for ( af = ch->affected; af != NULL; af = af->next )
    {
        mod = af->modifier;
        switch ( af->location )
        {
        case APPLY_STR:
            ch->mod_stat[STAT_STR] += mod;
            break;
        case APPLY_DEX:
            ch->mod_stat[STAT_DEX] += mod;
            break;
        case APPLY_INT:
            ch->mod_stat[STAT_INT] += mod;
            break;
        case APPLY_WIS:
            ch->mod_stat[STAT_WIS] += mod;
            break;
        case APPLY_CON:
            ch->mod_stat[STAT_CON] += mod;
            break;

        case APPLY_SEX:
            ch->sex += mod;
            break;
        case APPLY_MANA:
            ch->max_mana += mod;
            break;
        case APPLY_HIT:
            ch->max_hit += mod;
            break;
        case APPLY_MOVE:
            ch->max_move += mod;
            break;

        case APPLY_AC:
            for ( i = 0; i < 4; i++ )
                ch->armor[i] += mod;
            break;
        case APPLY_HITROLL:
            ch->hitroll += mod;
            break;
        case APPLY_DAMROLL:
            ch->damroll += mod;
            break;

        case APPLY_SAVING_PARA:
            ch->saving_throw += mod;
            break;
        case APPLY_SAVING_ROD:
            ch->saving_throw += mod;
            break;
        case APPLY_SAVING_PETRI:
            ch->saving_throw += mod;
            break;
        case APPLY_SAVING_BREATH:
            ch->saving_throw += mod;
            break;
        case APPLY_SAVING_SPELL:
            ch->saving_throw += mod;
            break;
        case APPLY_ALIGN:
            ch->alignment += mod;
            break;
        }
    }

    for ( naf = ch->newaffected; naf != NULL; naf = naf->next )
    {
        mod = naf->modifier;
        switch ( naf->location )
        {
        case APPLY_STR:
            ch->mod_stat[STAT_STR] += mod;
            break;
        case APPLY_DEX:
            ch->mod_stat[STAT_DEX] += mod;
            break;
        case APPLY_INT:
            ch->mod_stat[STAT_INT] += mod;
            break;
        case APPLY_WIS:
            ch->mod_stat[STAT_WIS] += mod;
            break;
        case APPLY_CON:
            ch->mod_stat[STAT_CON] += mod;
            break;

        case APPLY_SEX:
            ch->sex += mod;
            break;
        case APPLY_MANA:
            ch->max_mana += mod;
            break;
        case APPLY_HIT:
            ch->max_hit += mod;
            break;
        case APPLY_MOVE:
            ch->max_move += mod;
            break;

        case APPLY_AC:
            for ( i = 0; i < 4; i++ )
                ch->armor[i] += mod;
            break;
        case APPLY_HITROLL:
            ch->hitroll += mod;
            break;
        case APPLY_DAMROLL:
            ch->damroll += mod;
            break;

        case APPLY_SAVING_PARA:
            ch->saving_throw += mod;
            break;
        case APPLY_SAVING_ROD:
            ch->saving_throw += mod;
            break;
        case APPLY_SAVING_PETRI:
            ch->saving_throw += mod;
            break;
        case APPLY_SAVING_BREATH:
            ch->saving_throw += mod;
            break;
        case APPLY_SAVING_SPELL:
            ch->saving_throw += mod;
            break;
        case APPLY_ALIGN:
            ch->alignment += mod;
            break;
        }
    }

    /* make sure sex is RIGHT!!!! */
    if ( ch->sex < 0 || ch->sex > 2 )
        ch->sex = ch->pcdata->true_sex;
}

/*
 * Retrieve a character's trusted level for permission checking.
 */
int get_trust( CHAR_DATA * ch )
{
    if ( ch->desc != NULL && ch->desc->original != NULL )
        ch = ch->desc->original;

    if ( ch->trust != 0 )
        return ch->trust;

    if ( IS_NPC( ch ) && ch->level >= LEVEL_HERO )
        return LEVEL_HERO - 1;
    else
        return ch->level;
}

/*
 * Retrieve a character's age.
 */
int get_age( CHAR_DATA * ch )
{
    return 17 + ( ch->played + ( int ) ( current_time - ch->logon ) ) / 72000;
}

/* command for retrieving stats */
int get_curr_stat( CHAR_DATA * ch, int stat )
{
    int max;

    if ( IS_NPC( ch ) || ch->level > LEVEL_IMMORTAL )
        max = MAX_ATTAINABLE_STATS;

    else
    {
        max = pc_race_table[ch->race].max_stats[stat] + 4;

        if ( class_table[ch->Class].attr_prime == stat )
            max += 2;

        if ( ch->race == race_lookup( "human" ) )
            max += 1;

        max = UMIN( max, MAX_ATTAINABLE_STATS );
    }

    return URANGE( 3, ch->perm_stat[stat] + ch->mod_stat[stat], max );
}

/* command for returning max training score */
int get_max_train( CHAR_DATA * ch, int stat )
{
    int max;

    if ( IS_NPC( ch ) || ch->level > LEVEL_IMMORTAL )
        return MAX_ATTAINABLE_STATS;

    max = pc_race_table[ch->race].max_stats[stat];
    if ( class_table[ch->Class].attr_prime == stat )
    {
        if ( ch->race == race_lookup( "human" ) )
            max += 3;
        else
            max += 2;
    }

    return UMIN( max, MAX_ATTAINABLE_STATS );
}

/*
 * Retrieve a character's carry capacity.
 */
int can_carry_n( CHAR_DATA * ch )
{
    if ( !IS_NPC( ch ) && ch->level >= LEVEL_IMMORTAL )
        return 1000;

    if ( IS_NPC( ch ) && IS_SET( ch->act, ACT_PET ) )
        return 0;

    return MAX_WEAR + 2 * get_curr_stat( ch, STAT_DEX ) + ch->level;
}

/*
 * Retrieve a character's carry capacity.
 */
int can_carry_w( CHAR_DATA * ch )
{
    if ( !IS_NPC( ch ) && ch->level >= LEVEL_IMMORTAL )
        return 1000000;

    if ( IS_NPC( ch ) && IS_SET( ch->act, ACT_PET ) )
        return 0;

    return str_app[get_curr_stat( ch, STAT_STR )].carry + ch->level * 5 / 2;
}

/*
 * See if a string is one of the names of an object.
 */

/*
 * bool is_name( const char *str, char *namelist )
 * {
 *   char name[MAX_INPUT_LENGTH];
 * 
 *   for ( ; ; )
 *   {
 *	 namelist = one_argument( namelist, name );
 *	 if ( name[0] == '\0' )
 *	     return FALSE;
 *	 if ( !str_cmp( str, name ) )
 *	     return TRUE;
 *   }
 * }
 * 
 */

/*
 * See if a string is one of the names of an object.
 */

bool is_full_name( const char *str, char *namelist )
{
    char name[MAX_INPUT_LENGTH];

    for ( ;; )
    {
        namelist = one_argument( namelist, name );
        if ( name[0] == '\0' )
            return FALSE;
        if ( !str_cmp( str, name ) )
            return TRUE;
    }
}

bool is_exact_name( char *str, char *namelist )
{
    char name[MAX_INPUT_LENGTH], part[MAX_INPUT_LENGTH];
    char *list, *string;

    string = str;
    /* we need ALL parts of string to match part of namelist */
    for ( ;; )                  /* start parsing string */
    {
        str = one_argument( str, part );

        if ( part[0] == '\0' )
            return TRUE;

        /* check to see if this is part of namelist */
        list = namelist;
        for ( ;; )              /* start parsing namelist */
        {
            list = one_argument( list, name );
            if ( name[0] == '\0' )  /* this name was not found */
                return FALSE;

            if ( !str_cmp( string, name ) )
                return TRUE;    /* full pattern match */

            if ( !str_cmp( part, name ) )
                break;
        }
    }
}

bool is_name( char *str, char *namelist )
{
    char name[MAX_INPUT_LENGTH], part[MAX_INPUT_LENGTH];
    char *list, *string;

    string = str;
    /* we need ALL parts of string to match part of namelist */
    for ( ;; )                  /* start parsing string */
    {
        str = one_argument( str, part );

        if ( part[0] == '\0' )
            return TRUE;

        /* check to see if this is part of namelist */
        list = namelist;
        for ( ;; )              /* start parsing namelist */
        {
            list = one_argument( list, name );
            if ( name[0] == '\0' )  /* this name was not found */
                return FALSE;

            if ( !str_prefix( string, name ) )
                return TRUE;    /* full pattern match */

            if ( !str_prefix( part, name ) )
                break;
        }
    }
}

/*
 * Apply or remove an affect to a character.
 */
void affect_modify( CHAR_DATA * ch, AFFECT_DATA * paf, bool fAdd )
{
    OBJ_DATA *wield;
    int mod, i;

    mod = paf->modifier;

    if ( fAdd )
    {
        SET_BIT( ch->affected_by, paf->bitvector );
    }
    else
    {
        REMOVE_BIT( ch->affected_by, paf->bitvector );
        mod = 0 - mod;
    }

    switch ( paf->location )
    {
    default:
        bug( "Affect_modify: unknown location %d.", paf->location );
        return;

    case APPLY_NONE:
        break;
    case APPLY_STR:
        ch->mod_stat[STAT_STR] += mod;
        break;
    case APPLY_DEX:
        ch->mod_stat[STAT_DEX] += mod;
        break;
    case APPLY_INT:
        ch->mod_stat[STAT_INT] += mod;
        break;
    case APPLY_WIS:
        ch->mod_stat[STAT_WIS] += mod;
        break;
    case APPLY_CON:
        ch->mod_stat[STAT_CON] += mod;
        break;
    case APPLY_SEX:
        ch->sex += mod;
        break;
    case APPLY_CLASS:
        break;
    case APPLY_LEVEL:
        break;
    case APPLY_AGE:
        break;
    case APPLY_HEIGHT:
        break;
    case APPLY_WEIGHT:
        break;
    case APPLY_MANA:
        ch->max_mana += mod;
        if ( ch->mana > ch->max_mana )
            ch->mana = ch->max_mana;
        break;
    case APPLY_HIT:
        ch->max_hit += mod;
        if ( ch->hit > ch->max_hit )
            ch->hit = ch->max_hit;
        break;
    case APPLY_MOVE:
        ch->max_move += mod;
        if ( ch->move > ch->max_move )
            ch->move = ch->max_move;
        break;
    case APPLY_GOLD:
        break;
    case APPLY_EXP:
        break;
    case APPLY_AC:
        for ( i = 0; i < 4; i++ )
            ch->armor[i] += mod;
        break;
    case APPLY_HITROLL:
        ch->hitroll += mod;
        break;
    case APPLY_DAMROLL:
        ch->damroll += mod;
        break;
    case APPLY_SAVING_PARA:
        ch->saving_throw += mod;
        break;
    case APPLY_SAVING_ROD:
        ch->saving_throw += mod;
        break;
    case APPLY_SAVING_PETRI:
        ch->saving_throw += mod;
        break;
    case APPLY_SAVING_BREATH:
        ch->saving_throw += mod;
        break;
    case APPLY_SAVING_SPELL:
        ch->saving_throw += mod;
        break;
    case APPLY_ALIGN:
        ch->alignment += mod;
        break;
    }

    /*
     * Check for weapon wielding.
     * Guard against recursion (for weapons with affects).
     */
    if ( !IS_NPC( ch ) && ( wield = get_eq_char( ch, WEAR_WIELD ) ) != NULL
         && get_obj_weight( wield ) >
         str_app[get_curr_stat( ch, STAT_STR )].wield )
    {
        static int depth;

        if ( depth == 0 )
        {
            depth++;
            act( "You drop $p.", ch, wield, NULL, TO_CHAR );
            act( "$n drops $p.", ch, wield, NULL, TO_ROOM );
            obj_from_char( wield );
            obj_to_room( wield, ch->in_room );
            depth--;
        }
    }

    return;
}

void newaffect_modify( CHAR_DATA * ch, NEWAFFECT_DATA * npaf, bool fAdd )
{
    OBJ_DATA *wield;
    int mod, i;

    mod = npaf->modifier;

    if ( fAdd )
    {
        SET_NEWAFF( ch->newaff, npaf->bitvector );
    }
    else
    {
        REMOVE_NEWAFF( ch->newaff, npaf->bitvector );
        mod = 0 - mod;
    }

    switch ( npaf->location )
    {
    default:
        bug( "newaffect_modify: unknown location %d.", npaf->location );
        return;

    case APPLY_NONE:
        break;
    case APPLY_STR:
        ch->mod_stat[STAT_STR] += mod;
        break;
    case APPLY_DEX:
        ch->mod_stat[STAT_DEX] += mod;
        break;
    case APPLY_INT:
        ch->mod_stat[STAT_INT] += mod;
        break;
    case APPLY_WIS:
        ch->mod_stat[STAT_WIS] += mod;
        break;
    case APPLY_CON:
        ch->mod_stat[STAT_CON] += mod;
        break;
    case APPLY_SEX:
        ch->sex += mod;
        break;
    case APPLY_CLASS:
        break;
    case APPLY_LEVEL:
        break;
    case APPLY_AGE:
        break;
    case APPLY_HEIGHT:
        break;
    case APPLY_WEIGHT:
        break;
    case APPLY_MANA:
        ch->max_mana += mod;
        if ( ch->mana > ch->max_mana )
            ch->mana = ch->max_mana;
        break;

    case APPLY_HIT:
        ch->max_hit += mod;
        if ( ch->hit > ch->max_hit )
            ch->hit = ch->max_hit;
        break;

    case APPLY_MOVE:
        ch->max_move += mod;
        if ( ch->move > ch->max_move )
            ch->move = ch->max_move;
        break;

    case APPLY_GOLD:
        break;
    case APPLY_EXP:
        break;
    case APPLY_AC:
        for ( i = 0; i < 4; i++ )
            ch->armor[i] += mod;
        break;

    case APPLY_HITROLL:
        ch->hitroll += mod;
        break;
    case APPLY_DAMROLL:
        ch->damroll += mod;
        break;
    case APPLY_SAVING_PARA:
        ch->saving_throw += mod;
        break;
    case APPLY_SAVING_ROD:
        ch->saving_throw += mod;
        break;
    case APPLY_SAVING_PETRI:
        ch->saving_throw += mod;
        break;
    case APPLY_SAVING_BREATH:
        ch->saving_throw += mod;
        break;
    case APPLY_SAVING_SPELL:
        ch->saving_throw += mod;
        break;
    case APPLY_ALIGN:
        ch->alignment += mod;
        break;
    }

    /*
     * Check for weapon wielding.
     * Guard against recursion (for weapons with affects).
     */
    if ( !IS_NPC( ch ) && ( wield = get_eq_char( ch, WEAR_WIELD ) ) != NULL
         && get_obj_weight( wield ) >
         str_app[get_curr_stat( ch, STAT_STR )].wield )
    {
        static int depth;

        if ( depth == 0 )
        {
            depth++;
            act( "You drop $p.", ch, wield, NULL, TO_CHAR );
            act( "$n drops $p.", ch, wield, NULL, TO_ROOM );
            obj_from_char( wield );
            obj_to_room( wield, ch->in_room );
            depth--;
        }
    }

    return;
}

/*
 * Give an affect to a char.
 */
void affect_to_char( CHAR_DATA * ch, AFFECT_DATA * paf )
{
    AFFECT_DATA *paf_new;

    if ( affect_free == NULL )
    {
        paf_new = alloc_perm( sizeof( *paf_new ) );
    }
    else
    {
        paf_new = affect_free;
        affect_free = affect_free->next;
    }

    *paf_new = *paf;
    paf_new->next = ch->affected;
    ch->affected = paf_new;

    affect_modify( ch, paf_new, TRUE );
    return;
}

void newaffect_to_char( CHAR_DATA * ch, NEWAFFECT_DATA * npaf )
{
    NEWAFFECT_DATA *npaf_new;

    if ( newaffect_free == NULL )
    {
        npaf_new = alloc_perm( sizeof( *npaf_new ) );
    }
    else
    {
        npaf_new = newaffect_free;
        newaffect_free = newaffect_free->next;
    }

    *npaf_new = *npaf;
    npaf_new->next = ch->newaffected;
    ch->newaffected = npaf_new;

    newaffect_modify( ch, npaf_new, TRUE );
    return;
}

/* give an affect to an object */
void affect_to_obj( OBJ_DATA * obj, AFFECT_DATA * paf )
{
    AFFECT_DATA *paf_new;

    if ( affect_free == NULL )
        paf_new = alloc_perm( sizeof( *paf_new ) );
    else
    {
        paf_new = affect_free;
        affect_free = affect_free->next;
    }

    *paf_new = *paf;
    paf_new->next = obj->affected;
    obj->affected = paf_new;

    return;
}

/*
 * Remove an affect from a char.
 */
void affect_remove( CHAR_DATA * ch, AFFECT_DATA * paf )
{
    if ( ch->affected == NULL )
    {
        bug( "Affect_remove: no affect.", 0 );
        return;
    }

    affect_modify( ch, paf, FALSE );

    if ( paf == ch->affected )
    {
        ch->affected = paf->next;
    }
    else
    {
        AFFECT_DATA *prev;

        for ( prev = ch->affected; prev != NULL; prev = prev->next )
        {
            if ( prev->next == paf )
            {
                prev->next = paf->next;
                break;
            }
        }

        if ( prev == NULL )
        {
            bug( "Affect_remove: cannot find paf.", 0 );
            return;
        }
    }

    paf->next = affect_free;
    affect_free = paf->next;
    return;
}

void newaffect_remove( CHAR_DATA * ch, NEWAFFECT_DATA * npaf )
{
    if ( ch->newaffected == NULL )
    {
        bug( "Affect_remove: no affect.", 0 );
        return;
    }

    newaffect_modify( ch, npaf, FALSE );

    if ( npaf == ch->newaffected )
    {
        ch->newaffected = npaf->next;
    }
    else
    {
        NEWAFFECT_DATA *nprev;

        for ( nprev = ch->newaffected; nprev != NULL; nprev = nprev->next )
        {
            if ( nprev->next == npaf )
            {
                nprev->next = npaf->next;
                break;
            }
        }

        if ( nprev == NULL )
        {
            bug( "newaffect_remove: cannot find npaf.", 0 );
            return;
        }
    }

    npaf->next = newaffect_free;
    newaffect_free = npaf->next;
    return;
}

void affect_remove_obj( OBJ_DATA * obj, AFFECT_DATA * paf )
{
    if ( obj->affected == NULL )
    {
        bug( "Affect_remove_object: no affect.", 0 );
        return;
    }

    if ( obj->carried_by != NULL && obj->wear_loc != -1 )
        affect_modify( obj->carried_by, paf, FALSE );

    if ( paf == obj->affected )
    {
        obj->affected = paf->next;
    }
    else
    {
        AFFECT_DATA *prev;

        for ( prev = obj->affected; prev != NULL; prev = prev->next )
        {
            if ( prev->next == paf )
            {
                prev->next = paf->next;
                break;
            }
        }

        if ( prev == NULL )
        {
            bug( "Affect_remove_object: cannot find paf.", 0 );
            return;
        }
    }

    paf->next = affect_free;
    affect_free = paf->next;
    return;
}

/*
 * Strip all affects of a given sn.
 */
void affect_strip( CHAR_DATA * ch, int sn )
{
    AFFECT_DATA *paf;
    AFFECT_DATA *paf_next;

    for ( paf = ch->affected; paf != NULL; paf = paf_next )
    {
        paf_next = paf->next;
        if ( paf->type == sn )
            affect_remove( ch, paf );
    }

    return;
}

void newaffect_strip( CHAR_DATA * ch, int sn )
{
    NEWAFFECT_DATA *npaf;
    NEWAFFECT_DATA *npaf_next;

    for ( npaf = ch->newaffected; npaf != NULL; npaf = npaf_next )
    {
        npaf_next = npaf->next;
        if ( npaf->type == sn )
            newaffect_remove( ch, npaf );
    }

    return;
}

/*
 * Return true if a char is affected by a spell.
 */
bool is_affected( CHAR_DATA * ch, int sn )
{
    AFFECT_DATA *paf;

    for ( paf = ch->affected; paf != NULL; paf = paf->next )
    {
        if ( paf->type == sn )
            return TRUE;
    }

    return FALSE;
}

bool is_newaffected( CHAR_DATA * ch, int sn )
{
    NEWAFFECT_DATA *npaf;

    for ( npaf = ch->newaffected; npaf != NULL; npaf = npaf->next )
    {
        if ( npaf->type == sn )
            return TRUE;
    }

    return FALSE;
}

/*
 * Add or enhance an affect.
 */
void affect_join( CHAR_DATA * ch, AFFECT_DATA * paf )
{
    AFFECT_DATA *paf_old;
    bool found;

    found = FALSE;
    for ( paf_old = ch->affected; paf_old != NULL; paf_old = paf_old->next )
    {
        if ( paf_old->type == paf->type )
        {
            paf->level = ( paf->level += paf_old->level ) / 2;
            paf->duration += paf_old->duration;
            paf->modifier += paf_old->modifier;
            affect_remove( ch, paf_old );
            break;
        }
    }

    affect_to_char( ch, paf );
    return;
}

void newaffect_join( CHAR_DATA * ch, NEWAFFECT_DATA * npaf )
{
    NEWAFFECT_DATA *npaf_old;
    bool found;

    found = FALSE;
    for ( npaf_old = ch->newaffected; npaf_old != NULL;
          npaf_old = npaf_old->next )
    {
        if ( npaf_old->type == npaf->type )
        {
            npaf->level = ( npaf->level += npaf_old->level ) / 2;
            npaf->duration += npaf_old->duration;
            npaf->modifier += npaf_old->modifier;
            newaffect_remove( ch, npaf_old );
            break;
        }
    }

    newaffect_to_char( ch, npaf );
    return;
}

/*
 * Move a char out of a room.
 */
void char_from_room( CHAR_DATA * ch )
{
    OBJ_DATA *obj;

    if ( ch->in_room == NULL )
    {
        bug( "Char_from_room: NULL.", 0 );
        return;
    }

    if ( !IS_NPC( ch ) )
        --ch->in_room->area->nplayer;

    if ( ( obj = get_eq_char( ch, WEAR_LIGHT ) ) != NULL
         && obj->item_type == ITEM_LIGHT
         && obj->value[2] != 0 && ch->in_room->light > 0 )
        --ch->in_room->light;

    if ( ch == ch->in_room->people )
    {
        ch->in_room->people = ch->next_in_room;
    }
    else
    {
        CHAR_DATA *prev;

        for ( prev = ch->in_room->people; prev; prev = prev->next_in_room )
        {
            if ( prev->next_in_room == ch )
            {
                prev->next_in_room = ch->next_in_room;
                break;
            }
        }

        if ( prev == NULL )
            bug( "Char_from_room: ch not found.", 0 );
    }

    ch->in_room = NULL;
    ch->next_in_room = NULL;
    ch->on = NULL;              /* sanity check! */
    return;
}

/*
 * Move a char into a room.
 */
void char_to_room( CHAR_DATA * ch, ROOM_INDEX_DATA * pRoomIndex )
{
    OBJ_DATA *obj;

    if ( pRoomIndex == NULL )
    {
        bug( "Char_to_room: NULL. - ch->name = %s", ch->name );
        return;
    }

    ch->in_room = pRoomIndex;
    ch->next_in_room = pRoomIndex->people;
    pRoomIndex->people = ch;

    if ( !IS_NPC( ch ) )
    {
        if ( ch->in_room->area->empty )
        {
            ch->in_room->area->empty = FALSE;
            ch->in_room->area->age = 0;
        }
        ++ch->in_room->area->nplayer;
    }

    if ( ( obj = get_eq_char( ch, WEAR_LIGHT ) ) != NULL
         && obj->item_type == ITEM_LIGHT && obj->value[2] != 0 )
        ++ch->in_room->light;

    if ( IS_AFFECTED( ch, AFF_PLAGUE ) )
    {
        AFFECT_DATA *af, plague;
        CHAR_DATA *vch;
        int save;

        for ( af = ch->affected; af != NULL; af = af->next )
        {
            if ( af->type == gsn_plague )
                break;
        }

        if ( af == NULL )
        {
            REMOVE_BIT( ch->affected_by, AFF_PLAGUE );
            return;
        }

        if ( af->level == 1 )
            return;

        plague.type = gsn_plague;
        plague.level = af->level - 1;
        plague.duration = number_range( 1, 2 * plague.level );
        plague.location = APPLY_STR;
        plague.modifier = -5;
        plague.bitvector = AFF_PLAGUE;

        for ( vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room )
        {
            switch ( check_immune( vch, DAM_DISEASE ) )
            {
            case ( IS_NORMAL ):
                save = af->level - 4;
                break;
            case ( IS_IMMUNE ):
                save = 0;
                break;
            case ( IS_RESISTANT ):
                save = af->level - 8;
                break;
            case ( IS_VULNERABLE ):
                save = af->level;
                break;
            default:
                save = af->level - 4;
                break;
            }

            if ( save != 0 && !saves_spell( save, vch ) && !IS_IMMORTAL( vch )
                 && !IS_AFFECTED( vch, AFF_PLAGUE ) && number_bits( 6 ) == 0 )
            {
                send_to_char( "You feel hot and feverish.\n\r", vch );
                act( "$n shivers and looks very ill.", vch, NULL, NULL,
                     TO_ROOM );
                affect_join( vch, &plague );
            }
        }
    }

    return;
}

/*
 * Give an obj to a char.
 */
void obj_to_char( OBJ_DATA * obj, CHAR_DATA * ch )
{
    obj->next_content = ch->carrying;
    ch->carrying = obj;
    obj->carried_by = ch;
    obj->in_room = NULL;
    obj->in_obj = NULL;
    ch->carry_number += get_obj_number( obj );
    ch->carry_weight += get_obj_weight( obj );
}

/*
 * Take an obj from its character.
 */
void obj_from_char( OBJ_DATA * obj )
{
    CHAR_DATA *ch;

    if ( ( ch = obj->carried_by ) == NULL )
    {
        bug( "Obj_from_char: null ch.", 0 );
        return;
    }

    if ( obj->wear_loc != WEAR_NONE )
        unequip_char( ch, obj );

    if ( ch->carrying == obj )
    {
        ch->carrying = obj->next_content;
    }
    else
    {
        OBJ_DATA *prev;

        for ( prev = ch->carrying; prev != NULL; prev = prev->next_content )
        {
            if ( prev->next_content == obj )
            {
                prev->next_content = obj->next_content;
                break;
            }
        }

        if ( prev == NULL )
            bug( "Obj_from_char: obj not in list.", 0 );
    }

    obj->carried_by = NULL;
    obj->next_content = NULL;
    ch->carry_number -= get_obj_number( obj );
    ch->carry_weight -= get_obj_weight( obj );
    return;
}

/*
 * Find the ac value of an obj, including position effect.
 */
int apply_ac( OBJ_DATA * obj, int iWear, int type )
{
    if ( obj->item_type != ITEM_ARMOR )
        return 0;

    switch ( iWear )
    {
    case WEAR_BODY:
        return 3 * obj->value[type];
    case WEAR_HEAD:
        return 2 * obj->value[type];
    case WEAR_LEGS:
        return 2 * obj->value[type];
    case WEAR_FEET:
        return obj->value[type];
    case WEAR_HANDS:
        return obj->value[type];
    case WEAR_ARMS:
        return obj->value[type];
    case WEAR_SHIELD:
        return obj->value[type];
    case WEAR_FINGER_L:
        return obj->value[type];
    case WEAR_FINGER_R:
        return obj->value[type];
    case WEAR_NECK_1:
        return obj->value[type];
    case WEAR_NECK_2:
        return obj->value[type];
    case WEAR_ABOUT:
        return 2 * obj->value[type];
    case WEAR_WAIST:
        return obj->value[type];
    case WEAR_WRIST_L:
        return obj->value[type];
    case WEAR_WRIST_R:
        return obj->value[type];
    case WEAR_HOLD:
        return obj->value[type];
    }

    return 0;
}

/*
 * Find a piece of eq on a character.
 */
OBJ_DATA *get_eq_char( CHAR_DATA * ch, int iWear )
{
    OBJ_DATA *obj;

    if ( ch == NULL )
        return NULL;

    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
    {
        if ( obj->wear_loc == iWear )
            return obj;
    }

    return NULL;
}

/*
 * Equip a char with an obj.
 */
void equip_char( CHAR_DATA * ch, OBJ_DATA * obj, int iWear )
{
    AFFECT_DATA *paf;
    int i;

    if ( get_eq_char( ch, iWear ) != NULL )
    {
        bug( "Equip_char: already equipped (%d).", iWear );
        return;
    }

    if ( ( IS_OBJ_STAT( obj, ITEM_ANTI_EVIL ) && IS_EVIL( ch ) )
         || ( IS_OBJ_STAT( obj, ITEM_ANTI_GOOD ) && IS_GOOD( ch ) )
         || ( IS_OBJ_STAT( obj, ITEM_ANTI_NEUTRAL ) && IS_NEUTRAL( ch ) )
         || ( IS_OBJ_STAT( obj, ITEM_ANTI_MAGE ) && ch->Class == CLASS_MAGE
              && !IS_NPC( ch ) ) || ( IS_OBJ_STAT( obj, ITEM_ANTI_CLERIC )
                                      && ch->Class == CLASS_CLERIC )
         || ( IS_OBJ_STAT( obj, ITEM_ANTI_THIEF ) && ch->Class == CLASS_THIEF )
         || ( IS_OBJ_STAT( obj, ITEM_ANTI_WARRIOR )
              && ch->Class == CLASS_WARRIOR )
/* adding lines for anti_gender */
         || ( IS_OBJ_STAT( obj, ITEM_ANTI_MALE ) && ch->sex == SEX_MALE )
         || ( IS_OBJ_STAT( obj, ITEM_ANTI_FEMALE ) && ch->sex == SEX_FEMALE )
         || ( IS_OBJ_STAT( obj, ITEM_ANTI_NEUTER ) && ch->sex == SEX_NEUTRAL )
/* end of lines for anti_gender */
         || ( obj->clan > 0 && !IS_NPC( ch )
              && ch->pcdata->clan != obj->clan ) )
    {
        /*
         * Thanks to Morgenes for the bug fix here!
         */
        act( "You are zapped by $p and drop it.", ch, obj, NULL, TO_CHAR );
        act( "$n is zapped by $p and drops it.", ch, obj, NULL, TO_ROOM );
        obj_from_char( obj );
        obj_to_room( obj, ch->in_room );
        return;
    }

    for ( i = 0; i < 4; i++ )
        ch->armor[i] -= apply_ac( obj, iWear, i );
    obj->wear_loc = iWear;

    if ( !obj->enchanted )
        for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
            affect_modify( ch, paf, TRUE );
    for ( paf = obj->affected; paf != NULL; paf = paf->next )
        affect_modify( ch, paf, TRUE );

    if ( obj->item_type == ITEM_LIGHT
         && obj->value[2] != 0 && ch->in_room != NULL )
        ++ch->in_room->light;

    return;
}

/*
 * Unequip a char with an obj.
 */
void unequip_char( CHAR_DATA * ch, OBJ_DATA * obj )
{
    AFFECT_DATA *paf;
    int i;

    if ( obj->wear_loc == WEAR_NONE )
    {
        bug( "Unequip_char: already unequipped.", 0 );
        return;
    }

    for ( i = 0; i < 4; i++ )
        ch->armor[i] += apply_ac( obj, obj->wear_loc, i );
    obj->wear_loc = -1;

    if ( !obj->enchanted )
        for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
            affect_modify( ch, paf, FALSE );
    for ( paf = obj->affected; paf != NULL; paf = paf->next )
        affect_modify( ch, paf, FALSE );

    if ( obj->item_type == ITEM_LIGHT
         && obj->value[2] != 0
         && ch->in_room != NULL && ch->in_room->light > 0 )
        --ch->in_room->light;

    return;
}

/*
 * Count occurrences of an obj in a list.
 */
int count_obj_list( OBJ_INDEX_DATA * pObjIndex, OBJ_DATA * list )
{
    OBJ_DATA *obj;
    int nMatch;

    nMatch = 0;
    for ( obj = list; obj != NULL; obj = obj->next_content )
    {
        if ( obj->pIndexData == pObjIndex )
            nMatch++;
    }

    return nMatch;
}

/*
 * Move an obj out of a room.
 */
void obj_from_room( OBJ_DATA * obj )
{
    ROOM_INDEX_DATA *in_room;
    CHAR_DATA *ch;
    if ( ( in_room = obj->in_room ) == NULL )
    {
        bug( "obj_from_room: NULL.", 0 );
        return;
    }
    for ( ch = in_room->people; ch != NULL; ch = ch->next_in_room )
        if ( ch->on == obj )
            ch->on = NULL;

    if ( obj == in_room->contents )
    {
        in_room->contents = obj->next_content;
    }
    else
    {
        OBJ_DATA *prev;

        for ( prev = in_room->contents; prev; prev = prev->next_content )
        {
            if ( prev->next_content == obj )
            {
                prev->next_content = obj->next_content;
                break;
            }
        }

        if ( prev == NULL )
        {
            bug( "Obj_from_room: obj not found.", 0 );
            return;
        }
    }

    obj->in_room = NULL;
    obj->next_content = NULL;
    return;
}

/*
 * Move an obj into a room.
 */
void obj_to_room( OBJ_DATA * obj, ROOM_INDEX_DATA * pRoomIndex )
{
    if ( pRoomIndex == NULL )
    {
        bug( "NULL pRoomIndex in obj_to_room.", 0 );
        if ( obj )
            extract_obj( obj );
        return;
    }
    if ( obj == NULL )
    {
        bug( "NULL obj in obj_to_room.", 0 );
        return;
    }
    obj->next_content = pRoomIndex->contents;
    pRoomIndex->contents = obj;
    obj->in_room = pRoomIndex;
    obj->carried_by = NULL;
    obj->in_obj = NULL;
    return;
}

/*
 * Move an object into an object.
 */
void obj_to_obj( OBJ_DATA * obj, OBJ_DATA * obj_to )
{
    obj->next_content = obj_to->contains;
    obj_to->contains = obj;
    obj->in_obj = obj_to;
    obj->in_room = NULL;
    obj->carried_by = NULL;
    if ( obj_to->pIndexData->vnum == OBJ_VNUM_PIT )
        obj->cost = 0;

    for ( ; obj_to != NULL; obj_to = obj_to->in_obj )
    {
        if ( obj_to->carried_by != NULL )
        {
            obj_to->carried_by->carry_number += get_obj_number( obj );
            obj_to->carried_by->carry_weight += get_obj_weight( obj );
        }
    }

    return;
}

/*
 * Move an object out of an object.
 */
void obj_from_obj( OBJ_DATA * obj )
{
    OBJ_DATA *obj_from;

    if ( ( obj_from = obj->in_obj ) == NULL )
    {
        bug( "Obj_from_obj: null obj_from.", 0 );
        return;
    }

    if ( obj == obj_from->contains )
    {
        obj_from->contains = obj->next_content;
    }
    else
    {
        OBJ_DATA *prev;

        for ( prev = obj_from->contains; prev; prev = prev->next_content )
        {
            if ( prev->next_content == obj )
            {
                prev->next_content = obj->next_content;
                break;
            }
        }

        if ( prev == NULL )
        {
            bug( "Obj_from_obj: obj not found.", 0 );
            return;
        }
    }

    obj->next_content = NULL;
    obj->in_obj = NULL;

    for ( ; obj_from != NULL; obj_from = obj_from->in_obj )
    {
        if ( obj_from->carried_by != NULL )
        {
            obj_from->carried_by->carry_number -= get_obj_number( obj );
            obj_from->carried_by->carry_weight -= get_obj_weight( obj );
        }
    }

    return;
}

/*
 * Extract an obj from the world.
 */
void extract_obj( OBJ_DATA * obj )
{
    OBJ_DATA *obj_content;
    OBJ_DATA *obj_next;

    if ( obj->in_room != NULL )
        obj_from_room( obj );
    else if ( obj->carried_by != NULL )
        obj_from_char( obj );
    else if ( obj->in_obj != NULL )
        obj_from_obj( obj );

    for ( obj_content = obj->contains; obj_content; obj_content = obj_next )
    {
        obj_next = obj_content->next_content;
        extract_obj( obj->contains );
    }

    if ( object_list == obj )
    {
        object_list = obj->next;
    }
    else
    {
        OBJ_DATA *prev;

        for ( prev = object_list; prev != NULL; prev = prev->next )
        {
            if ( prev->next == obj )
            {
                prev->next = obj->next;
                break;
            }
        }

        if ( prev == NULL )
        {
            bug( "Extract_obj: obj %d not found.", obj->pIndexData->vnum );
            return;
        }
    }

    {
        AFFECT_DATA *paf;
        AFFECT_DATA *paf_next;

        for ( paf = obj->affected; paf != NULL; paf = paf_next )
        {
            paf_next = paf->next;
            paf->next = affect_free;
            affect_free = paf;
        }
    }

    {
        EXTRA_DESCR_DATA *ed;
        EXTRA_DESCR_DATA *ed_next;

        for ( ed = obj->extra_descr; ed != NULL; ed = ed_next )
        {
            ed_next = ed->next;
            free_string( &ed->description );
            free_string( &ed->keyword );
            extra_descr_free = ed;
        }
    }

    free_string( &obj->name );
    free_string( &obj->description );
    free_string( &obj->short_descr );
    free_string( &obj->owner );
    --obj->pIndexData->count;
    obj->next = obj_free;
    obj_free = obj;
    return;
}

/*
 * Extract a char from the world.
 */
void extract_char( CHAR_DATA * ch, bool fPull )
{
    CHAR_DATA *wch;
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;

    if ( ch->in_room == NULL )
    {
        bug( "Extract_char: NULL.", 0 );
        return;
    }

    nuke_pets( ch );
    ch->pet = NULL;             /* just in case */

    if ( fPull )

        die_follower( ch );

    stop_fighting( ch, TRUE );

    for ( obj = ch->carrying; obj != NULL; obj = obj_next )
    {
        obj_next = obj->next_content;
        extract_obj( obj );
    }

    char_from_room( ch );

    if ( !fPull )
    {
        char_to_room( ch, get_room_index( ROOM_VNUM_ALTAR ) );
        return;
    }

    if ( IS_NPC( ch ) )
    {
        ch->pIndexData->count--;
        if ( ch->reset_count )
            ( *ch->reset_count )--;
    }

    if ( ch->desc != NULL && ch->desc->original != NULL )
    {
        do_return( ch, "" );
    }

    for ( wch = char_list; wch != NULL; wch = wch->next )
    {
        if ( wch->reply == ch )
        {
            wch->reply = NULL;
        }
    }

    /* Remove from the list of Char objects */
    if ( ch == char_list )
    {
        char_list = ch->next;
    }
    else
    {
        CHAR_DATA *prev;

        for ( prev = char_list; prev != NULL && prev->next != ch;
              prev = prev->next );

        if ( prev )
        {
            prev->next = ch->next;
        }
        else
        {
            bug( "Extract_char: char not found in char_list.", 0 );
            return;
        }
    }

    /* Remove from the list of players */
    if ( !IS_NPC( ch ) )
    {
        if ( ch == player_list )
        {
            player_list = ch->next_player;
        }
        else
        {
            CHAR_DATA *prev;

            for ( prev = player_list; prev && prev->next_player != ch;
                  prev = prev->next_player );

            if ( prev )
            {
                prev->next_player = ch->next_player;
            }
            else
            {
                bug( "Extract_char: char not found in player_list.", 0 );
                return;
            }
        }
    }

    if ( ch->desc )
    {
        ch->desc->character = NULL;
    }

    free_char( ch );
    return;
}

void pk_extract_char( CHAR_DATA * ch, bool fPull )
{
    CHAR_DATA *wch;

    if ( ch->in_room == NULL )
    {
        bug( "Extract_char: NULL.", 0 );
        return;
    }

    nuke_pets( ch );
    ch->pet = NULL;             /* just in case */

    if ( fPull )

        die_follower( ch );

    stop_fighting( ch, TRUE );

    char_from_room( ch );

    if ( !fPull )
    {
        char_to_room( ch, get_room_index( ROOM_VNUM_ALTAR ) );
        return;
    }

    if ( IS_NPC( ch ) )
    {
        ch->pIndexData->count--;
        if ( ch->reset_count )
            ( *ch->reset_count )--;
    }

    if ( ch->desc != NULL && ch->desc->original != NULL )
        do_return( ch, "" );

    for ( wch = char_list; wch != NULL; wch = wch->next )
    {
        if ( wch->reply == ch )
            wch->reply = NULL;
    }

    /* Remove from the list of Char objects */
    if ( ch == char_list )
        char_list = ch->next;
    else
    {
        CHAR_DATA *prev;

        for ( prev = char_list; prev && prev->next != ch; prev = prev->next )
            ;

        if ( prev )
            prev->next = ch->next;
        else
        {
            bug( "Extract_char: char not found.", 0 );
            return;
        }
    }

    /* Remove from the list of players */
    if ( !IS_NPC( ch ) )
    {
        if ( ch == player_list )
            player_list = ch->next_player;
        else
        {
            CHAR_DATA *prev;

            for ( prev = player_list; prev && prev->next != ch;
                  prev = prev->next_player )
                ;

            if ( prev )
                prev->next_player = ch->next_player;
            else
            {
                bug( "Extract_char: char not found in player_list.", 0 );
                return;
            }
        }
    }

    if ( ch->desc )
        ch->desc->character = NULL;
    free_char( ch );
    return;
}

/*
 * Find a char in the room.
 */
CHAR_DATA *get_char_room( CHAR_DATA * ch, char *argument )
{
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *rch;
    int number;
    int count;

    number = number_argument( argument, arg2 );
    count = 0;
    if ( !str_cmp( arg2, "self" ) )
        return ch;
    if ( !ch->in_room )         /* Make sure in_room isn't NULL (ConsoleChar) - Zane */
        return NULL;
    for ( rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room )
    {
        if ( !can_see( ch, rch ) || !is_name( arg2, rch->name ) )
            continue;
        if ( ++count == number )
            return rch;
    }

    return NULL;
}

/*
 * Find a mob in the room.
 */
CHAR_DATA *get_mob_room( CHAR_DATA * ch, char *argument )
{
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *rch;
    int number;
    int count;

    number = number_argument( argument, arg2 );
    count = 0;

    if ( !str_cmp( arg2, "self" ) )
        return ch;

    if ( !ch->in_room )         /* Make sure in_room isn't NULL (ConsoleChar) - Zane */
        return NULL;

    for ( rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room )
    {
        if ( !IS_NPC( rch ) || !can_see( ch, rch )
             || !is_name( arg2, rch->name ) )
            continue;

        if ( ++count == number )
            return rch;
    }

    return NULL;
}

/*
 * Find a player in the room.
 */
CHAR_DATA *get_player_room( CHAR_DATA * ch, char *argument )
{
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *rch;
    int number;
    int count;

    number = number_argument( argument, arg2 );
    count = 0;

    if ( !str_cmp( arg2, "self" ) )
        return ch;

    if ( !ch->in_room )         /* Make sure in_room isn't NULL (ConsoleChar) - Zane */
        return NULL;

    for ( rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room )
    {
        if ( IS_NPC( rch ) || !can_see( ch, rch )
             || !is_name( arg2, rch->name ) )
            continue;
        if ( ++count == number )
            return rch;
    }

    return NULL;
}

/*
 * Find a char in the world.
 */
CHAR_DATA *get_char_world( CHAR_DATA * ch, char *argument )
{
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *wch;
    int number;
    int count;

    if ( ( wch = get_char_room( ch, argument ) ) != NULL )
        return wch;

    number = number_argument( argument, arg2 );
    count = 0;
    for ( wch = char_list; wch != NULL; wch = wch->next )
    {
        if ( wch->in_room == NULL || !can_see( ch, wch )
             || !is_name( arg2, wch->name ) )
            continue;
        if ( ++count == number )
            return wch;
    }

    return NULL;
}

/*
 * Find a mob in the world.
 * (Just like get_char_world() only it doesn't search PCs)
 */
CHAR_DATA *get_mob_world( CHAR_DATA * ch, char *argument )
{
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *wch;
    int number;
    int count;

    if ( ( wch = get_mob_room( ch, argument ) ) != NULL )
        return wch;

    number = number_argument( argument, arg2 );
    count = 0;

    for ( wch = char_list; wch != NULL; wch = wch->next )
    {
        if ( wch->in_room == NULL || !can_see( ch, wch )
             || !IS_NPC( wch ) || !is_name( arg2, wch->name ) )
            continue;

        if ( ++count == number )
            return wch;
    }

    return NULL;
}

/*
 * Find a player in the world.
 * (Just like get_char_world() only it doesn't search NPCs)
 */
CHAR_DATA *get_player_world( CHAR_DATA * ch, char *argument )
{
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *wch;
    int number;
    int count;

    if ( ( wch = get_player_room( ch, argument ) ) != NULL )
        return wch;

    number = number_argument( argument, arg2 );
    count = 0;

    for ( wch = player_list; wch != NULL; wch = wch->next_player )
    {
        if ( wch->in_room == NULL || !can_see( ch, wch )
             || !is_name( arg2, wch->name ) )
            continue;
        if ( ++count == number )
            return wch;
    }

    return NULL;
}

/*
 * Find some object with a given index data.
 * Used by area-reset 'P' command.
 */
OBJ_DATA *get_obj_type( OBJ_INDEX_DATA * pObjIndex )
{
    OBJ_DATA *obj;

    for ( obj = object_list; obj != NULL; obj = obj->next )
    {
        if ( obj->pIndexData == pObjIndex )
            return obj;
    }

    return NULL;
}

/*
 * Find an obj in a list.
 */
OBJ_DATA *get_obj_list( CHAR_DATA * ch, char *argument, OBJ_DATA * list )
{
    char arg2[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int number;
    int count;

    number = number_argument( argument, arg2 );
    count = 0;
    for ( obj = list; obj != NULL; obj = obj->next_content )
    {
        if ( can_see_obj( ch, obj ) && is_name( arg2, obj->name ) )
        {
            if ( ++count == number )
                return obj;
        }
    }

    return NULL;
}

/*
 * Find an obj in player's inventory.
 */
OBJ_DATA *get_obj_carry( CHAR_DATA * ch, char *argument )
{
    char arg2[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int number;
    int count;

    number = number_argument( argument, arg2 );
    count = 0;
    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
    {
        if ( obj->wear_loc == WEAR_NONE
             && ( can_see_obj( ch, obj ) ) && is_name( arg2, obj->name ) )
        {
            if ( ++count == number )
                return obj;
        }
    }

    return NULL;
}

/*
 * Find an obj in player's equipment.
 */
OBJ_DATA *get_obj_wear( CHAR_DATA * ch, char *argument )
{
    char arg2[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int number;
    int count;

    number = number_argument( argument, arg2 );
    count = 0;
    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
    {
        if ( obj->wear_loc != WEAR_NONE
             && can_see_obj( ch, obj ) && is_name( arg2, obj->name ) )
        {
            if ( ++count == number )
                return obj;
        }
    }

    return NULL;
}

/*
 * Find an obj in the room or in inventory.
 */
OBJ_DATA *get_obj_here( CHAR_DATA * ch, char *argument )
{
    OBJ_DATA *obj;

    obj = get_obj_list( ch, argument, ch->in_room->contents );
    if ( obj != NULL )
        return obj;

    if ( ( obj = get_obj_carry( ch, argument ) ) != NULL )
        return obj;

    if ( ( obj = get_obj_wear( ch, argument ) ) != NULL )
        return obj;

    return NULL;
}

/*
 * Find an obj in the world.
 */
OBJ_DATA *get_obj_world( CHAR_DATA * ch, char *argument )
{
    char arg2[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int number;
    int count;

    if ( ( obj = get_obj_here( ch, argument ) ) != NULL )
        return obj;

    number = number_argument( argument, arg2 );
    count = 0;
    for ( obj = object_list; obj != NULL; obj = obj->next )
    {
        if ( can_see_obj( ch, obj ) && is_name( arg2, obj->name ) )
        {
            if ( ++count == number )
                return obj;
        }
    }

    return NULL;
}

/*
 * Create a 'money' obj.
 */
OBJ_DATA *create_money( int amount )
{
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj;

    if ( amount <= 0 )
    {
        bug( "Create_money: zero or negative money %d.", amount );
        amount = 1;
    }

    if ( amount == 1 )
    {
        obj = create_object( get_obj_index( OBJ_VNUM_MONEY_ONE ), 0 );
    }
    else
    {
        obj = create_object( get_obj_index( OBJ_VNUM_MONEY_SOME ), 0 );
        sprintf( buf, obj->short_descr, amount );
        free_string( &obj->short_descr );
        obj->short_descr = str_dup( buf );
        obj->value[0] = amount;
        obj->cost = amount;
    }

    return obj;
}

/*
 * Return # of objects which an object counts as.
 * Thanks to Tony Chamberlain for the correct recursive code here.
 */
int get_obj_number( OBJ_DATA * obj )
{
    int number;

    if ( obj->item_type == ITEM_CONTAINER || obj->item_type == ITEM_MONEY )
        number = 0;
    else
        number = 1;

    for ( obj = obj->contains; obj != NULL; obj = obj->next_content )
        number += get_obj_number( obj );

    return number;
}

/*
 * Return weight of an object, including weight of contents.
 */
int get_obj_weight( OBJ_DATA * obj )
{
    int weight;

    weight = obj->weight;
    for ( obj = obj->contains; obj != NULL; obj = obj->next_content )
        weight += get_obj_weight( obj );

    return weight;
}

/*
 * True if room is dark.
 */
bool room_is_dark( ROOM_INDEX_DATA * pRoomIndex )
{
    if ( pRoomIndex->light > 0 )
        return FALSE;

    if ( IS_SET( pRoomIndex->room_flags, ROOM_DARK ) )
        return TRUE;

    if ( pRoomIndex->sector_type == SECT_INSIDE
         || pRoomIndex->sector_type == SECT_CITY )
        return FALSE;

    if ( weather_info.sunlight == SUN_SET || weather_info.sunlight == SUN_DARK )
        return TRUE;

    return FALSE;
}

/*
 * True if room is private.
 */
bool room_is_private( ROOM_INDEX_DATA * pRoomIndex )
{
    CHAR_DATA *rch;
    int count;

    count = 0;
    for ( rch = pRoomIndex->people; rch != NULL; rch = rch->next_in_room )
        count++;

    if ( IS_SET( pRoomIndex->room_flags, ROOM_PRIVATE ) && count >= 2 )
        return TRUE;

    if ( IS_SET( pRoomIndex->room_flags, ROOM_SOLITARY ) && count >= 1 )
        return TRUE;

    if ( IS_SET( pRoomIndex->room_flags, ROOM_IMP_ONLY ) )
        return TRUE;

    return FALSE;
}

/* visibility on a room -- for entering and exits */
bool can_see_room( CHAR_DATA * ch, ROOM_INDEX_DATA * pRoomIndex )
{
    if ( IS_SET( pRoomIndex->room_flags, ROOM_IMP_ONLY )
         && get_trust( ch ) < MAX_LEVEL )
        return FALSE;

    if ( IS_SET( pRoomIndex->room_flags, ROOM_GODS_ONLY )
         && !IS_IMMORTAL( ch ) )
        return FALSE;

    if ( IS_SET( pRoomIndex->room_flags, ROOM_HEROES_ONLY ) && !IS_HERO( ch ) )
        return FALSE;

    if ( IS_SET( pRoomIndex->room_flags, ROOM_ADMIN_ONLY ) && !IS_ADMIN( ch ) )
        return FALSE;

    if ( IS_SET( pRoomIndex->room_flags, ROOM_NEWBIES_ONLY )
         && ch->level > 5 && !IS_IMMORTAL( ch ) )
        return FALSE;

    return TRUE;
}

/*
 * True if char's race has access to the given skill
 */
bool has_racial_skill( CHAR_DATA * ch, long sn )
{
    long x;

    for ( x = 0; x < 5; x++ )
    {
        char *skill = pc_race_table[ch->race].skills[x];

        if ( skill == NULL )
            break;

        if ( sn == skill_lookup( skill ) )
            return TRUE;
    }

    return FALSE;
}

/*
 * True if char has access to the given skill
 */
bool can_use( CHAR_DATA * ch, long sn )
{
    /* NPC's can do anything */
    if ( IS_NPC( ch ) )
        return TRUE;

    /* Can the char's race do the skill? */
    if ( has_racial_skill( ch, sn ) )
        return TRUE;

    /* Otherwise, check the char's level against the minimum level
     * for the skill for the char's class.
     */
    if ( ch->level >= skill_table[sn].skill_level[ch->Class] )
        return TRUE;

    /* Otherwise, the char can't do that skill */
    return FALSE;
}

/*
 * True if char can practice a given skill
 */
bool can_practice( CHAR_DATA * ch, long sn )
{
    /* if ( sn == gsn_alcohol_tolerance )
     * return FALSE;
     */

    if ( sn < 0 || !can_use( ch, sn ) )
        return FALSE;

    if ( !IS_NPC( ch ) )
    {
        if ( ch->pcdata->learned[sn] < 1 )
            return FALSE;
    }

    return TRUE;
}

/*
 * True if char can see victim.
 */
bool can_see( CHAR_DATA * ch, CHAR_DATA * victim )
{
/* RT changed so that WIZ_INVIS has levels */
    if ( ch == victim )
        return TRUE;

    if ( IS_SET( victim->comm, VIS_CONSOLE )
         || IS_SET( ch->comm, VIS_CONSOLE ) )
        return TRUE;

    if ( !IS_NPC( victim )
         && IS_SET( victim->act, PLR_WIZINVIS )
         && get_trust( ch ) < victim->invis_level )
        return FALSE;

    if ( ( !IS_NPC( ch ) && IS_SET( ch->act, PLR_HOLYLIGHT ) )
         || ( IS_NPC( ch ) && IS_IMMORTAL( ch ) ) )
        return TRUE;

    if ( IS_AFFECTED( ch, AFF_BLIND ) )
        return FALSE;

    if ( room_is_dark( ch->in_room ) && !IS_AFFECTED( ch, AFF_INFRARED ) )
        return FALSE;

    if ( IS_AFFECTED( victim, AFF_INVISIBLE )
         && !IS_AFFECTED( ch, AFF_DETECT_INVIS ) )
        return FALSE;

    /* sneaking */
    if ( IS_AFFECTED( victim, AFF_SNEAK )
         && !IS_AFFECTED( ch, AFF_DETECT_HIDDEN )
         && victim->fighting == NULL
         && ( IS_NPC( ch ) ? !IS_NPC( victim ) : IS_NPC( victim ) ) )
    {
        int chance;
        chance = get_skill( victim, gsn_sneak );
        chance += get_curr_stat( ch, STAT_DEX ) * 3 / 2;
        chance -= get_curr_stat( ch, STAT_INT ) * 2;
        chance += ch->level - victim->level * 3 / 2;

        if ( number_percent(  ) < chance )
            return FALSE;
    }

    if ( IS_AFFECTED( victim, AFF_HIDE )
         && !IS_AFFECTED( ch, AFF_DETECT_HIDDEN ) && victim->fighting == NULL
/*    &&   ( IS_NPC(ch) ? !IS_NPC(victim) : IS_NPC(victim) )*/  )
        return FALSE;

    return TRUE;
}

/*
 * True if char can see obj.
 */
bool can_see_obj( CHAR_DATA * ch, OBJ_DATA * obj )
{
    if ( !IS_NPC( ch ) && IS_SET( ch->act, PLR_HOLYLIGHT ) )
        return TRUE;

    if ( IS_SET( obj->extra_flags, ITEM_VIS_DEATH ) )
        return FALSE;

    if ( IS_AFFECTED( ch, AFF_BLIND ) && obj->item_type != ITEM_POTION )
        return FALSE;

    if ( obj->item_type == ITEM_LIGHT && obj->value[2] != 0 )
        return TRUE;

    if ( IS_SET( obj->extra_flags, ITEM_INVIS )
         && !IS_AFFECTED( ch, AFF_DETECT_INVIS ) )
        return FALSE;

    if ( IS_OBJ_STAT( obj, ITEM_GLOW ) )
        return TRUE;

    if ( room_is_dark( ch->in_room ) && !IS_AFFECTED( ch, AFF_INFRARED ) )
        return FALSE;

    return TRUE;
}

/*
 * True if char can drop obj.
 */
bool can_drop_obj( CHAR_DATA * ch, OBJ_DATA * obj )
{
    if ( !IS_SET( obj->extra_flags, ITEM_NODROP ) )
        return TRUE;

    if ( !IS_NPC( ch ) && ch->level >= LEVEL_IMMORTAL )
        return TRUE;

    return FALSE;
}

/*
 * Return ascii name of an item type.
 */
char *item_type_name( OBJ_DATA * obj )
{
    switch ( obj->item_type )
    {
    case ITEM_LIGHT:
        return "light";
    case ITEM_SCROLL:
        return "scroll";
    case ITEM_WAND:
        return "wand";
    case ITEM_STAFF:
        return "staff";
    case ITEM_WEAPON:
        return "weapon";
    case ITEM_TREASURE:
        return "treasure";
    case ITEM_ARMOR:
        return "armor";
    case ITEM_POTION:
        return "potion";
    case ITEM_TRASH:
        return "trash";
    case ITEM_CONTAINER:
        return "container";
    case ITEM_DRINK_CON:
        return "drink container";
    case ITEM_KEY:
        return "key";
    case ITEM_FOOD:
        return "food";
    case ITEM_MONEY:
        return "money";
    case ITEM_BOAT:
        return "boat";
    case ITEM_CORPSE_NPC:
        return "npc corpse";
    case ITEM_CORPSE_PC:
        return "pc corpse";
    case ITEM_FOUNTAIN:
        return "fountain";
    case ITEM_PILL:
        return "pill";
    case ITEM_MAP:
        return "map";
    case ITEM_FURNITURE:
        return "furniture";
    case ITEM_PORTAL:
        return "portal";
    }

    bug( "Item_type_name: unknown type %d.", obj->item_type );
    return "(unknown)";
}

/*
 * Return ascii name of an affect location.
 */
char *affect_loc_name( int location )
{
    switch ( location )
    {
    case APPLY_NONE:
        return "none";
    case APPLY_STR:
        return "strength";
    case APPLY_DEX:
        return "dexterity";
    case APPLY_INT:
        return "intelligence";
    case APPLY_WIS:
        return "wisdom";
    case APPLY_CON:
        return "constitution";
    case APPLY_SEX:
        return "sex";
    case APPLY_CLASS:
        return "class";
    case APPLY_LEVEL:
        return "level";
    case APPLY_AGE:
        return "age";
    case APPLY_MANA:
        return "mana";
    case APPLY_HIT:
        return "hp";
    case APPLY_MOVE:
        return "moves";
    case APPLY_GOLD:
        return "gold";
    case APPLY_EXP:
        return "experience";
    case APPLY_AC:
        return "armor class";
    case APPLY_HITROLL:
        return "hit roll";
    case APPLY_DAMROLL:
        return "damage roll";
    case APPLY_SAVING_PARA:
        return "save vs paralysis";
    case APPLY_SAVING_ROD:
        return "save vs rod";
    case APPLY_SAVING_PETRI:
        return "save vs petrification";
    case APPLY_SAVING_BREATH:
        return "save vs breath";
    case APPLY_SAVING_SPELL:
        return "save vs spell";
    case APPLY_ALIGN:
        return "alignment";
    }

    bug( "Affect_location_name: unknown location %d.", location );
    return "(unknown)";
}

/*
 * Return ascii name of an affect bit vector.
 */
char *affect_bit_name( int vector )
{
    char buf[MAX_STRING_LENGTH];
    buf[0] = '\0';
    if ( vector & AFF_BLIND )
        strcat( buf, " blind" );
    if ( vector & AFF_INVISIBLE )
        strcat( buf, " invisible" );
    if ( vector & AFF_DETECT_EVIL )
        strcat( buf, " detect_evil" );
    if ( vector & AFF_DETECT_INVIS )
        strcat( buf, " detect_invis" );
    if ( vector & AFF_DETECT_MAGIC )
        strcat( buf, " detect_magic" );
    if ( vector & AFF_DETECT_HIDDEN )
        strcat( buf, " detect_hidden" );
    if ( vector & AFF_HOLD )
        strcat( buf, " hold" );
    if ( vector & AFF_SANCTUARY )
        strcat( buf, " sanctuary" );
    if ( vector & AFF_FAERIE_FIRE )
        strcat( buf, " faerie_fire" );
    if ( vector & AFF_INFRARED )
        strcat( buf, " infrared" );
    if ( vector & AFF_CURSE )
        strcat( buf, " curse" );
    if ( vector & AFF_FLAMING )
        strcat( buf, " flaming" );
    if ( vector & AFF_POISON )
        strcat( buf, " poison" );
    if ( vector & AFF_PROTECT )
        strcat( buf, " protect" );
    if ( vector & AFF_PARALYSIS )
        strcat( buf, " paralysis" );
    if ( vector & AFF_SLEEP )
        strcat( buf, " sleep" );
    if ( vector & AFF_SNEAK )
        strcat( buf, " sneak" );
    if ( vector & AFF_HIDE )
        strcat( buf, " hide" );
    if ( vector & AFF_CHARM )
        strcat( buf, " charm" );
    if ( vector & AFF_FLYING )
        strcat( buf, " flying" );
    if ( vector & AFF_PASS_DOOR )
        strcat( buf, " pass_door" );
    if ( vector & AFF_BERSERK )
        strcat( buf, " berserk" );
    if ( vector & AFF_CALM )
        strcat( buf, " calm" );
    if ( vector & AFF_HASTE )
        strcat( buf, " haste" );
    if ( vector & AFF_PLAGUE )
        strcat( buf, " plague" );
    if ( vector & AFF_DARK_VISION )
        strcat( buf, " dark_vision" );
    if ( vector & AFF_SWIM )
        strcat( buf, " swim" );
    if ( vector & AFF_REGENERATION )
        strcat( buf, " regeneration" );
    if ( vector & AFF_WEB )
        strcat( buf, " web" );
    return ( buf[0] != '\0' ) ? buf + 1 : "none";
}

/*
 * Return ascii name of extra flags vector.
 */
char *extra_bit_name( int extra_flags )
{
    char buf[MAX_STRING_LENGTH];
    buf[0] = '\0';
    if ( extra_flags & ITEM_GLOW )
        strcat( buf, " glow" );
    if ( extra_flags & ITEM_HUM )
        strcat( buf, " hum" );
    if ( extra_flags & ITEM_DARK )
        strcat( buf, " dark" );
    if ( extra_flags & ITEM_LOCK )
        strcat( buf, " lock" );
    if ( extra_flags & ITEM_EVIL )
        strcat( buf, " evil" );
    if ( extra_flags & ITEM_INVIS )
        strcat( buf, " invis" );
    if ( extra_flags & ITEM_MAGIC )
        strcat( buf, " magic" );
    if ( extra_flags & ITEM_NODROP )
        strcat( buf, " nodrop" );
    if ( extra_flags & ITEM_BLESS )
        strcat( buf, " bless" );
    if ( extra_flags & ITEM_ANTI_GOOD )
        strcat( buf, " anti-good" );
    if ( extra_flags & ITEM_ANTI_EVIL )
        strcat( buf, " anti-evil" );
    if ( extra_flags & ITEM_ANTI_NEUTRAL )
        strcat( buf, " anti-neutral" );
    if ( extra_flags & ITEM_NOREMOVE )
        strcat( buf, " noremove" );
    if ( extra_flags & ITEM_INVENTORY )
        strcat( buf, " inventory" );
    if ( extra_flags & ITEM_NOPURGE )
        strcat( buf, " nopurge" );
    if ( extra_flags & ITEM_VIS_DEATH )
        strcat( buf, " vis_death" );
    if ( extra_flags & ITEM_ROT_DEATH )
        strcat( buf, " rot_death" );
    if ( extra_flags & ITEM_ROT_PLAYER_DEATH )
        strcat( buf, " rot_pc_death" );
    return ( buf[0] != '\0' ) ? buf + 1 : "none";
}

/* return ascii name of an act vector */
char *act_bit_name( int act_flags )
{
    char buf[MAX_STRING_LENGTH];
    buf[0] = '\0';

    if ( IS_SET( act_flags, ACT_IS_NPC ) )
    {
        strcat( buf, " npc" );
        if ( act_flags & ACT_SENTINEL )
            strcat( buf, " sentinel" );
        if ( act_flags & ACT_SCAVENGER )
            strcat( buf, " scavenger" );
        if ( act_flags & ACT_AGGRESSIVE )
            strcat( buf, " aggressive" );
        if ( act_flags & ACT_STAY_AREA )
            strcat( buf, " stay_area" );
        if ( act_flags & ACT_WIMPY )
            strcat( buf, " wimpy" );
        if ( act_flags & ACT_PET )
            strcat( buf, " pet" );
        if ( act_flags & ACT_TRAIN )
            strcat( buf, " train" );
        if ( act_flags & ACT_PRACTICE )
            strcat( buf, " practice" );
        if ( act_flags & ACT_UNDEAD )
            strcat( buf, " undead" );
        if ( act_flags & ACT_CLERIC )
            strcat( buf, " cleric" );
        if ( act_flags & ACT_MAGE )
            strcat( buf, " mage" );
        if ( act_flags & ACT_THIEF )
            strcat( buf, " thief" );
        if ( act_flags & ACT_WARRIOR )
            strcat( buf, " warrior" );
        if ( act_flags & ACT_NOALIGN )
            strcat( buf, " no_align" );
        if ( act_flags & ACT_NOPURGE )
            strcat( buf, " no_purge" );
        if ( act_flags & ACT_IS_HEALER )
            strcat( buf, " healer" );
        if ( act_flags & ACT_GAIN )
            strcat( buf, " skill_train" );
        if ( act_flags & ACT_UPDATE_ALWAYS )
            strcat( buf, " update_always" );
    }
    else
    {
        strcat( buf, " player" );
        if ( act_flags & PLR_BOUGHT_PET )
            strcat( buf, " owner" );
        if ( act_flags & PLR_AUTOASSIST )
            strcat( buf, " autoassist" );
        if ( act_flags & PLR_AUTOEXIT )
            strcat( buf, " autoexit" );
        if ( act_flags & PLR_AUTOLOOT )
            strcat( buf, " autoloot" );
        if ( act_flags & PLR_AUTOSAC )
            strcat( buf, " autosac" );
        if ( act_flags & PLR_AUTOGOLD )
            strcat( buf, " autogold" );
        if ( act_flags & PLR_AUTOSPLIT )
            strcat( buf, " autosplit" );
        if ( act_flags & PLR_HOLYLIGHT )
            strcat( buf, " holy_light" );
        if ( act_flags & PLR_WIZINVIS )
            strcat( buf, " wizinvis" );
        if ( act_flags & PLR_CANLOOT )
            strcat( buf, " loot_corpse" );
        if ( act_flags & PLR_NOSUMMON )
            strcat( buf, " no_summon" );
        if ( act_flags & PLR_NOFOLLOW )
            strcat( buf, " no_follow" );
        if ( act_flags & PLR_FREEZE )
            strcat( buf, " frozen" );
        if ( act_flags & PLR_THIEF )
            strcat( buf, " thief" );
        if ( act_flags & PLR_KILLER )
            strcat( buf, " killer" );
        if ( act_flags & PLR_PERMIT )
            strcat( buf, " permit" );
    }
    return ( buf[0] != '\0' ) ? buf + 1 : "none";
}

char *comm_bit_name( int comm_flags )
{
    char buf[MAX_STRING_LENGTH];
    buf[0] = '\0';

    if ( comm_flags & COMM_QUIET )
        strcat( buf, " quiet" );
    if ( comm_flags & COMM_DEAF )
        strcat( buf, " deaf" );
    if ( comm_flags & COMM_NOWIZ )
        strcat( buf, " no_wiz" );
    if ( comm_flags & COMM_NOADMIN )
        strcat( buf, " no_admin" );
    if ( comm_flags & COMM_NOHERO )
        strcat( buf, " no_hero" );
    if ( comm_flags & COMM_NOAUCTION )
        strcat( buf, " no_auction" );
    if ( comm_flags & COMM_NOGOSSIP )
        strcat( buf, " no_gossip" );
    if ( comm_flags & COMM_NOMUSIC )
        strcat( buf, " no_nomusic" );
    if ( comm_flags & COMM_NOQUESTION )
        strcat( buf, " no_question" );
    if ( comm_flags & COMM_NO_OOC )
        strcat( buf, " no_ooc" );
    if ( comm_flags & COMM_COMPACT )
        strcat( buf, " compact" );
    if ( comm_flags & COMM_BRIEF )
        strcat( buf, " brief" );
    if ( comm_flags & COMM_PROMPT )
        strcat( buf, " prompt" );
    if ( comm_flags & COMM_COMBINE )
        strcat( buf, " combine" );
    if ( comm_flags & COMM_NOEMOTE )
        strcat( buf, " no_emote" );
    if ( comm_flags & COMM_NOSHOUT )
        strcat( buf, " no_shout" );
    if ( comm_flags & COMM_NOTELL )
        strcat( buf, " no_tell" );
    if ( comm_flags & COMM_NOCHANNELS )
        strcat( buf, " no_channels" );

    return ( buf[0] != '\0' ) ? buf + 1 : "none";
}

char *imm_bit_name( int imm_flags )
{
    char buf[MAX_STRING_LENGTH];
    buf[0] = '\0';

    if ( imm_flags & IMM_SUMMON )
        strcat( buf, " summon" );
    if ( imm_flags & IMM_CHARM )
        strcat( buf, " charm" );
    if ( imm_flags & IMM_MAGIC )
        strcat( buf, " magic" );
    if ( imm_flags & IMM_WEAPON )
        strcat( buf, " weapon" );
    if ( imm_flags & IMM_BASH )
        strcat( buf, " blunt" );
    if ( imm_flags & IMM_PIERCE )
        strcat( buf, " piercing" );
    if ( imm_flags & IMM_SLASH )
        strcat( buf, " slashing" );
    if ( imm_flags & IMM_FIRE )
        strcat( buf, " fire" );
    if ( imm_flags & IMM_COLD )
        strcat( buf, " cold" );
    if ( imm_flags & IMM_LIGHTNING )
        strcat( buf, " lightning" );
    if ( imm_flags & IMM_ACID )
        strcat( buf, " acid" );
    if ( imm_flags & IMM_POISON )
        strcat( buf, " poison" );
    if ( imm_flags & IMM_NEGATIVE )
        strcat( buf, " negative" );
    if ( imm_flags & IMM_HOLY )
        strcat( buf, " holy" );
    if ( imm_flags & IMM_ENERGY )
        strcat( buf, " energy" );
    if ( imm_flags & IMM_MENTAL )
        strcat( buf, " mental" );
    if ( imm_flags & IMM_DISEASE )
        strcat( buf, " disease" );
    if ( imm_flags & IMM_DROWNING )
        strcat( buf, " drowning" );
    if ( imm_flags & IMM_LIGHT )
        strcat( buf, " light" );
    if ( imm_flags & VULN_IRON )
        strcat( buf, " iron" );
    if ( imm_flags & VULN_WOOD )
        strcat( buf, " wood" );
    if ( imm_flags & VULN_SILVER )
        strcat( buf, " silver" );

    return ( buf[0] != '\0' ) ? buf + 1 : "none";
}

char *wear_bit_name( int wear_flags )
{
    char buf[MAX_STRING_LENGTH];
    buf[0] = '\0';
    if ( wear_flags & ITEM_TAKE )
        strcat( buf, " take" );
    if ( wear_flags & ITEM_WEAR_FINGER )
        strcat( buf, " finger" );
    if ( wear_flags & ITEM_WEAR_NECK )
        strcat( buf, " neck" );
    if ( wear_flags & ITEM_WEAR_BODY )
        strcat( buf, " torso" );
    if ( wear_flags & ITEM_WEAR_HEAD )
        strcat( buf, " head" );
    if ( wear_flags & ITEM_WEAR_LEGS )
        strcat( buf, " legs" );
    if ( wear_flags & ITEM_WEAR_FEET )
        strcat( buf, " feet" );
    if ( wear_flags & ITEM_WEAR_HANDS )
        strcat( buf, " hands" );
    if ( wear_flags & ITEM_WEAR_ARMS )
        strcat( buf, " arms" );
    if ( wear_flags & ITEM_WEAR_SHIELD )
        strcat( buf, " shield" );
    if ( wear_flags & ITEM_WEAR_ABOUT )
        strcat( buf, " body" );
    if ( wear_flags & ITEM_WEAR_WAIST )
        strcat( buf, " waist" );
    if ( wear_flags & ITEM_WEAR_WRIST )
        strcat( buf, " wrist" );
    if ( wear_flags & ITEM_WIELD )
        strcat( buf, " wield" );
    if ( wear_flags & ITEM_HOLD )
        strcat( buf, " hold" );

    return ( buf[0] != '\0' ) ? buf + 1 : "none";
}

char *form_bit_name( int form_flags )
{
    char buf[MAX_STRING_LENGTH];
    buf[0] = '\0';
    if ( form_flags & FORM_POISON )
        strcat( buf, " poison" );
    else if ( form_flags & FORM_EDIBLE )
        strcat( buf, " edible" );
    if ( form_flags & FORM_MAGICAL )
        strcat( buf, " magical" );
    if ( form_flags & FORM_INSTANT_DECAY )
        strcat( buf, " instant_rot" );
    if ( form_flags & FORM_OTHER )
        strcat( buf, " other" );
    if ( form_flags & FORM_ANIMAL )
        strcat( buf, " animal" );
    if ( form_flags & FORM_SENTIENT )
        strcat( buf, " sentient" );
    if ( form_flags & FORM_UNDEAD )
        strcat( buf, " undead" );
    if ( form_flags & FORM_CONSTRUCT )
        strcat( buf, " construct" );
    if ( form_flags & FORM_MIST )
        strcat( buf, " mist" );
    if ( form_flags & FORM_INTANGIBLE )
        strcat( buf, " intangible" );
    if ( form_flags & FORM_BIPED )
        strcat( buf, " biped" );
    if ( form_flags & FORM_CENTAUR )
        strcat( buf, " centaur" );
    if ( form_flags & FORM_INSECT )
        strcat( buf, " insect" );
    if ( form_flags & FORM_SPIDER )
        strcat( buf, " spider" );
    if ( form_flags & FORM_CRUSTACEAN )
        strcat( buf, " crustacean" );
    if ( form_flags & FORM_WORM )
        strcat( buf, " worm" );
    if ( form_flags & FORM_BLOB )
        strcat( buf, " blob" );
    if ( form_flags & FORM_MAMMAL )
        strcat( buf, " mammal" );
    if ( form_flags & FORM_BIRD )
        strcat( buf, " bird" );
    if ( form_flags & FORM_REPTILE )
        strcat( buf, " reptile" );
    if ( form_flags & FORM_SNAKE )
        strcat( buf, " snake" );
    if ( form_flags & FORM_DRAGON )
        strcat( buf, " dragon" );
    if ( form_flags & FORM_AMPHIBIAN )
        strcat( buf, " amphibian" );
    if ( form_flags & FORM_FISH )
        strcat( buf, " fish" );
    if ( form_flags & FORM_COLD_BLOOD )
        strcat( buf, " cold_blooded" );

    return ( buf[0] != '\0' ) ? buf + 1 : "none";
}

char *part_bit_name( int part_flags )
{
    char buf[MAX_STRING_LENGTH];
    buf[0] = '\0';
    if ( part_flags & PART_HEAD )
        strcat( buf, " head" );
    if ( part_flags & PART_ARMS )
        strcat( buf, " arms" );
    if ( part_flags & PART_LEGS )
        strcat( buf, " legs" );
    if ( part_flags & PART_HEART )
        strcat( buf, " heart" );
    if ( part_flags & PART_BRAINS )
        strcat( buf, " brains" );
    if ( part_flags & PART_GUTS )
        strcat( buf, " guts" );
    if ( part_flags & PART_HANDS )
        strcat( buf, " hands" );
    if ( part_flags & PART_FEET )
        strcat( buf, " feet" );
    if ( part_flags & PART_FINGERS )
        strcat( buf, " fingers" );
    if ( part_flags & PART_EAR )
        strcat( buf, " ears" );
    if ( part_flags & PART_EYE )
        strcat( buf, " eyes" );
    if ( part_flags & PART_LONG_TONGUE )
        strcat( buf, " long_tongue" );
    if ( part_flags & PART_EYESTALKS )
        strcat( buf, " eyestalks" );
    if ( part_flags & PART_TENTACLES )
        strcat( buf, " tentacles" );
    if ( part_flags & PART_FINS )
        strcat( buf, " fins" );
    if ( part_flags & PART_WINGS )
        strcat( buf, " wings" );
    if ( part_flags & PART_TAIL )
        strcat( buf, " tail" );
    if ( part_flags & PART_CLAWS )
        strcat( buf, " claws" );
    if ( part_flags & PART_FANGS )
        strcat( buf, " fangs" );
    if ( part_flags & PART_HORNS )
        strcat( buf, " horns" );
    if ( part_flags & PART_SCALES )
        strcat( buf, " scales" );

    return ( buf[0] != '\0' ) ? buf + 1 : "none";
}

char *weapon_bit_name( int weapon_flags )
{
    char buf[MAX_STRING_LENGTH];
    buf[0] = '\0';
    if ( weapon_flags & WEAPON_FLAMING )
        strcat( buf, " flaming" );
    if ( weapon_flags & WEAPON_FROST )
        strcat( buf, " frost" );
    if ( weapon_flags & WEAPON_VAMPIRIC )
        strcat( buf, " vampiric" );
    if ( weapon_flags & WEAPON_SHARP )
        strcat( buf, " sharp" );
    if ( weapon_flags & WEAPON_VORPAL )
        strcat( buf, " vorpal" );
    if ( weapon_flags & WEAPON_TWO_HANDS )
        strcat( buf, " two-handed" );

    return ( buf[0] != '\0' ) ? buf + 1 : "none";
}

char *off_bit_name( int off_flags )
{
    char buf[MAX_STRING_LENGTH];
    buf[0] = '\0';

    if ( off_flags & OFF_AREA_ATTACK )
        strcat( buf, " area attack" );
    if ( off_flags & OFF_BACKSTAB )
        strcat( buf, " backstab" );
    if ( off_flags & OFF_BASH )
        strcat( buf, " bash" );
    if ( off_flags & OFF_BERSERK )
        strcat( buf, " berserk" );
    if ( off_flags & OFF_DISARM )
        strcat( buf, " disarm" );
    if ( off_flags & OFF_DODGE )
        strcat( buf, " dodge" );
    if ( off_flags & OFF_FADE )
        strcat( buf, " fade" );
    if ( off_flags & OFF_FAST )
        strcat( buf, " fast" );
    if ( off_flags & OFF_KICK )
        strcat( buf, " kick" );
    if ( off_flags & OFF_KICK_DIRT )
        strcat( buf, " kick_dirt" );
    if ( off_flags & OFF_PARRY )
        strcat( buf, " parry" );
    if ( off_flags & OFF_RESCUE )
        strcat( buf, " rescue" );
    if ( off_flags & OFF_TAIL )
        strcat( buf, " tail" );
    if ( off_flags & OFF_TRIP )
        strcat( buf, " trip" );
    if ( off_flags & OFF_CRUSH )
        strcat( buf, " crush" );
    if ( off_flags & ASSIST_ALL )
        strcat( buf, " assist_all" );
    if ( off_flags & ASSIST_ALIGN )
        strcat( buf, " assist_align" );
    if ( off_flags & ASSIST_RACE )
        strcat( buf, " assist_race" );
    if ( off_flags & ASSIST_PLAYERS )
        strcat( buf, " assist_players" );
    if ( off_flags & ASSIST_GUARD )
        strcat( buf, " assist_guard" );
    if ( off_flags & ASSIST_VNUM )
        strcat( buf, " assist_vnum" );

    return ( buf[0] != '\0' ) ? buf + 1 : "none";
}

AFFECT_DATA *affect_find( AFFECT_DATA * paf, int sn )
{
    AFFECT_DATA *paf_find;

    for ( paf_find = paf; paf_find != NULL; paf_find = paf_find->next )
    {
        if ( paf_find->type == sn )
            return paf_find;
    }

    return NULL;
}
