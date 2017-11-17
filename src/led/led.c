/*
 * Author: Brendan Le Foll <brendan.le.foll@intel.com>
 * Contributions: Mihai Tudor Panu <mihai.tudor.panu@intel.com>
 * Contributions: Sarah Knepper <sarah.knepper@intel.com>
 *                Abhishek Malik <abhishek.malik@intel.com>
 * Copyright (c) 2016 Intel Corporation.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "led.h"
#include "upm_types.h"
#include "open62541.h"

UA_Client *client = NULL;
UA_NodeId node_id;
UA_StatusCode ua_status;

led_context led_init(uint8_t pin){
    // make sure MRAA is initialized
    int mraa_rv;
    if ((mraa_rv = mraa_init()) != MRAA_SUCCESS)
    {
        printf("%s: mraa_init() failed (%d).\n", __FUNCTION__, mraa_rv);
        return NULL;
    }

    led_context dev =
      (led_context)malloc(sizeof(struct _led_context));

    if (!dev)
      return NULL;

    dev->led_pin = pin;
    dev->gpio = mraa_gpio_init(dev->led_pin);

    if (mraa_gpio_dir(dev->gpio, MRAA_GPIO_OUT) != MRAA_SUCCESS)
      return NULL;

    /* Create open62541 client. */
    if (client == NULL) {
        client = UA_Client_new(UA_ClientConfig_standard);
        /* TODO: Don't hardcode server address here. */
        ua_status = UA_Client_connect(client, "opc.tcp://localhost:4840");
        if (ua_status != UA_STATUSCODE_GOOD) {
            printf("Could not start OPC-UA client\n");
            UA_Client_delete(client);
            client = NULL;
            goto led_exit;
        }

        /* Create the OPC UA Node associated with this device. */
        UA_VariableAttributes node_attr;
        UA_VariableAttributes_init(&node_attr);
        node_attr.displayName = UA_LOCALIZEDTEXT("en_US", "My UPM LED");
        node_attr.description = UA_LOCALIZEDTEXT("en_US", "Sensor description goes here...");

        /* NOTE: This is important - sensor data type. */
        UA_Boolean button_pressed = UA_FALSE;
        UA_Variant_setScalar(&node_attr.value, &button_pressed, &UA_TYPES[UA_TYPES_BOOLEAN]);
        node_attr.dataType = UA_TYPES[UA_TYPES_BOOLEAN].typeId;

        /* Register the node to the server. */
        ua_status = UA_Client_addVariableNode(client,
                                              UA_NODEID_NUMERIC(1, 0), // Random new node ID
                                              UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER),
                                              UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),
                                              UA_QUALIFIEDNAME(0, "LED Node"),
                                              UA_NODEID_NULL, // no variable type
                                              node_attr,
                                              &node_id);

        if (ua_status != UA_STATUSCODE_GOOD) {
            printf("Could not create OPC-UA variable node.\n");
            UA_Client_disconnect(client);
            UA_Client_delete(client);
            client = NULL;
        }
    }

led_exit:
    return dev;
}

void led_close(led_context dev){

    if (client) {
        UA_Client_deleteNode(client, node_id, UA_TRUE);
        UA_Client_disconnect(client);
        UA_Client_delete(client);
        client = NULL;
    }

    free(dev);
}

upm_result_t led_on(led_context dev){
    UA_Variant new_value;
    UA_Variant_init(&new_value);
    UA_Boolean read_value;

    if (mraa_gpio_write(dev->gpio, 1) != MRAA_SUCCESS)
        return UPM_ERROR_OPERATION_FAILED;

    read_value = UA_TRUE;

    if (client != NULL) {
        UA_Variant_setScalar(&new_value, &read_value, &UA_TYPES[UA_TYPES_BOOLEAN]);
        ua_status = UA_Client_writeValueAttribute(client, node_id, &new_value);
    }

    return UPM_SUCCESS;
}

upm_result_t led_off(led_context dev){
    UA_Variant new_value;
    UA_Variant_init(&new_value);
    UA_Boolean read_value;

    if (mraa_gpio_write(dev->gpio, 0) != MRAA_SUCCESS)
        return UPM_ERROR_OPERATION_FAILED;

    read_value = UA_FALSE;

    if (client != NULL) {
        UA_Variant_setScalar(&new_value, &read_value, &UA_TYPES[UA_TYPES_BOOLEAN]);
        ua_status = UA_Client_writeValueAttribute(client, node_id, &new_value);
    }

    return UPM_SUCCESS;
}
