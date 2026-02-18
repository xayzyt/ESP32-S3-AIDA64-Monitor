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
