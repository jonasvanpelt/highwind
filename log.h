/*
 * AUTHOR: Jonas Van Pelt
 */
#ifndef LOG_H_ 
#define LOG_H_

enum log_errCode {LOG_ERR_NONE=0,LOG_ERR_UNDEFINED,LOG_ERR_MOUNT_SD,LOG_ERR_OPEN_FILE,LOG_ERR_WRITE,LOG_ERR_CLOSE};
typedef enum log_errCode LOG_errCode;

//for initialization of log
extern LOG_errCode init_log();
extern LOG_errCode mount_sd_card();

//for logging data coming from lisa
extern LOG_errCode open_data_lisa_log();
extern LOG_errCode write_data_lisa_log(char *data,int length);
extern LOG_errCode close_data_lisa_log();

//for loging data coming from groundstation
extern LOG_errCode open_data_groundstation_log();
extern LOG_errCode write_data_groundstation_log(char *data,int length);
extern LOG_errCode close_data_groundstation_log();

//for logging program
extern LOG_errCode log_write(char *file_name,char *message);
extern LOG_errCode error_write(char *file_name,char *message);

#endif /*LOG_H_*/
