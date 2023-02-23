idf_build_get_property(project_dir PROJECT_DIR)
idf_component_register(SRCS "app.c" "nvs.c" "wifi.c" "ble.c" "ota.c" "misc.c" "aes.c" "mqtt.c" "gpio.c" "device.c" "switch.c"
                    INCLUDE_DIRS "."
                    EMBED_TXTFILES ${project_dir}/server_certs/ca_cert.pem
                                   ${project_dir}/rsa_key/private.pem
                                   ${project_dir}/client_certs/aws_client_cert.pem
                                   ${project_dir}/client_certs/aws_client_key.pem)

target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-error=unused-const-variable)

create_esp_enc_img(${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}.bin
    ${project_dir}/rsa_key/private.pem ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}_secure.bin app)
