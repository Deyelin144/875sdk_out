#ifndef __ADB_RB_H
#define __ADB_RB_H

typedef struct adb_rb adb_rb;
typedef struct adb_ev adb_ev;

adb_rb *adb_ringbuffer_init(int size);
void adb_ringbuffer_release(adb_rb *rb);
int adb_ringbuffer_get(adb_rb *rb, void *buf, int size, int timeout);
int adb_ringbuffer_put(adb_rb *rb, const void *buf, int size);
adb_ev *adb_event_init(void);
void adb_event_release(adb_ev *ev);
int adb_event_set(adb_ev *ev, int bits);
int adb_event_get(adb_ev *ev, int bits, int ms);

#endif

#ifndef __ADB_EV_H
#define __ADB_EV_H






#endif