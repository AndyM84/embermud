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

#if defined(WIN32)
#include <time.h>
#endif
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "merc.h"

extern bool can_use( CHAR_DATA * ch, long sn );

/* command procedures needed */
DECLARE_DO_FUN( do_quit );
DECLARE_DO_FUN( do_say );       /* added for AUTO_HATE */

/*
 * Local functions.
 */
int hit_gain args( ( CHAR_DATA * ch ) );
int mana_gain args( ( CHAR_DATA * ch ) );
int move_gain args( ( CHAR_DATA * ch ) );
void mobile_update args( ( void ) );
void weather_update args( ( void ) );
void char_update args( ( void ) );
void regen_update args( ( void ) );
void obj_update args( ( void ) );
void aggr_update args( ( void ) );

/* used for saving */

int save_number = 0;

/*
 * Advancement stuff.
 */
void advance_level( CHAR_DATA * ch )
{
    char buf[MAX_STRING_LENGTH];
    int incarn;
    int add_hp;
    int add_mana;
    int add_move;
    int add_prac;

    incarn = 0;

    incarn = ch->incarnations;

    ch->pcdata->last_level =
        ( ch->played + ( int ) ( current_time - ch->logon ) ) / 3600;
    add_hp =
        con_app[get_curr_stat( ch, STAT_CON )].hitp +
        number_range( class_table[ch->Class].hp_min,
                      class_table[ch->Class].hp_max );
    add_mana =
        number_range( 2,
                      ( 2 * get_curr_stat( ch, STAT_INT ) +
                        get_curr_stat( ch, STAT_WIS ) ) / 5 );
    if ( !class_table[ch->Class].fMana )
        add_mana /= 2;
    add_move = number_range( 1, ( get_curr_stat( ch, STAT_CON )
                                  + get_curr_stat( ch, STAT_DEX ) ) / 6 );
    add_prac = wis_app[get_curr_stat( ch, STAT_WIS )].practice;

    add_hp = add_hp * 9 / 10;
    add_mana = add_mana * 9 / 10;
    add_move = add_move * 9 / 10;
#ifdef BONUS_INCARNATIONS
    if ( ch->incarnations != 0 );
    {
        add_hp = add_hp * ( 1 + ( ch->incarnations / 10 ) );
        add_mana = add_mana * ( 1 + ( ch->incarnations / 10 ) );
        add_move = add_move * ( 1 + ( ch->incarnations / 10 ) );
    }
#endif
    add_hp = UMAX( 1, add_hp );
    add_mana = UMAX( 1, add_mana );
    add_move = UMAX( 6, add_move );

    ch->exp -= exp_per_level( ch, ch->pcdata->points );
    ch->max_hit += add_hp;
    ch->max_mana += add_mana;
    ch->max_move += add_move;
    ch->practice += add_prac;
    ch->train += 1;

    ch->pcdata->perm_hit += add_hp;
    ch->pcdata->perm_mana += add_mana;
    ch->pcdata->perm_move += add_move;

    if ( !IS_NPC( ch ) )
        REMOVE_BIT( ch->act, PLR_BOUGHT_PET );

    sprintf( buf,
             "Your gain is: %d/%d hp, %d/%d m, %d/%d mv %d/%d prac.\n\r",
             add_hp, ch->max_hit,
             add_mana, ch->max_mana,
             add_move, ch->max_move, add_prac, ch->practice );
    log_string( buf );
    send_to_char( buf, ch );
    return;
}

void gain_exp( CHAR_DATA * ch, int gain )
{
    if ( IS_NPC( ch ) || ch->level >= LEVEL_HERO )
        return;

    /* XP tuner here.  Makes it easier for us to adjust how fast people are leveling. -Zane */
    /* Commented out here.  If we modify it here, they see their exp 
       awards in an unmodified amount, but gain the modified amount.
       Players notice.  Moved to part of the xp_compute sections 
       - Dorzak 11/28/2001 */
    /*  if ( gain > 0 ) */
    /*      gain = ( int ) ( gain * EXP_MULTIPLIER ); */

    // If the gain would give ch more than MAX_EXP, just give them exactly MAX_EXP.
    if ( gain > ( MAX_EXP - ch->exp ) )
    {
        ch->exp = MAX_EXP;
    }
    // If the gain would be negative (a loss) AND would be more
    // negative than the player is currently positive (in other
    // words, would give them negative xp), just give them 0 xp.
    else if ( gain < 0 && abs( gain ) > ch->exp )
    {
        ch->exp = 0;
    }
    else
    {
        ch->exp += gain;
    }

    return;
}

/*
 * Regeneration stuff.
 */
int hit_gain( CHAR_DATA * ch )
{
    int gain;
    int number;
    extern bool chaos;

    if ( IS_NPC( ch ) )
    {
        gain = 5 + ch->level;

        switch ( ch->position )
        {
        default:
            gain /= 2;
            break;
        case POS_SLEEPING:
            gain = 3 * gain / 2;
            break;
        case POS_RESTING:
            break;
        case POS_FIGHTING:
            gain /= 3;
            break;
        }

    }
    else
    {
        gain = UMAX( 3, get_curr_stat( ch, STAT_CON ) - 3 + ch->level / 2 );
        gain += class_table[ch->Class].hp_max - 10;
        number = number_percent(  );
        if ( can_use( ch, gsn_fast_healing )
             && number < ch->pcdata->learned[gsn_fast_healing] )
        {
            gain += number * gain / 100;
            if ( ch->hit < ch->max_hit )
                check_improve( ch, gsn_fast_healing, TRUE, 8 );
        }

        switch ( ch->position )
        {
        default:
            gain /= 4;
            break;
        case POS_SLEEPING:
            break;
        case POS_RESTING:
            gain /= 2;
            break;
        case POS_FIGHTING:
            gain /= 6;
            break;
        }

        if ( ch->pcdata->condition[COND_FULL] == 0 )
            gain /= 2;

        if ( ch->pcdata->condition[COND_THIRST] == 0 )
            gain /= 2;

    }
    if ( ch->on != NULL && ch->on->item_type == ITEM_FURNITURE )
        gain = gain * ch->on->value[3] / 100;

    if ( IS_AFFECTED( ch, AFF_POISON ) )
        gain /= 4;

    if ( IS_AFFECTED( ch, AFF_PLAGUE ) )
        gain /= 8;

    if ( IS_AFFECTED( ch, AFF_HASTE ) )
        gain /= 2;

    if ( chaos )
        gain /= 2;

    return UMIN( gain, ch->max_hit - ch->hit );
}

int mana_gain( CHAR_DATA * ch )
{
    int gain;
    int number;
    extern bool chaos;

    if ( IS_NPC( ch ) )
    {
        gain = 5 + ch->level;
        switch ( ch->position )
        {
        default:
            gain /= 2;
            break;
        case POS_SLEEPING:
            gain = 3 * gain / 2;
            break;
        case POS_RESTING:
            break;
        case POS_FIGHTING:
            gain /= 3;
            break;
        }
    }
    else
    {
        gain = ( get_curr_stat( ch, STAT_WIS )
                 + get_curr_stat( ch, STAT_INT ) + ch->level ) / 2;
        number = number_percent(  );
        if ( number < ch->pcdata->learned[gsn_meditation] )
        {
            gain += number * gain / 100;
            if ( ch->mana < ch->max_mana )
                check_improve( ch, gsn_meditation, TRUE, 8 );
        }
        if ( !class_table[ch->Class].fMana )
            gain /= 2;

        switch ( ch->position )
        {
        default:
            gain /= 4;
            break;
        case POS_SLEEPING:
            break;
        case POS_RESTING:
            gain /= 2;
            break;
        case POS_FIGHTING:
            gain /= 6;
            break;
        }

        if ( ch->pcdata->condition[COND_FULL] == 0 )
            gain /= 2;

        if ( ch->pcdata->condition[COND_THIRST] == 0 )
            gain /= 2;

    }

    if ( ch->on != NULL && ch->on->item_type == ITEM_FURNITURE )
        gain = gain * ch->on->value[4] / 100;

    if ( IS_AFFECTED( ch, AFF_POISON ) )
        gain /= 4;

    if ( IS_AFFECTED( ch, AFF_PLAGUE ) )
        gain /= 8;

    if ( IS_AFFECTED( ch, AFF_HASTE ) )
        gain /= 2;

    if ( chaos )
        gain /= 2;

    return UMIN( gain, ch->max_mana - ch->mana );
}

int move_gain( CHAR_DATA * ch )
{
    int gain;

    if ( IS_NPC( ch ) )
    {
        gain = ch->level;
    }
    else
    {
        gain = UMAX( 15, ch->level );

        switch ( ch->position )
        {
        case POS_SLEEPING:
            gain += get_curr_stat( ch, STAT_DEX );
            break;
        case POS_RESTING:
            gain += get_curr_stat( ch, STAT_DEX ) / 2;
            break;
        }

        if ( ch->pcdata->condition[COND_FULL] == 0 )
            gain /= 2;

        if ( ch->pcdata->condition[COND_THIRST] == 0 )
            gain /= 2;
    }

    if ( ch->on != NULL && ch->on->item_type == ITEM_FURNITURE )
        gain = gain * ch->on->value[3] / 100;

    if ( IS_AFFECTED( ch, AFF_POISON ) )
        gain /= 4;

    if ( IS_AFFECTED( ch, AFF_PLAGUE ) )
        gain /= 8;

    if ( IS_AFFECTED( ch, AFF_HASTE ) )
        gain /= 2;

    return UMIN( gain, ch->max_move - ch->move );
}

void gain_condition( CHAR_DATA * ch, int iCond, int value )
{
    int condition;

    if ( value == 0 || IS_NPC( ch ) || ch->level >= LEVEL_HERO )
        return;

    condition = ch->pcdata->condition[iCond];
    if ( condition == -1 )
        return;
    ch->pcdata->condition[iCond] = URANGE( 0, condition + value, 48 );

    if ( ch->pcdata->condition[iCond] == 0 )
    {
        switch ( iCond )
        {
        case COND_FULL:
            send_to_char( "You are hungry.\n\r", ch );
            break;

        case COND_THIRST:
            send_to_char( "You are thirsty.\n\r", ch );
            break;

        case COND_DRUNK:
            if ( condition != 0 )
                send_to_char( "You are sober.\n\r", ch );
            break;
        }
    }

    return;
}

/*
 * Mob autonomous action.
 * This function takes 25% to 35% of ALL Merc cpu time.
 * -- Furey
 */
void mobile_update( void )
{
    CHAR_DATA *ch;
    CHAR_DATA *ch_next;
    EXIT_DATA *pexit;
    int door;

    /* Examine all mobs. */
    for ( ch = char_list; ch != NULL; ch = ch_next )
    {
        ch_next = ch->next;

        if ( !IS_NPC( ch ) )
            continue;

        /* If who we're remembering is gone, forget them! */
        if ( ch->memory && !get_char_world( ch, ch->memory->name ) )
            ch->memory = NULL;

        if ( ch->in_room == NULL || IS_AFFECTED( ch, AFF_CHARM ) )
            continue;

        if ( ch->in_room->area->empty && !IS_SET( ch->act, ACT_UPDATE_ALWAYS ) )
            continue;

        if ( ch->in_room->area->nplayer > 0
             || IS_SET( ch->act, ACT_UPDATE_ALWAYS ) )
        {
            mprog_random_trigger( ch );
            /* If ch dies or changes
               position due to it's random
               trigger, continue - Kahn */
            if ( ch->position < POS_STANDING )
                continue;
        }

        /* That's all for sleeping / busy monster, and empty zones */
        if ( ch->position != POS_STANDING )
            continue;

        /* Scavenge */
        if ( IS_SET( ch->act, ACT_SCAVENGER )
             && ch->in_room->contents != NULL && number_bits( 6 ) == 0 )
        {
            OBJ_DATA *obj;
            OBJ_DATA *obj_best;
            int max;

            max = 1;
            obj_best = 0;
            for ( obj = ch->in_room->contents; obj; obj = obj->next_content )
            {
                if ( CAN_WEAR( obj, ITEM_TAKE ) && can_loot( ch, obj )
                     && obj->cost > max && obj->cost > 0 )
                {
                    obj_best = obj;
                    max = obj->cost;
                }
            }

            if ( obj_best )
            {
                obj_from_room( obj_best );
                obj_to_char( obj_best, ch );
                act( "$n gets $p.", ch, obj_best, NULL, TO_ROOM );
            }
        }

#ifdef AUTO_HATE
        /*Hate */
        {
            CHAR_DATA *rch;

            for ( rch = ch->in_room->people; rch; rch = rch->next_in_room )
            {

                if ( is_hating( ch, rch )
                     && ch->pIndexData->vnum != MOB_VNUM_SUPERMOB 
                     && ( can_see ( ch, rch ) )
                     && ( ch != rch ) )
                {
                    do_say( ch,
                            "Do you think you could just waltz back in here?\n\r" );
                    do_say( ch, "`RDie SCUM!`w" );
                    multi_hit( ch, rch, TYPE_UNDEFINED );
                    break;
                }
            }
        }
#endif
        /* Wander */
        if ( !IS_SET( ch->act, ACT_SENTINEL )
             && number_bits( 4 ) == 0
             && ( door = number_bits( 5 ) ) <= 5
             && ( pexit = ch->in_room->exit[door] ) != NULL
             && pexit->u1.to_room != NULL
             && !IS_SET( pexit->exit_info, EX_CLOSED )
             && !IS_SET( pexit->u1.to_room->room_flags, ROOM_NO_MOB )
             && ( !IS_SET( ch->act, ACT_STAY_AREA )
                  || pexit->u1.to_room->area == ch->in_room->area ) )
        {
            move_char( ch, door, FALSE );
            if ( ch->position < POS_STANDING )
                continue;
        }
    }

    return;
}

/*
 * Update the weather.
 */
void weather_update( void )
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;
    int diff;

    buf[0] = '\0';

    switch ( ++time_info.hour )
    {
    case 5:
        weather_info.sunlight = SUN_LIGHT;
        strcat( buf, "The day has begun.\n\r" );
        break;

    case 6:
        weather_info.sunlight = SUN_RISE;
        strcat( buf, "The sun rises in the east.\n\r" );
        break;

    case 19:
        weather_info.sunlight = SUN_SET;
        strcat( buf, "The sun slowly disappears in the west.\n\r" );
        break;

    case 20:
        weather_info.sunlight = SUN_DARK;
        strcat( buf, "The night has begun.\n\r" );
        break;

    case 24:
        time_info.hour = 0;
        time_info.day++;
        break;
    }

    if ( time_info.day >= 35 )
    {
        time_info.day = 0;
        time_info.month++;
    }

    if ( time_info.month >= 17 )
    {
        time_info.month = 0;
        time_info.year++;
    }

    /*
     * Weather change.
     */
    if ( time_info.month >= 9 && time_info.month <= 16 )
        diff = weather_info.mmhg > 985 ? -2 : 2;
    else
        diff = weather_info.mmhg > 1015 ? -2 : 2;

    weather_info.change += diff * dice( 1, 4 ) + dice( 2, 6 ) - dice( 2, 6 );
    weather_info.change = UMAX( weather_info.change, -12 );
    weather_info.change = UMIN( weather_info.change, 12 );

    weather_info.mmhg += weather_info.change;
    weather_info.mmhg = UMAX( weather_info.mmhg, 960 );
    weather_info.mmhg = UMIN( weather_info.mmhg, 1040 );

    switch ( weather_info.sky )
    {
    default:
        bug( "Weather_update: bad sky %d.", weather_info.sky );
        weather_info.sky = SKY_CLOUDLESS;
        break;

    case SKY_CLOUDLESS:
        if ( weather_info.mmhg < 990
             || ( weather_info.mmhg < 1010 && number_bits( 2 ) == 0 ) )
        {
            strcat( buf, "The sky is getting cloudy.\n\r" );
            weather_info.sky = SKY_CLOUDY;
        }
        break;

    case SKY_CLOUDY:
        if ( weather_info.mmhg < 970
             || ( weather_info.mmhg < 990 && number_bits( 2 ) == 0 ) )
        {
            strcat( buf, "It starts to rain.\n\r" );
            weather_info.sky = SKY_RAINING;
        }

        if ( weather_info.mmhg > 1030 && number_bits( 2 ) == 0 )
        {
            strcat( buf, "The clouds disappear.\n\r" );
            weather_info.sky = SKY_CLOUDLESS;
        }
        break;

    case SKY_RAINING:
        if ( weather_info.mmhg < 970 && number_bits( 2 ) == 0 )
        {
            strcat( buf, "Lightning flashes in the sky.\n\r" );
            weather_info.sky = SKY_LIGHTNING;
        }

        if ( weather_info.mmhg > 1030
             || ( weather_info.mmhg > 1010 && number_bits( 2 ) == 0 ) )
        {
            strcat( buf, "The rain stopped.\n\r" );
            weather_info.sky = SKY_CLOUDY;
        }
        break;

    case SKY_LIGHTNING:
        if ( weather_info.mmhg > 1010
             || ( weather_info.mmhg > 990 && number_bits( 2 ) == 0 ) )
        {
            strcat( buf, "The lightning has stopped.\n\r" );
            weather_info.sky = SKY_RAINING;
            break;
        }
        break;
    }

    if ( buf[0] != '\0' )
    {
        for ( d = descriptor_list; d != NULL; d = d->next )
        {
            if ( d->connected == CON_PLAYING && IS_OUTSIDE( d->character )
#ifdef NO_OLC_WEATHER
                 && d->pString == NULL
#endif
                 && IS_AWAKE( d->character ) )
                send_to_char( buf, d->character );
        }
    }

    return;
}

/*
 * Update all chars, including mobs.
*/
void char_update( void )
{
    CHAR_DATA *ch;
    CHAR_DATA *ch_next;
    CHAR_DATA *ch_quit;
    char buf[MAX_STRING_LENGTH];
    extern bool chaos;
    int blarg;
    ch_quit = NULL;

    /* update save counter */
    save_number++;

    if ( save_number > 30 )
        save_number = 0;
    for ( ch = char_list; ch != NULL; ch = ch_next )
    {
        AFFECT_DATA *paf;
        AFFECT_DATA *paf_next;
        NEWAFFECT_DATA *npaf;
        NEWAFFECT_DATA *npaf_next;

        ch_next = ch->next;

        if ( IS_SET( ch->act, PLR_JAILED ) && ch->jail_timer == 1 )
        {
            if ( IS_NPC( ch ) )
            {
                blarg = ch->was_in_room->vnum;
            }
            else
            {
                if ( JAIL_RELEASE_RECALL == 1 )
                    blarg = ch->pcdata->recall_room->vnum;
                else
                    blarg = JAIL_RELEASE_VNUM;
            }

            if ( JAIL_NOSHOUT == 1 )
            {
                REMOVE_BIT( ch->comm, COMM_NOSHOUT );
            }

            if ( JAIL_NOEMOTE == 1 )
            {
                REMOVE_BIT( ch->comm, COMM_NOEMOTE );
            }

            if ( JAIL_NOTELL == 1 )
            {
                REMOVE_BIT( ch->comm, COMM_NOTELL );
            }

            if ( JAIL_NOCHANNEL == 1 )
            {
                REMOVE_BIT( ch->comm, COMM_NOCHANNELS );
            }

            REMOVE_BIT( ch->act, PLR_JAILED );
            send_to_char( "`WYour `Rjail term`W has been `clifted\n\r", ch );

            if ( !IS_NPC( ch ) && ( JAIL_RELEASE_RECALL == 1 ) )
            {
                send_to_char
                    ( "`Wand you are being teleported back to `Grecall`W.`w\n",
                      ch );
                char_from_room( ch );
                ch->jail_timer = 0;
                char_to_room( ch, get_room_index( blarg ) );
                if ( IS_NPC( ch ) )
                    sprintf( buf,
                             "%s has served their time and has been freed jail.",
                             ch->short_descr );
                else
                    sprintf( buf,
                             "%s has served their time and has been freed jail.",
                             ch->name );
                do_sendinfo( ch, buf );
            }
            else
            {
                send_to_char
                    ( "`Wand you are being teleported back to your room.\n",
                      ch );
                char_from_room( ch );
                ch->jail_timer = 0;
                char_to_room( ch, get_room_index( blarg ) );
                sprintf( buf,
                         "%s has served their time and has been freed jail.",
                         ch->short_descr );
                do_sendinfo( ch, buf );

            }
        }

        if ( IS_SET( ch->act, PLR_JAILED ) && ch->jail_timer > 0 )
        {
            --ch->jail_timer;
        }

        /* Check to see if the player wants to see "ticks" or not -Lancelight */
        if ( !IS_NPC( ch ) && ch->pcdata->ticks == 0 )
        {
            if ( !IS_NPC( ch ) && ch->pcdata->tick == 1 && ch->desc->editor == 0
                 && ch->desc->pString == NULL && ch->desc->connected == 0 )
            {
                send_to_char( "\n\r", ch );
            }
        }

        if ( !IS_NPC( ch ) )
        {
            rprog_random_trigger( ch );

            if ( ch->position == POS_DEAD )
                continue;
        }

        if ( ( ch->timer > 30 ) && ch->level < LEVEL_IMMORTAL )
            ch_quit = ch;
        if ( ch->position >= POS_STUNNED )
        {
            if ( ch->hit < ch->max_hit )
                ch->hit += hit_gain( ch );
            else
                ch->hit = ch->max_hit;

            if ( ch->mana < ch->max_mana )
                ch->mana += mana_gain( ch );
            else
                ch->mana = ch->max_mana;

            if ( ch->move < ch->max_move )
                ch->move += move_gain( ch );
            else
                ch->move = ch->max_move;
        }

        if ( ch->position == POS_STUNNED )
            update_pos( ch );

        if ( !IS_NPC( ch ) )
        {
            OBJ_DATA *obj;

            if ( ( obj = get_eq_char( ch, WEAR_LIGHT ) ) != NULL
                 && obj->item_type == ITEM_LIGHT && obj->value[2] > 0 )
            {
                if ( --obj->value[2] == 0 && ch->in_room != NULL )
                {
                    --ch->in_room->light;
                    act( "$p goes out.", ch, obj, NULL, TO_ROOM );
                    act( "$p flickers and goes out.", ch, obj, NULL, TO_CHAR );
                    extract_obj( obj );
                }
                else if ( obj->value[2] <= 5 && ch->in_room != NULL )
                    act( "$p flickers.", ch, obj, NULL, TO_CHAR );
            }

            ++ch->timer;
            if ( IS_SET( ch->act, PLR_JAILED ) )
            {
                --ch->jail_timer;
            }
            if ( ch->timer >= 12 && ch->level < LEVEL_IMMORTAL )
            {
                if ( ch->was_in_room == NULL && ch->in_room != NULL )
                {
                    ch->was_in_room = ch->in_room;
                    if ( ch->fighting != NULL )
                        stop_fighting( ch, TRUE );
                    act( "$n disappears into the void.",
                         ch, NULL, NULL, TO_ROOM );
                    send_to_char( "You disappear into the void.\n\r", ch );
                    if ( ch->level > 1 )
                        save_char_obj( ch );
                    char_from_room( ch );
                    char_to_room( ch, get_room_index( ROOM_VNUM_LIMBO ) );
                }
            }
            if ( ch->level < LEVEL_IMMORTAL )
            {
                gain_condition( ch, COND_DRUNK, -1 * time_info.hour % 2 );
                gain_condition( ch, COND_FULL, -1 * time_info.hour % 2 );
                gain_condition( ch, COND_THIRST, -1 * time_info.hour % 2 );
            }
        }

        for ( paf = ch->affected; paf != NULL; paf = paf_next )
        {
            paf_next = paf->next;
            if ( paf->duration > 0 )
            {
                paf->duration--;
                if ( number_range( 0, 4 ) == 0 && paf->level > 0 )
                    paf->level--;   /* spell strength fades with time */
            }
            else if ( paf->duration < 0 )
                ;
            else
            {
                if ( paf_next == NULL
                     || paf_next->type != paf->type || paf_next->duration > 0 )
                {
                    if ( paf->type > 0 && skill_table[paf->type].msg_off )
                    {
                        send_to_char( skill_table[paf->type].msg_off, ch );
                        send_to_char( "\n\r", ch );
                    }
                }

                affect_remove( ch, paf );
            }
        }
        for ( npaf = ch->newaffected; npaf != NULL; npaf = npaf_next )
        {
            npaf_next = npaf->next;
            if ( npaf->duration > 0 )
            {
                npaf->duration--;
                if ( number_range( 0, 4 ) == 0 && npaf->level > 0 )
                    npaf->level--;  /* spell strength fades with time */
            }
            else if ( npaf->duration < 0 )
                ;
            else
            {
                if ( npaf_next == NULL
                     || npaf_next->type != npaf->type
                     || npaf_next->duration > 0 )
                {
                    if ( npaf->type > 0 && skill_table[npaf->type].msg_off )
                    {
                        send_to_char( skill_table[npaf->type].msg_off, ch );
                        send_to_char( "\n\r", ch );
                    }
                }

                newaffect_remove( ch, npaf );
            }
        }

        /*
         * Careful with the damages here,
         *   MUST NOT refer to ch after damage taken,
         *   as it may be lethal damage (on NPC).
         */

        if ( is_affected( ch, gsn_plague ) && ch != NULL )
        {
            AFFECT_DATA *af, plague;
            CHAR_DATA *vch;
            int save, dam;

            if ( ch->in_room == NULL )
                return;

            act( "$n writhes in agony as plague sores erupt from $s skin.",
                 ch, NULL, NULL, TO_ROOM );
            send_to_char( "You writhe in agony from the plague.\n\r", ch );
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

            for ( vch = ch->in_room->people; vch != NULL;
                  vch = vch->next_in_room )
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

                if ( save != 0 && !saves_spell( save, vch )
                     && !IS_IMMORTAL( vch ) && !IS_AFFECTED( vch, AFF_PLAGUE )
                     && number_bits( 4 ) == 0 )
                {
                    send_to_char( "You feel hot and feverish.\n\r", vch );
                    act( "$n shivers and looks very ill.", vch, NULL, NULL,
                         TO_ROOM );
                    affect_join( vch, &plague );
                }
            }

            dam = UMIN( ch->level, 5 );
            ch->mana -= dam;
            ch->move -= dam;
            damage( ch, ch, NULL, dam, gsn_plague, DAM_DISEASE );
        }
        else if ( IS_AFFECTED( ch, AFF_POISON ) && ch != NULL )
        {
            act( "$n shivers and suffers.", ch, NULL, NULL, TO_ROOM );
            send_to_char( "You shiver and suffer.\n\r", ch );
            damage( ch, ch, NULL, 2, gsn_poison, DAM_POISON );
        }
        else if ( ch->position == POS_INCAP && number_range( 0, 1 ) == 0 )
        {
            damage( ch, ch, NULL, 1, TYPE_UNDEFINED, DAM_NONE );
        }
        else if ( ch->position == POS_MORTAL )
        {
            damage( ch, ch, NULL, 1, TYPE_UNDEFINED, DAM_NONE );
        }
    }

    /*
     * Autosave and autoquit.
     * Check that these chars still exist.
     */
    for ( ch = player_list; ch != NULL; ch = ch_next )
    {
        ch_next = ch->next_player;

        if ( ch->desc != NULL && save_number == 30 && !chaos )
            save_char_obj( ch );

        if ( ch == ch_quit )
            do_quit( ch, "" );
    }

    return;
}

void regen_update( void )
{
    CHAR_DATA *ch;
    CHAR_DATA *ch_next;

    for ( ch = char_list; ch != NULL; ch = ch_next )
    {
        ch_next = ch->next;

        if ( ch->position >= POS_STUNNED
             && IS_AFFECTED( ch, AFF_REGENERATION ) )
        {
            if ( ch->hit < ch->max_hit )
                ch->hit += hit_gain( ch );
            else
                ch->hit = ch->max_hit;
        }
    }
    return;
}

/*
 * Update all objs.
 * This function is performance sensitive.
 */
void obj_update( void )
{
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    AFFECT_DATA *paf, *paf_next;

    for ( obj = object_list; obj != NULL; obj = obj_next )
    {
        CHAR_DATA *rch;
        char *message;

        obj_next = obj->next;

        if ( obj->carried_by
             || ( obj->in_room && obj->in_room->area->nplayer > 0 ) )
        {
            oprog_random_trigger( obj );
            if ( !obj )
                continue;
        }

        /* go through affects and decrement */
        for ( paf = obj->affected; paf != NULL; paf = paf_next )
        {
            paf_next = paf->next;
            if ( paf->duration > 0 )
            {
                paf->duration--;
                if ( number_range( 0, 4 ) == 0 && paf->level > 0 )
                    paf->level--;   /* spell strength fades with time */
            }
            else if ( paf->duration < 0 )
                ;
            else
            {
                if ( paf_next == NULL
                     || paf_next->type != paf->type || paf_next->duration > 0 )
                {
                    if ( paf->type > 0 && skill_table[paf->type].msg_off )
                    {
                        act_new( skill_table[paf->type].msg_off,
                                 obj->carried_by, obj, NULL, POS_SLEEPING,
                                 TO_CHAR );
                    }
                }

                affect_remove_obj( obj, paf );
            }
        }

        if ( obj->timer <= 0 || --obj->timer > 0 )
            continue;

        switch ( obj->item_type )
        {
        default:
            message = "$p crumbles into dust.";
            break;
        case ITEM_FOUNTAIN:
            message = "$p dries up.";
            break;
        case ITEM_CORPSE_NPC:
            message = "$p decays into dust.";
            break;
        case ITEM_CORPSE_PC:
            message = "$p decays into dust.";
            break;
        case ITEM_FOOD:
            message = "$p decomposes.";
            break;
        case ITEM_POTION:
            message = "$p has evaporated from disuse.";
        case ITEM_PORTAL:
            message = "$p flickers momentarily before fading away.";
            break;
        }

        if ( obj->carried_by != NULL )
        {
            if ( IS_NPC( obj->carried_by )
                 && obj->carried_by->pIndexData->pShop != NULL )
                obj->carried_by->gold += obj->cost / 5;
            else
                act( message, obj->carried_by, obj, NULL, TO_CHAR );
        }
        else if ( obj->in_room != NULL
                  && ( rch = obj->in_room->people ) != NULL )
        {
            if ( !( obj->in_obj && obj->in_obj->pIndexData->vnum == OBJ_VNUM_PIT
                    && !CAN_WEAR( obj->in_obj, ITEM_TAKE ) ) )
            {
                act( message, rch, obj, NULL, TO_ROOM );
                act( message, rch, obj, NULL, TO_CHAR );
            }
        }

        if ( obj->item_type == ITEM_CORPSE_PC && obj->contains )
        {                       /* save the contents */
            OBJ_DATA *t_obj, *next_obj;

            for ( t_obj = obj->contains; t_obj != NULL; t_obj = next_obj )
            {
                next_obj = t_obj->next_content;
                obj_from_obj( t_obj );

                if ( obj->in_obj )  /* in another object */
                    obj_to_obj( t_obj, obj->in_obj );

                if ( obj->carried_by )  /* carried */
                    obj_to_char( t_obj, obj->carried_by );

                if ( obj->in_room == NULL ) /* destroy it */
                    extract_obj( t_obj );

                else            /* to a room */
                    obj_to_room( t_obj, obj->in_room );
            }
        }

        extract_obj( obj );
    }

    return;
}

/*
 * Aggress.
 *
 * for each mortal PC
 *     for each mob in room
 *         aggress on some random PC
 *
 * This function takes 25% to 35% of ALL Merc cpu time.
 * Unfortunately, checking on each PC move is too tricky,
 *   because we don't the mob to just attack the first PC
 *   who leads the party into the room.
 *
 * -- Furey
 */

/*
 * Changed this a bit.  Now instead of using char_list it uses player_list which is solely a list of
 * players that are logged on.  MUCH faster than using char_list since we don't have to pick a few
 * players out of thousands of mobs.
 *
 * -Zane
 */
void aggr_update( void )
{
    CHAR_DATA *wch;
    CHAR_DATA *wch_next;
    CHAR_DATA *ch;
    CHAR_DATA *ch_next;
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    CHAR_DATA *victim;
    int count, random;

    for ( wch = player_list; wch; wch = wch_next )
    {
        wch_next = wch->next_player;

        /* FIXME!  Remove this code alltogether.  Having delayed act-progs is a problem.
         * What is someone triggers an act prog then runs out of the room?  The TriggeredBy is then
         * set and the mob/obj/room will try to act like that player is still in the room.  If you want
         * delayed triggers then maybe we should get a command queue system put in. - Zane */
/*	if ( IS_NPC( wch ) && wch->mpactnum > 0
	    && wch->in_room->area->nplayer > 0 )
	{

	    MPROG_ACT_LIST * tmp_act, *tmp2_act;
	    for ( tmp_act = wch->mpact; tmp_act != NULL;
		 tmp_act = tmp_act->next )
	    {
		 mprog_wordlist_check( tmp_act->buf,wch, tmp_act->ch,
				      tmp_act->obj, tmp_act->vo, ACT_PROG, MOB_PROG );
		 free_string( &tmp_act->buf );
	    }
	    for ( tmp_act = wch->mpact; tmp_act != NULL; tmp_act = 
tmp2_act )
	    {
		 tmp2_act = tmp_act->next;
		 free_mem( &tmp_act );
	    }
	    wch->mpactnum = 0;
	    wch->mpact    = NULL; 
	}*/

        if ( wch->level >= LEVEL_IMMORTAL || !wch->in_room
/*	||   wch->in_room->area->empty *//* How can the area be in it if this player is in that area? - Zane */
             || IS_SET( wch->in_room->room_flags, ROOM_SAFE ) )
            continue;

        for ( ch = wch->in_room->people; ch != NULL; ch = ch_next )
        {
            ch_next = ch->next_in_room;

            if ( !IS_NPC( ch )
                 || !IS_SET( ch->act, ACT_AGGRESSIVE )
                 || ch->fighting
                 || IS_AFFECTED( ch, AFF_CALM )
                 || IS_AFFECTED( ch, AFF_CHARM )
                 || !IS_AWAKE( ch )
                 || ( IS_SET( ch->act, ACT_WIMPY ) && IS_AWAKE( wch ) )
                 || !can_see( ch, wch ) || number_bits( 1 ) == 0 )
                continue;

            /*
             * Ok we have a 'wch' player character and a 'ch' npc aggressor.
             * Now make the aggressor fight a RANDOM pc victim in the room,
             *   giving each 'vch' an equal chance of selection.
             */
            count = 0;

            for ( vch = wch->in_room->people; vch != NULL; vch = vch->next )
            {
                if ( !IS_NPC( vch )
                     && vch->level < LEVEL_IMMORTAL
                     && ch->level >= vch->level - 5
                     && ( !IS_SET( ch->act, ACT_WIMPY ) || !IS_AWAKE( vch ) )
                     && can_see( ch, vch ) )
                    count++;
            }

            random = number_range( 1, count );
            count = 0;

            victim = NULL;
            for ( vch = wch->in_room->people; vch != NULL; vch = vch_next )
            {
                vch_next = vch->next_in_room;

                if ( !IS_NPC( vch )
                     && vch->level < LEVEL_IMMORTAL
                     && ch->level >= vch->level - 5
                     && ( !IS_SET( ch->act, ACT_WIMPY ) || !IS_AWAKE( vch ) )
                     && can_see( ch, vch ) && ++count == random )
                {
                    victim = vch;
                    break;
                }
            }

            if ( victim )
                multi_hit( ch, victim, TYPE_UNDEFINED );
        }
    }

    return;
}

/*
 * Handle all kinds of updates.
 * Called once per pulse from game loop.
 * Random times to defeat tick-timing clients and players.
 */

void update_handler( void )
{
    static int pulse_area;
    static int pulse_mobile;
    static int pulse_violence;
    static int pulse_point;
    static int pulse_auction;
    extern bool silentmode;

    if ( silentmode )
    {
        bug( "Silentmode = TRUE in update.c", 0 );
        silentmode = FALSE;
    }

    if ( --pulse_area <= 0 )
    {
        pulse_area = number_range( PULSE_AREA / 2, 3 * PULSE_AREA / 2 );
        update_last( "Update:", "area", "" );
        area_update(  );
    }
/* Lets update the auction channel. -Lancelight */
    if ( --pulse_auction <= 0 )
    {
        pulse_auction = PULSE_AUCTION;
        auction_update(  );
    }

    if ( --pulse_mobile <= 0 )
    {
        pulse_mobile = PULSE_MOBILE;
        update_last( "Update:", "mobile", "" );
        mobile_update(  );
    }

    if ( --pulse_violence <= 0 )
    {
        pulse_violence = PULSE_VIOLENCE;
        update_last( "Update:", "violence", "" );
        violence_update(  );
    }

    if ( --pulse_point <= 0 )
    {
#if defined(DEBUG)
        sprintf( buf, "TICK!\r" );
        log_string( buf );
#endif
        pulse_point = PULSE_TICK;

        /* number_range( PULSE_TICK / 2, 3 * PULSE_TICK / 2 ); */

        update_last( "Update:", "area", "" );
        weather_update(  );
        update_last( "Update:", "char", "" );
        char_update(  );
        update_last( "Update:", "obj", "" );
        obj_update(  );
    }
    else if ( pulse_point == PULSE_TICK / 2 )
    {
        update_last( "Update:", "regen", "" );
        regen_update(  );
    }

    update_last( "Update:", "aggr", "" );
    aggr_update(  );
    update_last( "End update loop.", "", "" );
    tail_chain(  );
    return;
}
