/*
 * @file: quectel_process.h
 *
 */

#ifndef __QUECTEL_PROCESS_H__
#define __QUECTEL_PROCESS_H__

#include <stdbool.h>

extern int quectel_check_process_by_name(const char *process_name);
extern int quectel_write_pid_file(const char *file);
extern int quectel_set_process_single_instance(const char *lock_file);
extern void quectel_release_process_single_instance(int fd);
extern int quectel_check_process_singal_instance(const char *lock_file);

#endif /* end of __QUECTEL_PROCESS_H__ */
