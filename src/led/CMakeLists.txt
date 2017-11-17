upm_mixed_module_init (NAME led
    DESCRIPTION "Light-emitting Diode (LED)"
    C_HDR led.h open62541.h
    C_SRC led.c open62541.c
    CPP_HDR led.hpp
    CPP_SRC led.cxx
    FTI_SRC led_fti.c
    CPP_WRAPS_C
    REQUIRES mraa)
