#ifndef SNTP_TIME
#define SNTP_TIME
#ifdef __cplusplus
extern "C" {
#endif

void time_sync_notification_cb(struct timeval *tv);

void obtain_time(void);

void initialize_sntp(void);
#ifdef __cplusplus
}
#endif
#endif