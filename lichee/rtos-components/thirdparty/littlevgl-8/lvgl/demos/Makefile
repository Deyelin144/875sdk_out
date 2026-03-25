LVDEMOSDIR=../lvgl/demos

LVGL_BENCHMARKFILES=$(LVDEMOSDIR)/benchmark/lv_demo_benchmark.o \
	$(LVDEMOSDIR)/benchmark/assets/img_benchmark_cogwheel_alpha16.o \
	$(LVDEMOSDIR)/benchmark/assets/img_benchmark_cogwheel_argb.o \
	$(LVDEMOSDIR)/benchmark/assets/img_benchmark_cogwheel_chroma_keyed.o \
	$(LVDEMOSDIR)/benchmark/assets/img_benchmark_cogwheel_indexed16.o \
	$(LVDEMOSDIR)/benchmark/assets/img_benchmark_cogwheel_rgb.o \
	$(LVDEMOSDIR)/benchmark/assets/lv_font_bechmark_montserrat_12_compr_az.c.o \
	$(LVDEMOSDIR)/benchmark/assets/lv_font_bechmark_montserrat_16_compr_az.c.o \
	$(LVDEMOSDIR)/benchmark/assets/lv_font_bechmark_montserrat_28_compr_az.c.o
 
LVGL_KEYPADFILES=$(LVDEMOSDIR)/keypad_encoder/lv_demo_keypad_encoder.o

LVGL_MUSICFILES=$(LVDEMOSDIR)/music/lv_demo_music.o \
	$(LVDEMOSDIR)/music/lv_demo_music_list.o \
	$(LVDEMOSDIR)/music/lv_demo_music_main.o \
	$(LVDEMOSDIR)/music/assets/img_lv_demo_music_btn_corner_large.o \
	$(LVDEMOSDIR)/music/assets/img_lv_demo_music_btn_list_pause.o \
	$(LVDEMOSDIR)/music/assets/img_lv_demo_music_btn_list_pause_large.o \
	$(LVDEMOSDIR)/music/assets/img_lv_demo_music_btn_list_play.o \
	$(LVDEMOSDIR)/music/assets/img_lv_demo_music_btn_list_play_large.o \
	$(LVDEMOSDIR)/music/assets/img_lv_demo_music_btn_loop.o \
	$(LVDEMOSDIR)/music/assets/img_lv_demo_music_btn_loop_large.o \
	$(LVDEMOSDIR)/music/assets/img_lv_demo_music_btn_next.o \
	$(LVDEMOSDIR)/music/assets/img_lv_demo_music_btn_next_large.o \
	$(LVDEMOSDIR)/music/assets/img_lv_demo_music_btn_pause.o \
	$(LVDEMOSDIR)/music/assets/img_lv_demo_music_btn_pause_large.o \
	$(LVDEMOSDIR)/music/assets/img_lv_demo_music_btn_play.o \
	$(LVDEMOSDIR)/music/assets/img_lv_demo_music_btn_play_large.o \
	$(LVDEMOSDIR)/music/assets/img_lv_demo_music_btn_prev.o \
	$(LVDEMOSDIR)/music/assets/img_lv_demo_music_btn_prev_large.o \
	$(LVDEMOSDIR)/music/assets/img_lv_demo_music_btn_rnd.o \
	$(LVDEMOSDIR)/music/assets/img_lv_demo_music_btn_rnd_large.o \
	$(LVDEMOSDIR)/music/assets/img_lv_demo_music_corner_left.o \
	$(LVDEMOSDIR)/music/assets/img_lv_demo_music_corner_left_large.o \
	$(LVDEMOSDIR)/music/assets/img_lv_demo_music_corner_right.o \
	$(LVDEMOSDIR)/music/assets/img_lv_demo_music_corner_right_large.o \
	$(LVDEMOSDIR)/music/assets/img_lv_demo_music_cover_1.o \
	$(LVDEMOSDIR)/music/assets/img_lv_demo_music_cover_1_large.o \
	$(LVDEMOSDIR)/music/assets/img_lv_demo_music_cover_2.o \
	$(LVDEMOSDIR)/music/assets/img_lv_demo_music_cover_2_large.o \
	$(LVDEMOSDIR)/music/assets/img_lv_demo_music_cover_3.o \
	$(LVDEMOSDIR)/music/assets/img_lv_demo_music_cover_3_large.o \
	$(LVDEMOSDIR)/music/assets/img_lv_demo_music_icon_1.o \
	$(LVDEMOSDIR)/music/assets/img_lv_demo_music_icon_1_large.o \
	$(LVDEMOSDIR)/music/assets/img_lv_demo_music_icon_2.o \
	$(LVDEMOSDIR)/music/assets/img_lv_demo_music_icon_2_large.o \
	$(LVDEMOSDIR)/music/assets/img_lv_demo_music_icon_3.o \
	$(LVDEMOSDIR)/music/assets/img_lv_demo_music_icon_3_large.o \
	$(LVDEMOSDIR)/music/assets/img_lv_demo_music_icon_4.o \
	$(LVDEMOSDIR)/music/assets/img_lv_demo_music_icon_4_large.o \
	$(LVDEMOSDIR)/music/assets/img_lv_demo_music_list_border.o \
	$(LVDEMOSDIR)/music/assets/img_lv_demo_music_list_border_large.o \
	$(LVDEMOSDIR)/music/assets/img_lv_demo_music_logo.o \
	$(LVDEMOSDIR)/music/assets/img_lv_demo_music_slider_knob.o \
	$(LVDEMOSDIR)/music/assets/img_lv_demo_music_slider_knob_large.o \
	$(LVDEMOSDIR)/music/assets/img_lv_demo_music_wave_bottom.o \
	$(LVDEMOSDIR)/music/assets/img_lv_demo_music_wave_bottom_large.o \
	$(LVDEMOSDIR)/music/assets/img_lv_demo_music_wave_top.o \
	$(LVDEMOSDIR)/music/assets/img_lv_demo_music_wave_top_large.o

LVGL_STRESSFILES=$(LVDEMOSDIR)/stress/lv_demo_stress.o

LVGL_DEMOWIDGETSFILES=$(LVDEMOSDIR)/widgets/lv_demo_widgets.o \
	$(LVDEMOSDIR)/widgets/assets/img_clothes.o \
	$(LVDEMOSDIR)/widgets/assets/img_demo_widgets_avatar.o \
	$(LVDEMOSDIR)/widgets/assets/img_lvgl_logo.o

LVDEMOS_OBJECTS= \
	$(LVGL_DEMOWIDGETSFILES) \
	$(LVGL_MUSICFILES) \
	$(LVGL_BENCHMARKFILES) \
	$(LVGL_KEYPADFILES) \
	$(LVGL_STRESSFILES)

obj-y += $(LVDEMOS_OBJECTS)
