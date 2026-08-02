# CMakeLists.txt do componente 'main' do ESP-IDF
idf_component_register(
    SRCS 
        "main.c"
        "agendador.c"
        "autovigilancia.c"
        "redeWifi.c"
        "relogioNtp.c"
        "sensorFluxo.c"
        "clienteMqtt.c"
    INCLUDE_DIRS 
        "../include"
)
