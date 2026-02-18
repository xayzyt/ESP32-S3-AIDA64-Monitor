<<<<<<< HEAD
#ifndef __AIDA64_H
#define __AIDA64_H

typedef struct 
{
    int cpu_rate;
    int cpu_temp;
    int mem_rate;
    int mem_use;
}aida64_data_t;


void aida64_monitor_start(const char* ip);

void aida64_monitor_stop(void);

int aida64_monitor_isconnect(void);


#endif
=======
#ifndef __AIDA64_H
#define __AIDA64_H

typedef struct 
{
    int cpu_rate;
    int cpu_temp;
    int mem_rate;
    int mem_use;
}aida64_data_t;


void aida64_monitor_start(const char* ip);

void aida64_monitor_stop(void);

int aida64_monitor_isconnect(void);


#endif
>>>>>>> 81fc6d11b1a3aa3674e1ca4f6c48bf0d832d09d3
