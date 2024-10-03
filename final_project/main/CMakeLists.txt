idf_component_register(SRCS "RGB.c" "ADC_Driver.c" "wifi_app.c" "http_server.c" "main.c" 
"control_app/RGB_control.c" "control_app/BUZZER_control.c" "control_app/FAN_control.c"

                    INCLUDE_DIRS "."
                    EMBED_FILES webpage/app.css webpage/app.js webpage/favicon.ico webpage/index.html webpage/jquery-3.3.1.min.js
                    )
