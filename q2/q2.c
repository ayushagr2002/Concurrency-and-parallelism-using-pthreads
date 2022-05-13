#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <semaphore.h>
#include <errno.h>

#define BLK "\e[0;30m"
#define RED "\e[0;31m"
#define GRN "\e[0;32m"
#define YEL "\e[0;33m"
#define BLU "\e[0;34m"
#define MAG "\e[0;35m"
#define CYN "\e[0;36m"
#define WHT "\e[0;37m"
#define reset "\e[0m"

int caph, capa, capn;
int X;
int num_grps;
int *grp;
int goalscoringchance;
int curcaph, curcapa, curcapn;

typedef struct
{
    char pname[50];
    char fanteam;
    int time;
    int patiencevalue;
    int goals;
    int zonealloc;
    int isinsystem;
    int hasarrived;
    int rage;
} stperson;

typedef struct
{
    char team;
    int telapsed;
    float prob;
} stchance;

typedef struct
{
    int grp;
    int index;
} gi;

int goalsscored[2] = {0, 0};

stperson **pdetails;
stchance *chdetails;
pthread_mutex_t **personlock;

struct timespec globalstart;

pthread_mutex_t stadiums_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t queue_lock = PTHREAD_MUTEX_INITIALIZER;
sem_t semhn, semahn;

pthread_cond_t **speccond;
pthread_cond_t **ragecond;

typedef struct
{
    int grp;
    int idx;
    char team;
} stqueuenode;
typedef struct queue
{
    int nmember;
    stqueuenode *person;
} stqueue;
stqueue waitq;

void *team_func(void *idx)
{
    int id = *((int *)idx);
    sleep(chdetails[id].telapsed);
    int goalscored = 0;
    if (chdetails[id].team == 'H')
    {
        int temp = (rand() % 100) + 1;
        if (temp / 100.00f < chdetails[id].prob)
        {
            goalsscored[0]++;
            goalscored = 1;
            printf(WHT "Team %c scored their %dth goal\n" reset, chdetails[id].team, goalsscored[0]);
            for (int i = 0; i < num_grps; i++)
            {
                for (int j = 0; j < grp[i]; j++)
                {
                    pthread_mutex_lock(&personlock[i][j]);
                    if (pdetails[i][j].isinsystem == 1 && pdetails[i][j].zonealloc != '0')
                    {
                        if (pdetails[i][j].fanteam == 'A')
                        {
                            if (goalsscored[0] >= pdetails[i][j].goals)
                            {
                                //printf("Sent rage condition to %s\n", pdetails[i][j].pname);
                                pdetails[i][j].rage = 1;
                                pthread_cond_signal(&ragecond[i][j]);
                            }
                        }
                    }
                    pthread_mutex_unlock(&personlock[i][j]);
                }
            }
        }
        else
        {
            printf(WHT "Team %c missed the chance to score their %dth goal\n" reset, chdetails[id].team, goalsscored[0] + 1);
        }
    }
    else
    {
        int temp = (rand() % 100) + 1;
        if (temp / 100.00f < chdetails[id].prob)
        {
            goalsscored[1]++;
            goalscored = 1;
            printf(WHT "Team %c scored their %dth goal\n" reset, chdetails[id].team, goalsscored[1]);
            for (int i = 0; i < num_grps; i++)
            {
                for (int j = 0; j < grp[i]; j++)
                {
                    pthread_mutex_lock(&personlock[i][j]);
                    if (pdetails[i][j].isinsystem == 1 && pdetails[i][j].zonealloc != '0')
                    {
                        if (pdetails[i][j].fanteam == 'H')
                        {
                            if (goalsscored[1] >= pdetails[i][j].goals)
                            {
                                pdetails[i][j].rage = 1;
                                pthread_cond_signal(&ragecond[i][j]);
                            }
                        }
                    }
                    pthread_mutex_unlock(&personlock[i][j]);
                }
            }
        }
        else
        {
            printf(WHT "Team %c missed the chance to score their %dth goal\n" reset, chdetails[id].team, goalsscored[1] + 1);
        }
    }
    return NULL;
}

void *person_func(void *temp)
{
    gi *info = ((gi *)temp);
    pthread_mutex_lock(&personlock[info->grp][info->index]);
    sleep(pdetails[info->grp][info->index].time);
    pdetails[info->grp][info->index].hasarrived = 1;
    pthread_mutex_unlock(&personlock[info->grp][info->index]);
    printf(RED "%s has reached the stadium\n" reset, pdetails[info->grp][info->index].pname);
    int s;
    struct timespec start;

    pthread_mutex_lock(&stadiums_lock);
    if (pdetails[info->grp][info->index].fanteam == 'A')
    {
        if (curcapa == 0)
        {
            pthread_mutex_lock(&queue_lock);
            waitq.nmember++;
            waitq.person[waitq.nmember - 1].grp = info->grp;
            waitq.person[waitq.nmember - 1].idx = info->index;
            waitq.person[waitq.nmember - 1].team = 'A';
            pthread_mutex_unlock(&queue_lock);
            clock_gettime(CLOCK_REALTIME, &start);
            start.tv_sec += pdetails[info->grp][info->index].patiencevalue;
            int rc = pthread_cond_timedwait(&speccond[info->grp][info->index], &stadiums_lock, &start);
            if (rc == ETIMEDOUT)
            {
                printf(MAG "%s couldn't get a seat\n" reset, pdetails[info->grp][info->index].pname);
                //printf(BLU "%s is leaving for dinner\n" reset, pdetails[info->grp][info->index].pname);
                pthread_mutex_unlock(&stadiums_lock);
                pthread_mutex_lock(&personlock[info->grp][info->index]);
                pdetails[info->grp][info->index].isinsystem = 0;
                pthread_mutex_unlock(&personlock[info->grp][info->index]);
                
                int waiting = 0;
                for (int i = 0; i < grp[info->grp]; i++)
                {
                    pthread_mutex_lock(&personlock[info->grp][i]);
                    if (pdetails[info->grp][i].isinsystem == 1)
                    {
                        printf(BLU "%s is waiting for their friends at the exit\n" reset, pdetails[info->grp][info->index].pname);
                        waiting = 1;
                        pthread_mutex_unlock(&personlock[info->grp][i]);
                        break;
                    }
                    pthread_mutex_unlock(&personlock[info->grp][i]);
                }
                if (waiting == 0)
                {
                    printf(BLU "Group %d is leaving for dinner\n" reset, info->grp+1);
                }
                return NULL;
            }
        }
        else
        {
            curcapa--;
            pdetails[info->grp][info->index].zonealloc = 'A';
        }
    }
    else if (pdetails[info->grp][info->index].fanteam == 'H')
    {
        if (curcaph == 0 && curcapn == 0)
        {
            pthread_mutex_lock(&queue_lock);
            waitq.nmember++;
            waitq.person[waitq.nmember - 1].grp = info->grp;
            waitq.person[waitq.nmember - 1].idx = info->index;
            waitq.person[waitq.nmember - 1].team = 'H';
            pthread_mutex_unlock(&queue_lock);
            clock_gettime(CLOCK_REALTIME, &start);
            start.tv_sec += pdetails[info->grp][info->index].patiencevalue;
            int rc = pthread_cond_timedwait(&speccond[info->grp][info->index], &stadiums_lock, &start);
            if (rc == ETIMEDOUT)
            {
                printf(MAG "%s couldn't get a seat\n" reset, pdetails[info->grp][info->index].pname);
                //printf(BLU "%s is leaving for dinner\n" reset ,pdetails[info->grp][info->index].pname);
                //pdetails[info->grp][info->index].isinsystem = 0;
                pthread_mutex_unlock(&stadiums_lock);
                pthread_mutex_lock(&personlock[info->grp][info->index]);
                pdetails[info->grp][info->index].isinsystem = 0;
                pthread_mutex_unlock(&personlock[info->grp][info->index]);
                
                int waiting = 0;
                for (int i = 0; i < grp[info->grp]; i++)
                {
                    pthread_mutex_lock(&personlock[info->grp][i]);
                    if (pdetails[info->grp][i].isinsystem == 1)
                    {
                        printf(BLU "%s is waiting for their friends at the exit\n" reset, pdetails[info->grp][info->index].pname);
                        waiting = 1;
                        pthread_mutex_unlock(&personlock[info->grp][i]);
                        break;
                    }
                    pthread_mutex_unlock(&personlock[info->grp][i]);
                }
                if (waiting == 0)
                {
                    printf(BLU "Group %d is leaving for dinner\n" reset, info->grp+1);
                }
                return NULL;
            }
        }
        else
        {
            if (curcaph > 0)
            {
                curcaph--;
                pdetails[info->grp][info->index].zonealloc = 'H';
            }
            else
            {
                curcapn--;
                pdetails[info->grp][info->index].zonealloc = 'N';
            }
        }
    }
    else
    {
        if (curcaph == 0 && curcapn == 0 && curcapa == 0)
        {
            pthread_mutex_lock(&queue_lock);
            waitq.nmember++;
            waitq.person[waitq.nmember - 1].grp = info->grp;
            waitq.person[waitq.nmember - 1].idx = info->index;
            waitq.person[waitq.nmember - 1].team = 'N';
            pthread_mutex_unlock(&queue_lock);
            clock_gettime(CLOCK_REALTIME, &start);
            start.tv_sec += pdetails[info->grp][info->index].patiencevalue;
            int rc = pthread_cond_timedwait(&speccond[info->grp][info->index], &stadiums_lock, &start);
            if (rc == ETIMEDOUT)
            {
                printf(MAG "%s couldn't get a seat\n" reset, pdetails[info->grp][info->index].pname);
                //printf(BLU "%s is leaving for dinner\n" reset, pdetails[info->grp][info->index].pname);
                pthread_mutex_unlock(&stadiums_lock);
                pthread_mutex_lock(&personlock[info->grp][info->index]);
                pdetails[info->grp][info->index].isinsystem = 0;
                pthread_mutex_unlock(&personlock[info->grp][info->index]);
                
                int waiting = 0;
                for (int i = 0; i < grp[info->grp]; i++)
                {
                    pthread_mutex_lock(&personlock[info->grp][i]);
                    if (pdetails[info->grp][i].isinsystem == 1)
                    {
                        printf(BLU "%s is waiting for their friends at the exit\n" reset, pdetails[info->grp][info->index].pname);
                        waiting = 1;
                        pthread_mutex_unlock(&personlock[info->grp][i]);
                        break;
                    }
                    pthread_mutex_unlock(&personlock[info->grp][i]);
                }
                if (waiting == 0)
                {
                    printf(BLU "Group %d is leaving for dinner\n" reset, info->grp+1);
                }
                return NULL;
            }
        }
        else
        {
            if (curcapn > 0)
            {
                curcapn--;
                pdetails[info->grp][info->index].zonealloc = 'N';
            }
            else if (curcaph > 0)
            {
                curcaph--;
                pdetails[info->grp][info->index].zonealloc = 'H';
            }
            else
            {
                curcapa--;
                pdetails[info->grp][info->index].zonealloc = 'A';
            }
        }
    }
    pthread_mutex_unlock(&stadiums_lock);
    printf(MAG "%s has got a seat in zone %c\n" reset, pdetails[info->grp][info->index].pname, pdetails[info->grp][info->index].zonealloc);

    //printf("%s has got a seat in zone %c\n", pdetails[info->grp][info->index].pname, pdetails[info->grp][info->index].zonealloc);

    if (pdetails[info->grp][info->index].fanteam == 'N')
    {
        // printf("%s entered Neutral condition\n",pdetails[info->grp][info->index].pname);
        sleep(X);
        pthread_mutex_lock(&stadiums_lock);

        char zonealloc = pdetails[info->grp][info->index].zonealloc;
        pthread_mutex_lock(&queue_lock);
        if (waitq.nmember > 0)
        {
            if (waitq.person[waitq.nmember - 1].team == 'A')
            {
                if (zonealloc == 'A')
                {
                    pdetails[waitq.person[waitq.nmember - 1].grp][waitq.person[waitq.nmember - 1].idx].zonealloc = 'A';
                    waitq.nmember--;
                    pthread_cond_signal(&speccond[waitq.person[waitq.nmember - 1].grp][waitq.person[waitq.nmember - 1].idx]);
                }
            }
            else if (waitq.person[waitq.nmember - 1].team == 'H')
            {
                if (zonealloc == 'H')
                {
                    pdetails[waitq.person[waitq.nmember - 1].grp][waitq.person[waitq.nmember - 1].idx].zonealloc = 'H';
                    waitq.nmember--;
                    pthread_cond_signal(&speccond[waitq.person[waitq.nmember - 1].grp][waitq.person[waitq.nmember - 1].idx]);
                }
                else if (zonealloc == 'N')
                {
                    pdetails[waitq.person[waitq.nmember - 1].grp][waitq.person[waitq.nmember - 1].idx].zonealloc = 'N';
                    waitq.nmember--;
                    pthread_cond_signal(&speccond[waitq.person[waitq.nmember - 1].grp][waitq.person[waitq.nmember - 1].idx]);
                }
            }
            else
            {
                if (zonealloc == 'H')
                {
                    pdetails[waitq.person[waitq.nmember - 1].grp][waitq.person[waitq.nmember - 1].idx].zonealloc = 'H';
                    waitq.nmember--;
                    pthread_cond_signal(&speccond[waitq.person[waitq.nmember - 1].grp][waitq.person[waitq.nmember - 1].idx]);
                }
                else if (zonealloc == 'N')
                {
                    pdetails[waitq.person[waitq.nmember - 1].grp][waitq.person[waitq.nmember - 1].idx].zonealloc = 'N';
                    waitq.nmember--;
                    pthread_cond_signal(&speccond[waitq.person[waitq.nmember - 1].grp][waitq.person[waitq.nmember - 1].idx]);
                }
                else
                {
                    pdetails[waitq.person[waitq.nmember - 1].grp][waitq.person[waitq.nmember - 1].idx].zonealloc = 'A';
                    waitq.nmember--;
                    pthread_cond_signal(&speccond[waitq.person[waitq.nmember - 1].grp][waitq.person[waitq.nmember - 1].idx]);
                }
            }
        }
        else
        {
            if (zonealloc == 'A')
            {
                curcapa++;
            }
            else if (zonealloc == 'H')
            {
                curcaph++;
            }
            else
            {
                curcapn++;
            }
        }
        pthread_mutex_unlock(&queue_lock);
        printf(GRN "%s watched the match for %d seconds and is leaving\n" reset, pdetails[info->grp][info->index].pname, X);
        //printf(YEL "%s is leaving for dinner\n" reset, pdetails[info->grp][info->index].pname);
        pthread_mutex_unlock(&stadiums_lock);
        pthread_mutex_lock(&personlock[info->grp][info->index]);
        pdetails[info->grp][info->index].isinsystem = 0;
        pthread_mutex_unlock(&personlock[info->grp][info->index]);
        
        int waiting = 0;
        for (int i = 0; i < grp[info->grp]; i++)
        {
            pthread_mutex_lock(&personlock[info->grp][i]);
            if (pdetails[info->grp][i].isinsystem == 1)
            {
                printf(BLU "%s is waiting for their friends at the exit\n" reset, pdetails[info->grp][info->index].pname);
                waiting = 1;
                pthread_mutex_unlock(&personlock[info->grp][i]);
                break;
            }
            pthread_mutex_unlock(&personlock[info->grp][i]);
        }
        if (waiting == 0)
        {
            printf(BLU "Group %d is leaving for dinner\n" reset, info->grp+1);
        }
        //pthread_mutex_unlock(&personlock[info->grp][info->index]);
        return NULL;
    }

    //printf("%s entered lock\n",pdetails[info->grp][info->index].pname);

    struct timespec start2;
    if (pdetails[info->grp][info->index].fanteam == 'H')
    {
        pthread_mutex_lock(&personlock[info->grp][info->index]);
        //printf("%s acquired lock\n",pdetails[info->grp][info->index].pname);
        clock_gettime(CLOCK_REALTIME, &start2);
        start2.tv_sec += X;
        int rc;
        while (pdetails[info->grp][info->index].rage == 0)
        {
            //printf("%s entered timed wait lock\n",pdetails[info->grp][info->index].pname);
            rc = pthread_cond_timedwait(&ragecond[info->grp][info->index], &personlock[info->grp][info->index], &start2);
            if (rc == ETIMEDOUT)
            {
                break;
            }
        }
        pthread_mutex_unlock(&personlock[info->grp][info->index]);
        //printf("%s came out of lock\n",pdetails[info->grp][info->index].pname);
        char zonealloc = pdetails[info->grp][info->index].zonealloc;
        pthread_mutex_lock(&queue_lock);
        if (waitq.nmember > 0)
        {
            int index = -1;
            for (int i = 0; i < waitq.nmember; i++)
            {
                if (waitq.person[waitq.nmember - 1].team != 'A')
                {
                    index = i;
                    break;
                }
            }
            if (index >= 0)
            {

                if (zonealloc == 'H')
                {
                    pdetails[waitq.person[index].grp][waitq.person[index].idx].zonealloc = 'H';
                    waitq.nmember--;
                    pthread_cond_signal(&speccond[waitq.person[index].grp][waitq.person[index].idx]);
                }
                else if (zonealloc == 'N')
                {
                    pdetails[waitq.person[index].grp][waitq.person[index].idx].zonealloc = 'N';
                    waitq.nmember--;
                    pthread_cond_signal(&speccond[waitq.person[index].grp][waitq.person[index].idx]);
                }
            }
            else
            {
                if (zonealloc == 'H')
                {
                    curcaph++;
                }
                else
                {
                    curcapn++;
                }
            }
        }
        else
        {
            if (zonealloc == 'H')
            {
                curcaph++;
            }
            else
            {
                curcapn++;
            }
        }
        pthread_mutex_unlock(&queue_lock);
        if (rc == ETIMEDOUT)
        {
            printf(GRN "%s watched the match for %d seconds and is leaving\n" reset, pdetails[info->grp][info->index].pname, X);
        }
        else
        {
            printf(GRN "%s is leaving due to the bad defensive performance of his team\n" reset, pdetails[info->grp][info->index].pname);
        }
        // printf(YEL "%s is leaving for dinner\n" reset, pdetails[info->grp][info->index].pname);
        pthread_mutex_unlock(&stadiums_lock);
        pthread_mutex_lock(&personlock[info->grp][info->index]);
        pdetails[info->grp][info->index].isinsystem = 0;
        pthread_mutex_unlock(&personlock[info->grp][info->index]);
        
        int waiting = 0;
        for (int i = 0; i < grp[info->grp]; i++)
        {
            pthread_mutex_lock(&personlock[info->grp][i]);
            if (pdetails[info->grp][i].isinsystem == 1)
            {
                printf(BLU "%s is waiting for their friends at the exit\n" reset, pdetails[info->grp][info->index].pname);
                waiting = 1;
                pthread_mutex_unlock(&personlock[info->grp][i]);
                break;
            }
            pthread_mutex_unlock(&personlock[info->grp][i]);
        }
        if (waiting == 0)
        {
            printf(BLU "Group %d is leaving for dinner\n" reset, info->grp+1);
        }
        //pthread_mutex_unlock(&personlock[info->grp][info->index]);
        return NULL;
    }
    else
    {
        pthread_mutex_lock(&personlock[info->grp][info->index]);
        //printf("%s entered lock condition\n", pdetails[info->grp][info->index].pname);
        clock_gettime(CLOCK_REALTIME, &start2);
        start2.tv_sec += X;
        int rc;
        while (pdetails[info->grp][info->index].rage == 0)
        {
            //printf("%s entered rage condition\n",pdetails[info->grp][info->index].pname);

            rc = pthread_cond_timedwait(&ragecond[info->grp][info->index], &personlock[info->grp][info->index], &start2);
            if (rc == ETIMEDOUT)
                break;
        }

        pthread_mutex_unlock(&personlock[info->grp][info->index]);
        char zonealloc = pdetails[info->grp][info->index].zonealloc;
        pthread_mutex_lock(&queue_lock);
        if (waitq.nmember > 0)
        {
            int index = -1;
            for (int i = 0; i < waitq.nmember; i++)
            {
                if (waitq.person[waitq.nmember - 1].team == 'A')
                {
                    index = i;
                    break;
                }
            }
            if (index >= 0)
            {
                pdetails[waitq.person[index].grp][waitq.person[index].idx].zonealloc = 'A';
                waitq.nmember--;
                pthread_cond_signal(&speccond[waitq.person[index].grp][waitq.person[index].idx]);
            }
            else
            {
                curcapa++;
            }
        }
        else
        {
            curcapa++;
        }
        pthread_mutex_unlock(&queue_lock);
        if (rc == ETIMEDOUT)
        {
            printf(GRN "%s watched the match for %d seconds and is leaving\n" reset, pdetails[info->grp][info->index].pname, X);
        }
        else
        {
            printf(GRN "%s is leaving due to the bad defensive performance of his team\n" reset, pdetails[info->grp][info->index].pname);
        }
        //printf(YEL "%s is leaving for dinner\n" reset, pdetails[info->grp][info->index].pname);
        pthread_mutex_unlock(&stadiums_lock);
        pthread_mutex_lock(&personlock[info->grp][info->index]);
        pdetails[info->grp][info->index].isinsystem = 0;
        pthread_mutex_unlock(&personlock[info->grp][info->index]);
        
        int waiting = 0;
        for (int i = 0; i < grp[info->grp]; i++)
        {
            pthread_mutex_lock(&personlock[info->grp][i]);
            if (pdetails[info->grp][i].isinsystem == 1)
            {
                printf(BLU "%s is waiting for their friends at the exit\n" reset, pdetails[info->grp][info->index].pname);
                waiting = 1;
                pthread_mutex_unlock(&personlock[info->grp][i]);
                break;
            }
            pthread_mutex_unlock(&personlock[info->grp][i]);
        }
        if (waiting == 0)
        {
            printf(BLU "Group %d is leaving for dinner\n" reset, info->grp+1);
        }
        //pthread_mutex_unlock(&personlock[info->grp][info->index]);
        return NULL;
    }
    return NULL;
}

int main()
{

    scanf("%d %d %d", &caph, &capa, &capn);
    curcaph = caph;
    curcapa = capa;
    curcapn = capn;
    scanf("%d", &X);
    scanf("%d", &num_grps);
    pthread_t **personthread;
    
    pthread_t *chancethread;

    waitq.nmember = 0;
    waitq.person = (stqueuenode *)malloc(sizeof(stqueuenode) * 20);
    pdetails = (stperson **)malloc(sizeof(stperson *) * num_grps);
    personthread = (pthread_t **)malloc(sizeof(pthread_t *) * num_grps);
    personlock = (pthread_mutex_t **)malloc(sizeof(pthread_mutex_t *) * num_grps);
    speccond = (pthread_cond_t **)malloc(sizeof(pthread_cond_t *) * num_grps);
    ragecond = (pthread_cond_t **)malloc(sizeof(pthread_cond_t *) * num_grps);
    grp = (int *)malloc(sizeof(int) * num_grps);
    for (int i = 0; i < num_grps; i++)
    {
        int k;
        scanf("%d", &k);
        //printf("k = %d\n",k);
        grp[i] = k;
        pdetails[i] = (stperson *)malloc(sizeof(stperson) * k);
        personthread[i] = (pthread_t *)malloc(sizeof(pthread_t) * k);
        personlock[i] = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t) * k);
        speccond[i] = (pthread_cond_t *)malloc(sizeof(pthread_cond_t) * k);
        ragecond[i] = (pthread_cond_t *)malloc(sizeof(pthread_cond_t) * k);
        for (int j = 0; j < k; j++)
        {
            scanf("%s %c %d %d %d", pdetails[i][j].pname, &pdetails[i][j].fanteam, &pdetails[i][j].time, &pdetails[i][j].patiencevalue, &pdetails[i][j].goals);
            pdetails[i][j].zonealloc = '0';
            pdetails[i][j].isinsystem = 1;
            pdetails[i][j].hasarrived = 0;
            pdetails[i][j].rage = 0;
        }
    }
    scanf("%d", &goalscoringchance);
    chancethread = (pthread_t *)malloc(sizeof(pthread_t) * goalscoringchance);
    //printf("Goalscoring chance = %d\n",goalscoringchance);
    chdetails = (stchance *)malloc(sizeof(stchance) * goalscoringchance);
    for (int i = 0; i < goalscoringchance; i++)
    {
        char temp1;
        int temp2;
        float temp3;
        //printf("Scanned %d times\n",i);
        scanf("%c %d %f", &temp1, &temp2, &temp3);
        scanf("%c %d %f", &chdetails[i].team, &chdetails[i].telapsed, &chdetails[i].prob);
        //printf("%c %d %f",chdetails[i].team,chdetails[i].telapsed,chdetails[i].prob);
    }
   
    gi **temp;
    temp = (gi **)malloc(sizeof(gi *) * num_grps);
    gi **temp2[num_grps];
    
    clock_gettime(CLOCK_REALTIME, &globalstart);
    for (int i = 0; i < num_grps; i++)
    {
        temp[i] = (gi *)malloc(sizeof(gi) * grp[i]);
        temp2[i] = (gi **)malloc(sizeof(gi *) * grp[i]);
        for (int j = 0; j < grp[i]; j++)
        {
            temp[i][j].grp = i;
            temp[i][j].index = j;
            temp2[i][j] = &temp[i][j];
            pthread_create(&personthread[i][j], NULL, person_func, (void *)temp2[i][j]);
        }
    }
    for (int i = 0; i < goalscoringchance; i++)
    {
        int *idx = (int *)malloc(sizeof(int));
        *idx = i;
        pthread_create(&chancethread[i], NULL, team_func, (void *)idx);
    }
    for (int i = 0; i < num_grps; i++)
    {
        for (int j = 0; j < grp[i]; j++)
        {
            pthread_join(personthread[i][j], NULL);
        }
    }
}

/*
2 1 2
3
2
3
Vibhav N 3 2 -1
Sarthak H 1 3 2
Ayush A 2 1 4
4
Rachit H 1 2 4
Roshan N 2 1 -1
Adarsh A 1 2 1
Pranav N 3 1 -1
5
H 1 1
A 2 0.95
A 3 0.5
H 5 0.85
H 6 0.4
*/
