#include <client/Client.h>
#include <common/Errors.h>
#include <common/Logger.h>

#include <unistd.h>


int main(int argc, char *argv[])
{
#ifdef ENABLE_LOGGING
    Logger::GetInstance().SetOutputToFile(true, Logger::RULE_ALL, fmt::format("clientdbg_{}", getpid()));
    Logger::GetInstance().SetOutputToStdout(true, Logger::RULE_ALL);
#else
    Logger::GetInstance().SetOutputToStdout(true, Logger::RULE_MESSAGE | Logger::RULE_ERROR);
#endif

    ECode ret;
    Peer server;

	if (argc < 4) {
        LOG_ERROR("Usage: {} <client_id> <ip> <port>", argv[0]);
        return EXIT_FAILURE;
	}

    server.client_id = argv[1];
    server.ip = IP(argv[2]);
    server.port = atoi(argv[3]);

    if (!server.IsValid()) {
        LOG_ERROR("Invalid parameters specified.");
        return EXIT_FAILURE;
    }


    Client client(server);
    ret = client.Init();
    if (ret != ECode::OK) {
        LOG_ERROR("Can't init the client, errcode: {}", ret);
        return EXIT_FAILURE;
    }

    ret = client.Run();
    if (ret != ECode::OK) {
        LOG_ERROR("An error occured while running the client, errcode: {}", ret);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
