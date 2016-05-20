#include "svc.h"
#include "aux_timer.h"
#include "common/app/app.h"
#include "common/hal/hal.h"
#include "common/hal/lcd_segments.h"
#include <unistd.h>
#include <stdio.h>

void svc_main_proc(svc_main_proc_event_t event) {
	if(event & (SVC_MAIN_PROC_EVENT_KEY_ANY | SVC_MAIN_PROC_EVENT_KEY_ANY_LONG)) {
		svc_alarm_clear_pending();
		svc_countdown_clear_pending();
		svc_beep_key();
	}
	if(event & SVC_MAIN_PROC_EVENT_AUX_TIMER) {
		svc_aux_timer_handler();
	}
	svc_backlight_process(event);
	if(event & SVC_MAIN_PROC_EVENT_WDT) {
		svc_countdown_process();
		svc_compass_process();
		svc_otp_process();
	}
	if(!(event & SVC_MAIN_PROC_EVENT_AUX_TIMER) || svc_aux_timer_get_call_main()) {
		app_current_update();
		if(app_view_changed) {
			if(app_current->views[app_current->priv->view_current].enter) {
				app_current->views[app_current->priv->view_current].enter(app_current->priv->view_current, app_current);
				app_view_changed = 0;
			}
		}
		app_current->views[app_current->priv->view_current].main(app_current->priv->view_current, app_current, event);
	}
	
	if(svc_chro_get_state() == SVC_CHRO_STATE_RUN || svc_chro_get_state() == SVC_CHRO_STATE_HOLD) {
		hal_lcd_seg_set(HAL_LCD_SEG_LAP, 1);
		hal_lcd_seg_set_blink(HAL_LCD_SEG_LAP, 1);
	}
	svc_alarm_process();
	if(svc_alarm_get_any_enabled()) {
		hal_lcd_seg_set(HAL_LCD_SEG_BELL, 1);
		hal_lcd_seg_set_blink(HAL_LCD_SEG_BELL, svc_alarm_get_pending());
	}
	else {
		hal_lcd_seg_set(HAL_LCD_SEG_BELL, 0);
	}
	
	if((svc_countdown_get_n_running() > 0) || svc_countdown_get_pending()) {
		hal_lcd_seg_set(HAL_LCD_SEG_BARS, 1);
		hal_lcd_seg_set_blink(HAL_LCD_SEG_BARS, svc_countdown_get_pending());
	}
	else {
		hal_lcd_seg_set(HAL_LCD_SEG_BARS, 0);
	}
	
	if(event & SVC_MAIN_PROC_EVENT_WDT) {
		svc_countdown_draw_popup();
		svc_alarm_draw_popup();
	}
	
	svc_beep_hour();
}
