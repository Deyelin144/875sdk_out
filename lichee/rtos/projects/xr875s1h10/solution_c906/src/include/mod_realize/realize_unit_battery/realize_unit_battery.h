#ifndef _REALIZE_UNIT_BATTERY_H_
#define _REALIZE_UNIT_BATTERY_H_

enum {
	REALIZE_UNIT_BATTERY_NORMAL = 40,
	REALIZE_UNIT_BATTERY_LOW1 = 30,
	REALIZE_UNIT_BATTERY_LOW2 = 20,
	REALIZE_UNIT_BATTERY_SHUTDOWN = 8,
};

typedef struct {
    unsigned char is_init;
    unsigned char is_charge;
    unsigned char is_full;
	unsigned char low_power;
    unsigned int val;
} battery_info_t;

typedef struct {
	void *user_data;
	void (*battery_info)(void *);
} unit_battery_ctx_t;

typedef struct {
    unsigned int input_vol;
    unsigned int actual_vol;
} battery_vol_t;

typedef struct {
    unsigned int curr_adc;
    unsigned int curr_vol;
    unsigned int cali_vol;
    int diff;
    unsigned int bat_percent;
    unsigned int display_bat_percent;
} cali_info_t;

unsigned char realize_unit_battery_is_executing(void);
void realize_unit_battery_update_immediately();
int realize_unit_battery_cali(unsigned int vol, unsigned int diff, battery_vol_t *bat_vol);
int realize_unit_battery_get_cali_info(cali_info_t *cali_info);
int realize_unit_battery_init(void);
int realize_unit_battery_get_info(battery_info_t *battery_info);
int realize_unit_battery_set_timer(unit_battery_ctx_t *battery_ctx);
#endif

