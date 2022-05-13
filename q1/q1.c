#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define BLK "\e[0;30m"
#define RED "\e[0;31m"
#define GRN "\e[0;32m"
#define YEL "\e[0;33m"
#define BLU "\e[0;34m"
#define MAG "\e[0;35m"
#define CYN "\e[0;36m"
#define WHT "\e[0;37m"

//Regular bold text
#define BBLK "\e[1;30m"
#define BRED "\e[1;31m"
#define BGRN "\e[1;32m"
#define BYEL "\e[1;33m"
#define BBLU "\e[1;34m"
#define BMAG "\e[1;35m"
#define BCYN "\e[1;36m"
#define BWHT "\e[1;37m"

//Regular underline text
#define UBLK "\e[4;30m"
#define URED "\e[4;31m"
#define UGRN "\e[4;32m"
#define UYEL "\e[4;33m"
#define UBLU "\e[4;34m"
#define UMAG "\e[4;35m"
#define UCYN "\e[4;36m"
#define UWHT "\e[4;37m"

//Reset
#define reset "\e[0m"

int num_students;
int num_courses;
int num_labs;

time_t start, end;

pthread_mutex_t **TAlock;
pthread_mutex_t *studentlock;
pthread_mutex_t *lablock;
pthread_mutex_t *courselock;
pthread_cond_t *LabAvail;
pthread_cond_t *Movetonextpref;

typedef struct
{
    float calibre;
    int prefcourseno[3];
    int filled;
    int time;
    int alloc;
    char curcoursename[20];
    int lastprefidx;
    int curprefcourseno;
    int finalcourseno;
    int attendedtut;
    int isinsystem;
    int curcourseremoved;
} ststudent;

typedef struct
{
    char coursename[20];
    float iq;
    int maxslots;
    int nlabs;
    int *labs;
    int lastenrol;
    int stenrol;
    int removed;
    int TAalloc;
    int curlaballot;
    int curTAallot;
} stcourse;

typedef struct
{
    int times;
    int occupied;
} stTA;

typedef struct
{
    char labname[20];
    int TAcount;
    int maxtimes;
    stTA *TA;
    int labvalid;
} stlab;

stlab *labdetails;
stcourse *cdetails;
ststudent *stdetails;

char pre[3];

void pref(int n)
{
    if (n % 10 == 1)
    {
        strcpy(pre, "st");
    }
    else if (n % 10 == 2)
    {
        strcpy(pre, "nd");
    }
    else if (n % 10 == 3)
    {
        strcpy(pre, "rd");
    }
    else
    {
        strcpy(pre, "th");
    }
    return;
}

void *course_func(void *idx)
{
    int id = *(int *)idx;
    sleep(1);
    while (1)
    {
    L1:
        if (cdetails[id].removed == 1)
        {
            return NULL;
        }

        int flag = 0;

        // Choose TA
        while (1)
        {
            // check for each lab the course can take
            for (int i = 0; i < cdetails[id].nlabs; i++)
            {
                int labflag = 0;
                int labno = cdetails[id].labs[i];

                // check if lab is valid
                pthread_mutex_lock(&lablock[i]);
                if (labdetails[labno].labvalid == 0)
                {
                    pthread_mutex_unlock(&lablock[i]);
                    continue;
                }
                pthread_mutex_unlock(&lablock[i]);

                //check for each TA in lab
                for (int j = 0; j < labdetails[labno].TAcount; j++)
                {

                    pthread_mutex_lock(&TAlock[labno][j]);

                    //already occupied
                    if (labdetails[labno].TA[j].occupied == 1)
                    {
                        if (labdetails[labno].TA[j].times < labdetails[labno].maxtimes)
                        {
                            flag = 1;
                            labflag = 1;
                        }
                        pthread_mutex_unlock(&TAlock[labno][j]);
                        continue;
                    }

                    // if not occupied and has turns left then assign that TA
                    if (labdetails[labno].TA[j].times < labdetails[labno].maxtimes)
                    {
                        flag = 1;
                        labflag = 1;
                        labdetails[labno].TA[j].times++;
                        labdetails[labno].TA[j].occupied = 1;
                        cdetails[id].curlaballot = labno;
                        cdetails[id].curTAallot = j;
                        cdetails[id].TAalloc = 1;
                        pref(labdetails[cdetails[id].curlaballot].TA[cdetails[id].curTAallot].times);
                        printf(MAG "TA %d from lab %s has been allocated to course %s for his %d%s TA ship\n" reset, cdetails[id].curTAallot, labdetails[cdetails[id].curlaballot].labname, cdetails[id].coursename, labdetails[cdetails[id].curlaballot].TA[cdetails[id].curTAallot].times, pre);
                    }

                    pthread_mutex_unlock(&TAlock[labno][j]);

                    // break if assigned
                    if (cdetails[id].TAalloc == 1)
                    {
                        break;
                    }
                }

                //if the lab has no TAs available
                if (labflag == 0)
                {
                    pthread_mutex_lock(&lablock[i]);
                    labdetails[i].labvalid = 0;
                    pthread_cond_signal(&LabAvail[i]);
                    pthread_mutex_unlock(&lablock[i]);
                }

                // if TA is assigned then break
                if (cdetails[id].TAalloc == 1)
                {
                    break;
                }
            }
            if (cdetails[id].TAalloc == 1)
            {
                break;
            }
            // check if none of the labs of the course is eligible
            if (flag == 0)
            {
                pthread_mutex_lock(&courselock[id]);
                cdetails[id].removed = 1;
                printf(BRED "Course %s doesn't have any TAs eligible and is removed from course offerings\n" reset, cdetails[id].coursename);
                pthread_mutex_unlock(&courselock[id]);

                for (int i = 0; i < num_students; i++)
                {
                    pthread_mutex_lock(&studentlock[i]);
                    if (stdetails[i].isinsystem == 1)
                    {
                        if (stdetails[i].curprefcourseno == id)
                        {
                            stdetails[i].curcourseremoved = 1;
                            pthread_cond_signal(&Movetonextpref[i]);
                        }
                    }
                    pthread_mutex_unlock(&studentlock[i]);
                }
                break;
            }
            sleep(2);
        }

        if (cdetails[id].removed == 0)
        {
            int sltsalloc = 0;
            int D = 0;
            D = (rand() % cdetails[id].maxslots) + 1;
            printf(BLU "Course %s has been allocated %d seats\n" reset, cdetails[id].coursename, D);

            for (int i = 0; i < num_students; i++)
            {
                pthread_mutex_lock(&studentlock[i]);
                if (stdetails[i].isinsystem == 1)
                {
                    if (stdetails[i].curprefcourseno == id && stdetails[i].alloc == -1 && stdetails[i].filled == 1)
                    {
                        stdetails[i].alloc = id;
                        printf(GRN "Student %d has been allocated a seat in course %s\n" reset, i, cdetails[id].coursename);
                        sltsalloc++;
                    }
                }
                pthread_mutex_unlock(&studentlock[i]);
                if (sltsalloc == D)
                {
                    break;
                }
            }

            sleep(1);
            // start tutorial with students
            printf(MAG "Tutorial has started for course %s with %d seats filled out of %d seats\n" reset, cdetails[id].coursename, sltsalloc, D);

            // conduct tutorial
            sleep(5);

            //end tutorial

            pthread_mutex_lock(&TAlock[cdetails[id].curlaballot][cdetails[id].curTAallot]);
            labdetails[cdetails[id].curlaballot].TA[cdetails[id].curTAallot].occupied = 0;
            printf(GRN "TA %d from lab %s has completed the tutorial for course %s\n" reset, cdetails[id].curTAallot, labdetails[cdetails[id].curlaballot].labname, cdetails[id].coursename);
            pthread_mutex_unlock(&TAlock[cdetails[id].curlaballot][cdetails[id].curTAallot]);

            cdetails[id].TAalloc = 0;
            cdetails[id].curTAallot = -1;
            cdetails[id].curlaballot = -1;

            for (int i = 0; i < num_students; i++)
            {
                pthread_mutex_lock(&studentlock[i]);
                if (stdetails[i].isinsystem == 1)
                {
                    if (stdetails[i].alloc == id)
                    {
                        stdetails[i].attendedtut = 1;
                        stdetails[i].alloc = -1;
                        pthread_cond_signal(&Movetonextpref[i]);
                        //printf("Sent signal to student %d\n",i);
                    }
                }
                pthread_mutex_unlock(&studentlock[i]);
            }
        }
        sleep(2);
    }
    return NULL;
}

void *stu_func(void *idx)
{
    int id = *(int *)idx;
    sleep(stdetails[id].time);
    pthread_mutex_lock(&studentlock[id]);
    printf(GRN "Student %d has filled in preferences for course registration\n" reset, id);
    stdetails[id].filled = 1;
    pthread_mutex_unlock(&studentlock[id]);

    pthread_mutex_lock(&studentlock[id]);

L2:

    while (stdetails[id].attendedtut == 0 && stdetails[id].curcourseremoved == 0)
        pthread_cond_wait(&Movetonextpref[id], &studentlock[id]);

    //printf("Student %d got out of conditional wait\n");
    if (stdetails[id].attendedtut == 1)
    {
        //printf("student %d Entered tut condition\n", id);
        //stdetails[id].curcourseremoved = 0;
        float prob = cdetails[stdetails[id].curprefcourseno].iq * stdetails[id].calibre;
        int garbage = rand() % 100;
        if ((garbage / 100.00f) < prob)
        {
            stdetails[id].finalcourseno = stdetails[id].curprefcourseno;
            stdetails[id].isinsystem = 0;
            printf(BGRN "Student %d has selected course %s permanently\n" reset, id, cdetails[stdetails[id].curprefcourseno].coursename);
            pthread_mutex_unlock(&studentlock[id]);
            return NULL;
        }
        else if (stdetails[id].lastprefidx < 2)
        {
            printf(YEL "Student %d has withdrawn from course %s\n" reset, id, cdetails[stdetails[id].curprefcourseno].coursename);
            //printf("Student %d has changed current preference from course %s(priority %d) to course %s(priority %d)\n", id, cdetails[stdetails[id].curprefcourseno].coursename, stdetails[id].lastprefidx, cdetails[stdetails[id].prefcourseno[stdetails[id].lastprefidx]].coursename, stdetails[id].lastprefidx + 1);
            stdetails[id].lastprefidx++;
            stdetails[id].alloc = -1;
            stdetails[id].attendedtut = 0;

            pthread_mutex_lock(&courselock[stdetails[id].prefcourseno[stdetails[id].lastprefidx]]);
            if (cdetails[stdetails[id].prefcourseno[stdetails[id].lastprefidx]].removed == 1)
            {
                pthread_mutex_unlock(&courselock[stdetails[id].prefcourseno[stdetails[id].lastprefidx]]);
                //printf("Student %d has next course removed already\n", id);
                printf(CYN "Student %d has changed current preference from course %s(priority %d) to course %s(priority %d)\n" reset, id, cdetails[stdetails[id].curprefcourseno].coursename, stdetails[id].lastprefidx, cdetails[stdetails[id].prefcourseno[stdetails[id].lastprefidx]].coursename, stdetails[id].lastprefidx + 1);

                stdetails[id].curprefcourseno = stdetails[id].prefcourseno[stdetails[id].lastprefidx];
                stdetails[id].lastprefidx++;

                if (stdetails[id].lastprefidx > 2)
                {
                    stdetails[id].curprefcourseno = -1;
                    goto L3;
                }

                pthread_mutex_lock(&courselock[stdetails[id].prefcourseno[stdetails[id].lastprefidx]]);

                if (cdetails[stdetails[id].prefcourseno[stdetails[id].lastprefidx]].removed == 1)
                {
                    // printf("Student %d has next course removed already\n", id);
                    pthread_mutex_unlock(&courselock[stdetails[id].prefcourseno[stdetails[id].lastprefidx]]);
                    printf(CYN "Student %d has changed current preference from course %s(priority %d) to course %s(priority %d)\n" reset, id, cdetails[stdetails[id].curprefcourseno].coursename, stdetails[id].lastprefidx, cdetails[stdetails[id].prefcourseno[stdetails[id].lastprefidx]].coursename, stdetails[id].lastprefidx + 1);
                    stdetails[id].curprefcourseno = -1;
                    goto L3;
                }

                pthread_mutex_unlock(&courselock[stdetails[id].prefcourseno[stdetails[id].lastprefidx]]);

                printf(CYN "Student %d has changed current preference from course %s(priority %d) to course %s(priority %d)\n" reset, id, cdetails[stdetails[id].curprefcourseno].coursename, stdetails[id].lastprefidx, cdetails[stdetails[id].prefcourseno[stdetails[id].lastprefidx]].coursename, stdetails[id].lastprefidx + 1);
                stdetails[id].curprefcourseno = stdetails[id].prefcourseno[stdetails[id].lastprefidx];
                strcpy(stdetails[id].curcoursename, cdetails[stdetails[id].curprefcourseno].coursename);
            }
            else
            {
                pthread_mutex_unlock(&courselock[stdetails[id].prefcourseno[stdetails[id].lastprefidx]]);
                printf(CYN "Student %d has changed current preference from course %s(priority %d) to course %s(priority %d)\n" reset, id, cdetails[stdetails[id].curprefcourseno].coursename, stdetails[id].lastprefidx, cdetails[stdetails[id].prefcourseno[stdetails[id].lastprefidx]].coursename, stdetails[id].lastprefidx + 1);
                stdetails[id].curprefcourseno = stdetails[id].prefcourseno[stdetails[id].lastprefidx];
                strcpy(stdetails[id].curcoursename, cdetails[stdetails[id].curprefcourseno].coursename);
            }
            goto L2;
        }
        else
        {
        L3:
            if (stdetails[id].attendedtut == 1)
                printf(YEL "Student %d has withdrawn from course %s\n" reset, id, cdetails[stdetails[id].curprefcourseno].coursename);
            printf(RED "Student %d couldn't get any of his preferred courses\n" reset, id);
            stdetails[id].isinsystem = 0;
            stdetails[id].finalcourseno = -1;
            pthread_mutex_unlock(&studentlock[id]);
            return NULL;
        }
    }
    else
    {
        stdetails[id].curcourseremoved = 0;
        //printf("Student %d Entered course removed cond\n", id);

        if (stdetails[id].lastprefidx < 2)
        {
            //printf("Student %d has withdrawn from course %s\n", id, cdetails[stdetails[id].curprefcourseno].coursename);
            stdetails[id].lastprefidx++;
            stdetails[id].alloc = -1;
            stdetails[id].attendedtut = 0;

            pthread_mutex_lock(&courselock[stdetails[id].prefcourseno[stdetails[id].lastprefidx]]);
            if (cdetails[stdetails[id].prefcourseno[stdetails[id].lastprefidx]].removed == 1)
            {
                pthread_mutex_unlock(&courselock[stdetails[id].prefcourseno[stdetails[id].lastprefidx]]);
                //printf("Student %d has next course removed already\n", id);
                printf(CYN "Student %d has changed current preference from course %s(priority %d) to course %s(priority %d)\n" reset, id, cdetails[stdetails[id].curprefcourseno].coursename, stdetails[id].lastprefidx, cdetails[stdetails[id].prefcourseno[stdetails[id].lastprefidx]].coursename, stdetails[id].lastprefidx + 1);
                //printf("Student %d could not select course %s because it is already removed\n",id,cdetails[stdetails[id].prefcourseno[stdetails[id].lastprefidx]].coursename);
                stdetails[id].lastprefidx++;
                stdetails[id].curprefcourseno = stdetails[id].prefcourseno[stdetails[id].lastprefidx];

                if (stdetails[id].lastprefidx > 2)
                {
                    stdetails[id].curprefcourseno = -1;
                    goto L3;
                }

                pthread_mutex_lock(&courselock[stdetails[id].prefcourseno[stdetails[id].lastprefidx]]);

                if (cdetails[stdetails[id].prefcourseno[stdetails[id].lastprefidx]].removed == 1)
                {
                    //printf("Student %d has next course removed already\n", id);
                    printf(CYN "Student %d has changed current preference from course %s(priority %d) to course %s(priority %d)\n" reset, id, cdetails[stdetails[id].curprefcourseno].coursename, stdetails[id].lastprefidx, cdetails[stdetails[id].prefcourseno[stdetails[id].lastprefidx]].coursename, stdetails[id].lastprefidx + 1);
                    stdetails[id].curprefcourseno = stdetails[id].prefcourseno[stdetails[id].lastprefidx];
                    pthread_mutex_unlock(&courselock[stdetails[id].prefcourseno[stdetails[id].lastprefidx]]);
                    //printf("Student %d could not select course %s because it is already removed\n",id,cdetails[stdetails[id].prefcourseno[stdetails[id].lastprefidx]].coursename);
                    stdetails[id].curprefcourseno = -1;
                    goto L3;
                }

                pthread_mutex_unlock(&courselock[stdetails[id].prefcourseno[stdetails[id].lastprefidx]]);

                printf(CYN "Student %d has changed current preference from course %s(priority %d) to course %s(priority %d)\n" reset, id, cdetails[stdetails[id].curprefcourseno].coursename, stdetails[id].lastprefidx, cdetails[stdetails[id].prefcourseno[stdetails[id].lastprefidx]].coursename, stdetails[id].lastprefidx + 1);
                stdetails[id].curprefcourseno = stdetails[id].prefcourseno[stdetails[id].lastprefidx];
                strcpy(stdetails[id].curcoursename, cdetails[stdetails[id].curprefcourseno].coursename);
            }
            else
            {
                pthread_mutex_unlock(&courselock[stdetails[id].prefcourseno[stdetails[id].lastprefidx]]);
                printf(CYN "Student %d has changed current preference from course %s(priority %d) to course %s(priority %d)\n" reset, id, cdetails[stdetails[id].curprefcourseno].coursename, stdetails[id].lastprefidx, cdetails[stdetails[id].prefcourseno[stdetails[id].lastprefidx]].coursename, stdetails[id].lastprefidx + 1);
                stdetails[id].curprefcourseno = stdetails[id].prefcourseno[stdetails[id].lastprefidx];
                strcpy(stdetails[id].curcoursename, cdetails[stdetails[id].curprefcourseno].coursename);
            }
            //pthread_mutex_unlock(&studentlock[id]);
            goto L2;
        }
        else
        {
            printf(RED "Student %d couldn't get any of his preferred courses\n" reset, id);
            stdetails[id].isinsystem = 0;
            stdetails[id].finalcourseno = -1;
            pthread_mutex_unlock(&studentlock[id]);
            return NULL;
        }
    }
}

void *lab_func(void *idx)
{
    int id = *(int *)idx;
    pthread_mutex_lock(&lablock[id]);
    while (labdetails[id].labvalid == 1)
        pthread_cond_wait(&LabAvail[id], &lablock[id]);
    printf(BBLU "Lab %s no longer has students available for TA ship\n" reset, labdetails[id].labname);
    pthread_mutex_unlock(&lablock[id]);
    return NULL;
}

void init()
{
    studentlock = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t) * num_students);
    Movetonextpref = (pthread_cond_t *)malloc(sizeof(pthread_cond_t) * num_students);

    for (int i = 0; i < num_students; i++)
    {
        pthread_mutex_init(&studentlock[i], NULL);
        pthread_cond_init(&Movetonextpref[i], NULL);
    }

    courselock = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t) * num_courses);
    for (int i = 0; i < num_courses; i++)
    {
        pthread_mutex_init(&courselock[i], NULL);
    }

    lablock = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t) * num_labs);
    LabAvail = (pthread_cond_t *)malloc(sizeof(pthread_cond_t) * num_labs);

    for (int i = 0; i < num_labs; i++)
    {
        pthread_mutex_init(&lablock[i], NULL);
        pthread_cond_init(&LabAvail[i], NULL);
    }

    TAlock = (pthread_mutex_t **)malloc(sizeof(pthread_mutex_t *) * num_labs);

    cdetails = (stcourse *)malloc(sizeof(stcourse) * num_courses);
    stdetails = (ststudent *)malloc(sizeof(ststudent) * num_students);
    labdetails = (stlab *)malloc(sizeof(stlab) * num_labs);
    return;
}

void input()
{

    for (int i = 0; i < num_courses; i++)
    {
        scanf("%s %f %d %d", cdetails[i].coursename, &cdetails[i].iq, &cdetails[i].maxslots, &cdetails[i].nlabs);
        cdetails[i].labs = (int *)malloc(sizeof(int) * cdetails[i].nlabs);
        for (int j = 0; j < cdetails[i].nlabs; j++)
        {
            scanf("%d", &cdetails[i].labs[j]);
        }
        cdetails[i].stenrol = 0;
        cdetails[i].removed = 0;
        cdetails[i].curTAallot = -1;
        cdetails[i].TAalloc = 0;
        cdetails[i].curlaballot = -1;
        cdetails[i].lastenrol = 0;
    }

    for (int i = 0; i < num_students; i++)
    {
        scanf("%f %d %d %d %d", &stdetails[i].calibre, &stdetails[i].prefcourseno[0], &stdetails[i].prefcourseno[1], &stdetails[i].prefcourseno[2], &stdetails[i].time);
        stdetails[i].isinsystem = 1;
        stdetails[i].attendedtut = 0;
        stdetails[i].alloc = -1;
        stdetails[i].curprefcourseno = stdetails[i].prefcourseno[0];
        stdetails[i].finalcourseno = -1;
        stdetails[i].lastprefidx = 0;
        stdetails[i].filled = 0;
        stdetails[i].curcourseremoved = 0;
    }

    for (int i = 0; i < num_labs; i++)
    {
        scanf("%s %d %d", labdetails[i].labname, &labdetails[i].TAcount, &labdetails[i].maxtimes);
        labdetails[i].TA = (stTA *)malloc(sizeof(stTA) * labdetails[i].TAcount);

        TAlock[i] = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t) * labdetails[i].TAcount);
        for (int j = 0; j < labdetails[i].TAcount; j++)
        {
            pthread_mutex_init(&TAlock[i][j], NULL);
        }
        for (int j = 0; j < labdetails[i].TAcount; j++)
        {
            labdetails[i].TA[j].times = 0;
            labdetails[i].TA[j].occupied = 0;
        }
        labdetails[i].labvalid = 1;
    }
    return;
}

int main()
{
    srand(time(0));
    scanf("%d %d %d", &num_students, &num_labs, &num_courses);

    pthread_t students[num_students];
    pthread_t courses[num_courses];
    pthread_t labs[num_labs];

    // initialize mutex and conditional variables
    init();

    /*
    printf("Please wait for 2 sec before entering input\n");
    sleep(2);
    printf("Please enter the input now\n");
    */

    // take input
    input();

    // start time
    start = time(NULL);

    //launch threads into stu_func, course_func, lab_func
    for (int i = 0; i < num_students; i++)
    {
        int *idx = (int *)malloc(sizeof(int));
        *idx = i;
        pthread_create(&students[i], NULL, stu_func, (void *)(idx));
    }
    for (int i = 0; i < num_courses; i++)
    {
        int *idx = (int *)malloc(sizeof(int));
        *idx = i;
        pthread_create(&courses[i], NULL, course_func, (void *)(idx));
    }
    for (int i = 0; i < num_labs; i++)
    {
        int *idx = (int *)malloc(sizeof(int));
        *idx = i;
        pthread_create(&labs[i], NULL, lab_func, (void *)(idx));
    }

    // Wait for students to complete simulation
    for (int i = 0; i < num_students; i++)
    {
        pthread_join(students[i], NULL);
    }

    // print results of simulation
    printf("\nResults of the Simulation:\n");
    for (int i = 0; i < num_students; i++)
    {
        if (stdetails[i].finalcourseno != -1)
        {
            printf("Student %d got course %s(Priority: %d)\n", i, cdetails[stdetails[i].finalcourseno].coursename, stdetails[i].lastprefidx + 1);
        }
        else
        {
            printf("Student %d did not get any of his preferred courses\n", i);
        }
    }
    return 0;
}

/*
0.45 2 1 0 1
0.56 1 0 2 1
0.76 1 2 0 1
0.67 2 1 0 3
0.66 0 1 2 5
*/

/*
10 3 4
SMAI 0.8 3 2 0 2
NLP 0.95 4 1 0
CV 0.90 2 2 1 2
DSA 0.75 5 3 0 1 2
0.8 0 3 1 1
0.6 3 1 2 3
0.85 2 1 0 1
0.5 1 2 3 2
0.75 0 2 1 3
0.95 1 0 2 2
0.4 3 0 2 3
0.1 0 3 1 2
0.85 1 0 3 1
0.3 0 1 2 1
PRECOG 3 1
CVIT 4 2
RRC 1 3
*/