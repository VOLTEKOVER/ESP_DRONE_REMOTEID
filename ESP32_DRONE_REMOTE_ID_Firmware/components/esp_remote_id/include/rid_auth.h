#ifndef RID_AUTH_H
#define RID_AUTH_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_remote_id.h"

#define RID_AUTH_KEY_SIZE 32
#define RID_AUTH_SIG_SIZE 64
#define ODID_AUTH_PAGE_SIZE 23

bool rid_auth_init(const char *pem_key);
bool rid_auth_enabled(void);
bool rid_auth_sign_message(uint8_t msg_type, const uint8_t *msg_data, uint8_t msg_len,
                           uint8_t page_buf[ODID_AUTH_PAGE_SIZE], uint8_t *page_count);

#endif
