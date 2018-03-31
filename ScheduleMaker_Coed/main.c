#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/time.h>
#include <string.h>

#define NUM_PLAYERS 10
#define NUM_GAMES 15
#define NUM_WEEKS 9
#define MAX_ATTEMPTS 200
#define MAX_SCHEDULE_ATTEMPTS 20000
#define MAX_REMOVED_TEAMS 1000
#define MAX_REPEATS 3
#define MAX_GAMES 30

#define GUY 0
#define GIRL 1

typedef struct _coedteams{
    uint8_t player;
    uint8_t partner[10];
    uint8_t game_counter;
} COEDTEAMS, *PCOEDTEAMS;

typedef struct _game{
    uint8_t girlA;
    uint8_t guyA;
    uint8_t girlB;
    uint8_t guyB;
} GAME, *PGAME;

typedef struct _schedule{
    uint8_t week;
    GAME games[MAX_GAMES];
} SCHEDULE, *PSCHEDULE;

SCHEDULE schedule[NUM_WEEKS];
COEDTEAMS girls[NUM_PLAYERS];
COEDTEAMS guys[NUM_PLAYERS];
uint8_t remove_teams[NUM_PLAYERS * NUM_PLAYERS];
uint8_t all_teams[NUM_PLAYERS * NUM_PLAYERS];
uint8_t all_teams_count[NUM_PLAYERS * NUM_PLAYERS];
uint8_t all_teams_removed[NUM_PLAYERS * NUM_PLAYERS] = {0};
uint8_t team_list[30] = {0};
uint8_t g_num_repeat_partners = 0;
uint32_t g_count;
uint16_t g_team_index = 0;
uint16_t g_total_index;
uint8_t g_week_counter = 1;
uint16_t g_removed_teams = 0;

uint8_t removed_teams[MAX_REMOVED_TEAMS] = {0};
/*{
    //Week 1
    0x16,0x27,0x38,0x49,0x5a,0x47,0x81,0x65,0x98,0x24,
    0x23,0x41,0xa9,0xa5,0x55,0x91,0x72,0x83,0x94,0xa3,
    0x3a,0x79,0x87,0x66,0x12,0x18,0x32,0x54,0x6a,0x76,
    //Week 2
    0x28,0x31,0x69,0x95,0x45,0x11,0x7a,0x39,0x52,0x22,
    0x74,0x77,0xa4,0x26,0x1a,0x97,0x93,0x58,0xa1,0x86,
    0x67,0x59,0x63,0x84,0x13,0x48,0xaa,0x82,0x35,0x46,
    //Week 3
    0x42,0x51,0x78,0x4a,0x43,0x62,0x99,0x34,0x36,0x53,
    0xa2,0x9a,0x88,0x8a,0x19,0x57,0x85,0x14,0x75,0x17,
    0x29,0xa6,0x71,0x68,0x96,0x64,0x21,0xa7,0x25,0x33,
    //Week 4
    0x61,0x92,0x18,0x15,0x12,0x5a,0x66,0x72,0x37,0x44,
    0x38,0x27,0x94,0x87,0x23,0x56,0x65,0x81,0x79,0x41,
    0x83,0xa3,0x98,0x54,0x76,0x2a,0xa9,0xa5,0x3a,0x49,
    //Extra Games
};*/

uint8_t create_coed_week_schedule(void);
uint8_t select_teams(uint8_t *teamA, uint8_t *teamB, uint8_t *indexA, uint8_t *indexB);
uint8_t create_team(uint8_t player, uint8_t partner);
void update_games(uint8_t teamA, uint8_t teamB);
uint8_t check_game(uint8_t teamA, uint8_t teamB);
void add_team_list_to_schedule(uint8_t week);
void add_team_list_to_removed_teams(void);
void fill_teams(void);
void fill_removed_teams(void);
uint16_t check_game_count(void);
uint8_t check_schedule(void);
void clear_team_list(void);
void clear_schedule(void);
void clear_count(void);
void remove_player(uint8_t sex, uint8_t player);
void remove_games(void);
void remove_team(uint8_t team, uint8_t flag);
uint16_t rand_interval(uint16_t min, uint16_t max);

void DEBUG_delay(void);

int main (int argc, char *argv[])
{
    uint8_t flag = 1;
    uint8_t i = 0;
    uint8_t girl, guy;
    uint8_t games = 0;
    uint16_t game_count = 0;
    FILE* fp;
    g_num_repeat_partners = 3;

    fp = fopen("schedule.txt", "w");

    fill_removed_teams();

    while(g_week_counter <= NUM_WEEKS)
    {
        while(flag)
        {
            fill_teams();
            remove_games();

            if(create_coed_week_schedule())
            {
                clear_schedule();
                clear_team_list();
                game_count = 0;
                game_count = check_game_count();
            }
            else
                flag = 0;

            if(g_count > MAX_SCHEDULE_ATTEMPTS)
            {
                printf("\r\nMAX_ATTEMPTS REACHED\r\n");
                return 0;
            }
            g_count++;
        }

        fprintf(fp, "****** Week %i ******\r\n", g_week_counter);
        printf("****** Week %i ******\r\n", g_week_counter);

        while(games < NUM_GAMES)
        {
            girl = team_list[i] >> 4;
            guy = team_list[i] & 0x0F;
            fprintf(fp,"%i\t%i", girl, guy);
            printf("%i\t%i", girl, guy);

            i++;
            fprintf(fp,"\tvs\t");
            printf("\tvs\t");

            girl = team_list[i] >> 4;
            guy = team_list[i] & 0x0F;
            fprintf(fp,"%i\t%i", girl, guy);
            printf("%i\t%i", girl, guy);

            fprintf(fp,"\r\n");
            printf("\r\n");

            games++;
            i++;
        }

        flag = 1;
        games = 0;
        i = 0;
        g_count = 0;

        fprintf(fp, "\r\n");
        add_team_list_to_removed_teams();
        add_team_list_to_schedule(g_week_counter);
        clear_schedule();
        clear_team_list();
        game_count = 0;
        game_count = check_game_count();
        g_week_counter++;
    }

    flag = check_schedule();
    fclose(fp);
    return 0;
}

uint8_t create_coed_week_schedule(void)
{
    uint8_t teamA, teamB;
    uint8_t indexA, indexB;
    uint8_t game = 0;

    while(game < NUM_GAMES)
    {
        //Random Game
        if(select_teams(&teamA, &teamB, &indexA, &indexB))
        {
            //printf("\r                                                     \r");
            //printf("ATTEMPT: %i", g_count);
            //DEBUG_delay();

            return 1;
        }
        //Checks
        if(check_game(teamA, teamB))
        {
            //Add to Team List
            team_list[g_team_index++] = teamA;
            team_list[g_team_index++] = teamB;
            //Mark Teams Used
            remove_team(teamA, 0);
            remove_team(teamB, 0);

            update_games(teamA,teamB);

            game++;
            //printf("\r                                                     ");
            //printf("\r\nGame %i Selected!", game);
        }
    }
    return 0;
}

uint8_t select_teams(uint8_t *teamA, uint8_t *teamB, uint8_t *indexA, uint8_t *indexB)
{
    uint8_t flag;
    uint8_t girlA, guyA, girlB, guyB;
    uint32_t counter = 0;

    while(1)
    {
        //Got into a bad spot
        if(counter > MAX_ATTEMPTS)
        {
            counter = 0;
            return 1;
        }
        counter++;

        flag = 0;
        *indexA = rand_interval(0, g_total_index - 1);
        *teamA = all_teams[*indexA];

        //printf("\r                                                             \r");
        //printf("Attempt %i\tTeam A %x Team B %x", counter, *teamA, 0);

        girlA = *teamA >> 4;
        guyA = *teamA & 0x0F;

        //They've already played together
        if((girls[girlA - 1].partner[guyA] & 0x80) != 0)
        {
            flag = 1;
        }
        //They have enough games already
        if((girls[girlA - 1].game_counter > 2))
        {
            remove_player(GIRL,girlA);
            flag = 1;
        }
        if((guys[guyA - 1].game_counter > 2))
        {
            remove_player(GUY,guyA);
            flag = 1;
        }

        //Passed First 2 Checks, Pick Team B
        if(!flag)
        {
            counter = 0;
            while(1)
            {
                //Got into a bad spot
                if(counter > MAX_ATTEMPTS)
                {
                    counter = 0;
                    return 1;
                }
                counter++;
                flag = 0;
                *indexB = rand_interval(0, g_total_index - 1);
                *teamB = all_teams[*indexB];

                //printf("\r                                                             \r");
                //printf("Attempt %i\tTeam A [%x] Team B %x", counter, *teamA, *teamB);

                girlB = *teamB >> 4;
                guyB = *teamB & 0x0F;

                //This is Team A
                if((girlA == girlB) || (guyA == guyB))
                {
                    flag = 1;
                }
                //They've already played together
                if((girls[girlB - 1].partner[guyB] & 0x80) != 0)
                {
                    flag = 1;
                }
                //They have enough games already
                if((girls[girlB - 1].game_counter > 2))
                {
                    remove_player(GIRL,girlB);
                    flag = 1;
                }
                if((guys[guyB - 1].game_counter > 2))
                {
                    remove_player(GUY,guyB);
                    flag = 1;
                }
                //Passed First Checks, Check Game
                if(!flag)
                {
                    //printf("\r                                                             \r");
                    //printf("Attempt %i\tTeam A [%x] Team B [%x]", counter, *teamA, *teamB);
                    break;
                }
            }
            break;
        }
    }
    return 0;
}

uint8_t create_team(uint8_t player, uint8_t partner)
{
    uint16_t coed_team = 0;

    coed_team = partner;
    coed_team |= player << 4;

    return coed_team;
}

void update_games(uint8_t teamA, uint8_t teamB)
{
    uint8_t girl_1, girl_2, guy_1, guy_2;
    uint8_t i;

    girl_1 = teamA >> 4;
    girl_2 = teamB >> 4;
    guy_1 = teamA & 0x0F;
    guy_2 = teamB & 0x0F;

    //Flag High Bit to Indicate Partner Been Used
    for(i=0; (i < NUM_PLAYERS - 1); i++)
    {
        if(girls[girl_1 - 1].partner[i] == guy_1)
        {
         girls[girl_1 - 1].partner[i] |= 0x80;
        }
        if(girls[girl_2 - 1].partner[i] == guy_2)
        {
            girls[girl_2 - 1].partner[i] |= 0x80;
        }
        if(guys[guy_1 - 1].partner[i] == girl_1)
        {
            guys[guy_1 - 1].partner[i] |= 0x80;
        }
        if(guys[girl_2 - 1].partner[i] == girl_2)
        {
            guys[guy_2 - 1].partner[i] |= 0x80;
        }
    }
    girls[girl_1 - 1].game_counter++;
    girls[girl_2 - 1].game_counter++;
    guys[guy_1 - 1].game_counter++;
    guys[guy_2 - 1].game_counter++;

    return;
}
uint8_t check_game(uint8_t teamA, uint8_t teamB)
{
    uint8_t girl_1, girl_2, guy_1, guy_2;

    girl_1 = teamA >> 4;
    girl_2 = teamB >> 4;
    guy_1 = teamA & 0x0F;
    guy_2 = teamB & 0x0F;

    //Guy Same
    if(guy_1 == guy_2)
    {
        return 0;
    }

    //Girl Same
    if(girl_1 == girl_2)
    {
        return 0;
    }

    return 1;
}

void add_team_list_to_schedule(uint8_t week)
{
    uint8_t i;
    uint8_t index = 0;

    schedule[week - 1].week = week;

    for(i=0; i < NUM_GAMES; i++)
    {
        schedule[week - 1].games[i].girlA = (team_list[index] >> 4);
        schedule[week - 1].games[i].guyA = (team_list[index] & 0x0F);
        index++;
        schedule[week - 1].games[i].girlB = (team_list[index] >> 4);
        schedule[week - 1].games[i].guyB = (team_list[index] & 0x0F);
        index++;
    }
}
void add_team_list_to_removed_teams(void)
{
    uint8_t i,j;
    uint16_t length = sizeof(all_teams_removed);

    for(i=0; i < sizeof(team_list); i++)
    {
        for(j=0; j < length; j++)
        {
            if(all_teams_removed[j] == team_list[i])
            {
                all_teams_count[j]++;
            }
        }
    }
}
void fill_teams(void)
{
    uint8_t i,j;
    uint16_t counter = 0;

    //Fill Teams
    for(i=0; i < NUM_PLAYERS; i++)
    {
        girls[i].player = i+1;
        guys[i].player = i+1;

        for(j=0; j < NUM_PLAYERS; j++)
        {
            girls[i].partner[j] = j+1;
            guys[i].partner[j] = j+1;
            all_teams[counter]= create_team(girls[i].player, girls[i].partner[j]);
            counter++;
        }
    }
    g_total_index = counter;
}

void fill_removed_teams(void)
{
    uint8_t i,j;
    uint16_t counter = 0;

    //Fill Teams
    for(i=0; i < NUM_PLAYERS; i++)
    {
        girls[i].player = i+1;
        guys[i].player = i+1;

        for(j=0; j < NUM_PLAYERS; j++)
        {
            girls[i].partner[j] = j+1;
            guys[i].partner[j] = j+1;
            all_teams_removed[counter]= create_team(girls[i].player, girls[i].partner[j]);
            counter++;
        }
    }
    g_total_index = counter;
}

uint8_t check_schedule(void)
{
  uint8_t i, j;
  uint8_t girlA,girlB, guyA, guyB;

  for(i=0; i < NUM_WEEKS; i++)
  {
      for(j=0; j < NUM_GAMES; j++)
      {
          girlA = schedule[i].games[j].girlA;
          guyA = schedule[i].games[j].guyA;
          girlB = schedule[i].games[j].girlB;
          guyB = schedule[i].games[j].guyB;

          girls[girlA - 1].partner[guyA - 1]++;
          girls[girlB - 1].partner[guyB - 1]++;
          guys[guyA - 1].partner[girlA - 1]++;
          guys[guyB - 1].partner[girlB - 1]++;
      }
  }
  return 1;
}
uint16_t check_game_count(void)
{
    uint16_t count = 0;
    uint8_t i;

    for(i=0; i < sizeof(all_teams_count); i++)
    {
        count += all_teams_count[i];
    }

    return count;
}
void clear_team_list(void)
{
    uint8_t i;

    for(i=0; i < sizeof(team_list); i++)
    {
        team_list[i] = 0;
    }
}
void clear_schedule(void)
{
    uint8_t i,j;

    g_total_index = 99;
    g_team_index = 0;

    for(i=0; i < sizeof(all_teams); i++)
    {
        all_teams[i] = 0;
    }

    //Fill Teams
    for(i=0; i < NUM_PLAYERS; i++)
    {
        girls[i].player = 0;
        guys[i].player = 0;
        girls[i].game_counter = 0;
        guys[i].game_counter = 0;

        for(j=0; j < (NUM_PLAYERS - 1); j++)
        {
            girls[i].partner[j] = 0;
            guys[i].partner[j] = 0;
        }
    }
}

void clear_count(void)
{
    uint8_t i;

    for(i=0; i < sizeof(all_teams_count); i++)
    {
        all_teams_count[i] = 0;
    }
}

uint8_t remove_player_exists(uint8_t sex, uint8_t player)
{
    uint16_t i;

    if(sex == GIRL)
    {
        for(i=0; i < g_total_index; i++)
        {
            if((all_teams[i] >> 4) == (player & 0x7F))
            {
                return 1;
            }
        }
    }
    else
    {
        for(i=0; i < g_total_index; i++)
        {
            if((all_teams[i] & 0x0F) == player)
            {
                return 1;
            }
        }
    }

    return 0;
}
void remove_player(uint8_t sex, uint8_t player)
{
    uint8_t flag = 1;

    //Set high bit if player is guy
    if(sex == GIRL)
    {
        player = player | 0x80;
    }
    while(flag)
    {
        remove_team(player,2);
        flag = remove_player_exists(sex, player);
    }
}

void remove_games(void)
{
    uint32_t i;

    g_removed_teams = 0;

    for(i=0; i < sizeof(all_teams_count); i++)
    {
        if(all_teams_count[i] > 2)
        {
            remove_team(all_teams_removed[i], 0);
            g_removed_teams++;
        }
    }

}
void remove_team(uint8_t team, uint8_t flag)
{
    uint8_t i;
    uint8_t fl = 0;
    uint8_t index;
    uint8_t player = 0;

    if(flag == 2)
    {
        player = team;
        for(i=0; i < g_total_index; i++)
        {
            //GIRL
            if(player & 0x80)
            {
                team = (all_teams[i] >> 4);
                if(team == (player & 0x7F))
                {
                    index = i;
                    all_teams[i] = 0;
                    fl = 1;
                }
            }
            //GUY
            else
            {
                team = all_teams[i] & 0x0F;
                if(team == player)
                {
                    index = i;
                    all_teams[i] = 0;
                    fl = 1;
                }
            }

            if(fl)
            {
                for(i=index; i < g_total_index; i++)
                {
                    all_teams[i] = all_teams[i+1];
                }
                g_total_index--;
                fl = 0;
            }
        }
    }
    else
    {
        for(i=0; i < sizeof(all_teams); i++)
        {
            if(all_teams[i] == team)
            {
                index = i;
                all_teams[i] = 0;
                for(i=index; i < g_total_index; i++)
                {
                    all_teams[i] = all_teams[i+1];
                }
                g_total_index--;
                break;
            }
        }
    }
}

uint16_t rand_interval(uint16_t min, uint16_t max)
{
    int16_t r;
    uint16_t range;
    uint16_t buckets;
    uint16_t limit;

    range = 1 + max - min;

    if(range == 0 || (range > RAND_MAX))
    {
        range = 1;
    }

    buckets = RAND_MAX / range;
    limit = buckets * range;

    /* Create equal size buckets all in a row, then fire randomly towards
     * the buckets until you land in one of them. All buckets are equally
     * likely. If you land off the end of the line of buckets, try again. */
    do
    {
        r = rand();
    } while (r >= limit);

    return min + (r / buckets);
}

void DEBUG_delay(void)
{
    uint32_t i,j;

    for(i=0; i < 2000; i++)
    {
        for(j=0; j < 100000; j++)
        {

        }
    }
}
