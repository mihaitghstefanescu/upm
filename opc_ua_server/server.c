#include <signal.h>
#include "open62541.h"

UA_Boolean running = true;

void sig_handler(int signo)
{
    running = false;
}

int main(int argc, char** argv)
{
    signal(SIGINT, sig_handler);

    /* Server config and creation. */
    UA_ServerConfig config = UA_ServerConfig_standard;
    UA_ServerNetworkLayer nl = UA_ServerNetworkLayerTCP(UA_ConnectionConfig_standard, 4840);
    config.networkLayers = &nl;
    config.networkLayersSize = 1;
    UA_Server *server = UA_Server_new(config);

    /* Run the server main loop. */
    UA_StatusCode status = UA_Server_run(server, &running);

    /* Cleanup. */
    UA_Server_delete(server);
    nl.deleteMembers(&nl);

    return status;
}