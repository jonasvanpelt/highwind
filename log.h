/*
 * AUTHOR: Jonas Van Pelt
 */

#ifndef LOG_H_ 
#define LOG_H_

//for initialization of log
extern int init_log();
extern int mount_sd_card();

//for logging data coming from lisa
extern int open_data_lisa_log();
extern int write_data_lisa_log(char *data);
extern int close_data_lisa_log();

//for loging data coming from groundstation
extern int open_data_groundstation_log();
extern int write_data_groundstation_log(char *data);
extern int close_data_groundstation_log();

//for logging program
extern void log_write(char *file_name,char *message);
extern void error_write(char *file_name,char *message);

#endif /*LOG_H_*/
