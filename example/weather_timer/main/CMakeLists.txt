set(srcs
    main.c
    ./data/img_bilibili120.c
    ./data/font_led.c
    ./data/font_myawesome.c
    ./data/image_taikong.c
    ./data/font_qweather.c
    ./data/font_beijing.c
    ./data/font_alipuhui20.c
    ./dev/device_manage.c
    ./dev/device_platform.c
    ./bsp/gxhtc3.c
    ./bsp/qmc5883l.c
    ./bsp/qmi8658c.c
    ./bsp/lcd.c
    ./bsp/general/extend_io.c
    ./bsp/general/gen_i2c.c
    ./bsp/general/soft_i2c.c
    ./bsp/music.c
    ./bsp/ttv.c
    ./bsp/wifi.c
    ./bsp/sd_card.c
    ./bsp/camera.c
    ./app/app_wifi.c
    ./app/app_sr.c
    ./app/app_music.c
    ./app/app_API.c
    ./app/app_ui.c
    ./app/app_weather_ui.c
    ./app/app_2048.c
    ./app/app_dir.c
    ./app/app_set.c
    ./app/app_game.c
    ./app/app_init.c
    ./data/five.c
    ./data/grab.c
    )

idf_component_register(SRCS ${srcs}
                    INCLUDE_DIRS "." "./dev" "./bsp/inc" "./bsp/general" "./app/inc")

# spiffs_create_partition_image(storage ../spiffs FLASH_IN_PROJECT)
# storage,  data, spiffs,  ,        4M
# Determine whether esp-sr is fetched from component registry or from local path


# idf_build_get_property(build_components BUILD_COMPONENTS)
# if(esp-sr IN_LIST build_components)
#     set(esp-sr_name esp-sr) # Local component
# else()
#     set(esp-sr_name espressif__esp-sr) # Managed component
# endif()

# # Get path to voice data file
# idf_component_get_property(esp-sr_path ${esp-sr_name} COMPONENT_DIR)
# set(voice_data_image ${esp-sr_path}/esp-tts/esp_tts_chinese/esp_tts_voice_data_xiaoxin_small.dat)
# add_custom_target(voice_data ALL DEPENDS ${voice_data_image})
# add_dependencies(flash voice_data)

# partition_table_get_partition_info(size "--partition-name voice_data" "size")
# partition_table_get_partition_info(offset "--partition-name voice_data" "offset")

# if("${size}" AND "${offset}")
#     esptool_py_flash_to_partition(flash "voice_data" "${voice_data_image}")
# else()
#     set(message "Failed to find model in partition table file"
#                 "Please add a line(Name=voice_data, Type=data, Size=3890K) to the partition file.")
# endif()

