// include sdk headers to communicate with UE3
// WARNING: this header file can currently only be included once!
//   the SDK currently throws alot of warnings which can be ignored
#pragma warning(disable:4244)
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <windows.h>
#include <vector>
#include <Proxy/Offsets.h>
#include <SdkHeaders.h>

#include <fstream>

// for access to event manager
#include <Proxy/Events.h>
// for access to client/server
#include <Proxy/Network.h>
// for module api
#include <Proxy/Modules.h>
// for logging in main file
#include <Proxy/Logger.h>
using namespace BLRevive;

#define NUM_BOT_NAMES 19

struct ServerProperties {
	// can't set this early enough for it to take effect
	// float MaxIntermissionIdle;
	std::string RandomBotNames[NUM_BOT_NAMES];
	float GameRespawnTime;
	float GameForceRespawnTime;
	float GameSpectatorSwitchDelayTime;
	int NumEnemyVotesRequiredForKick;
	int NumFriendlyVotesRequiredForKick;
	int VoteKickBanSeconds;
	float MaxIdleTime;
	bool bFriendlyFire;
};

struct ServerHacks {
	bool disableOnMatchIdle;
};

struct ServerConfig {
	ServerProperties properties;
	ServerHacks hacks;

	char RandomBotNamesFString[NUM_BOT_NAMES * sizeof(FString)];
	char RandomBotNamesFStringBackup[NUM_BOT_NAMES * sizeof(FString)];
};

struct ServerStatus {
	std::string gameType;
	std::string mapName;
	int numPlayers;
	int numBots;
	int maxPlayers;
};

static ServerConfig serverConfig;
static void applyConfigRuntime(AFoxGame *game, const ServerConfig &config);
static void applyParametersToGameDefault(const ServerConfig &config);
static void backupAndSetBotnamesToGameDefault(ServerConfig &config);
static void restoreBotnamesToGameDefault(const ServerConfig &config);
static void applyHacks(const ServerConfig &config);

static ServerConfig serverConfigFromFile();

static void logError(std::string message) {
	LError("server-utils: " + message);
	LFlush;
}

static void logDebug(std::string message) {
	LDebug("server-utils: " + message);
	LFlush;
}

static void logWarn(std::string message) {
	LWarn("server-utils: " + message);
	LFlush;
}

/// <summary>
/// Thread thats specific to the module (function must exist and export demangled!)
/// </summary>
extern "C" __declspec(dllexport) void ModuleThread()
{
	if (!Utils::IsServer()) {
		return;
	}

	serverConfig = serverConfigFromFile();

	std::shared_ptr<Events::Manager> eventManager = Events::Manager::GetInstance();
#if 1
	eventManager->RegisterHandler({
		Events::ID("*", "IsConfigFiltered"),
		[=](Events::Info info) {
			applyParametersToGameDefault(serverConfig);
			backupAndSetBotnamesToGameDefault(serverConfig);
			applyHacks(serverConfig);
		},
		true,
		true});
	logDebug("registered handler for event * IsConfigFiltered");
#endif
#if 1
	eventManager->RegisterHandler({
		Events::ID("*", "UpdateGameSettings"),
		[=](Events::Info info) {
			restoreBotnamesToGameDefault(serverConfig);
		},
		false,
		true
		});
	logDebug("registered handler for event * UpdateGameSettings");
#endif
#if 0
	eventManager->RegisterHandler({
		Events::ID("*", "*"),
		[=](Events::Info info) {
		},
		true});
	logDebug("registered handler for event * *");
#endif
}

/// <summary>
/// Module initializer (function must exist and export demangled!)
/// </summary>
/// <param name="data"></param>
extern "C" __declspec(dllexport) void InitializeModule(Module::InitData *data)
{
    // check param validity
    if (!data || !data->EventManager || !data->Logger) {
        LError("module initializer param was null!"); LFlush;
        return;
    }

    // initialize logger (to enable logging to the same file)
    Logger::Link(data->Logger);

    // initialize event manager
    // an instance of the manager can be retrieved with Events::Manager::Instance() afterwards
    Events::Manager::Link(data->EventManager);

    // initialize your module
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

static std::string getConfigPath() {
	static char *pathStr = nullptr;
	if (pathStr == nullptr) {
		std::string path = Utils::FS::BlreviveConfigPath();
		pathStr = new char[path.length() + 1];
		strcpy(pathStr, path.c_str());
		return path;
	}
	else {
		return std::string(pathStr);
	}
}

static std::string getOutputPath(){
	std::string outputPath = getConfigPath() + "server_utils/";

	struct stat info;
	if (stat(outputPath.c_str(), &info) == 0) {
		// path exists
		if (info.st_mode & S_IFDIR) {
			// path exists and is a dir
			return outputPath;
		}
		else {
			logError(std::format("{0} exists but it is not a directory", outputPath));
			return "";
		}
		}
		else {
		// path do not exist
		if (CreateDirectory(outputPath.c_str(), nullptr)) {
			return outputPath;
		}
		else {
			logError(std::format("cannot create directory {0}", outputPath));
			return "";
		}
	}
}

static void writeConfig(json &toWrite, std::string path){
	std::ofstream output(path);
	if(!output.is_open()){
		logError(std::format("failed writing config to {0}", path));
		return;
	}
	output << toWrite.dump(4) << std::endl;
	output.close();
	return;
}

static json defaultConfigJson(){
	json defaultConfig;
	// defaultConfig["properties"]["MaxIntermissionIdle"] = 3600.0;
	for(int i = 0;i < NUM_BOT_NAMES; i++){
		defaultConfig["properties"]["RandomBotNames"][i] = std::format("blrevive bot MK{0}", i);
	}
	defaultConfig["properties"]["GameRespawnTime"] = 10.0;
	defaultConfig["properties"]["GameForceRespawnTime"] = 30.0;
	defaultConfig["properties"]["GameSpectatorSwitchDelayTime"] = 120.0;
	defaultConfig["properties"]["NumEnemyVotesRequiredForKick"] = 4;
	defaultConfig["properties"]["NumFriendlyVotesRequiredForKick"] = 2;
	defaultConfig["properties"]["VoteKickBanSeconds"] = 1200;
	defaultConfig["properties"]["MaxIdleTime"] = 180.0;
	defaultConfig["properties"]["bFriendlyFire"] = 0;

	defaultConfig["hacks"]["disableOnMatchIdle"] = 1;
	return defaultConfig;
}

static ServerConfig serverConfigFromJson(json input){
	ServerConfig config;
	try{
		// config.MaxIntermissionIdle = input["properties"]["MaxIntermissionIdle"];
		for(int i = 0; i < NUM_BOT_NAMES; i++){
			config.properties.RandomBotNames[i] = input["properties"]["RandomBotNames"][i];
			FString newName(config.properties.RandomBotNames[i].c_str());
			memcpy(&(config.RandomBotNamesFString[i * sizeof(FString)]), &newName, sizeof(FString));
		}
		config.properties.GameRespawnTime = input["properties"]["GameRespawnTime"];
		config.properties.GameForceRespawnTime = input["properties"]["GameForceRespawnTime"];
		config.properties.NumEnemyVotesRequiredForKick = input["properties"]["NumEnemyVotesRequiredForKick"];
		config.properties.NumFriendlyVotesRequiredForKick = input["properties"]["NumFriendlyVotesRequiredForKick"];
		config.properties.VoteKickBanSeconds = input["properties"]["VoteKickBanSeconds"];
		config.properties.MaxIdleTime = input["properties"]["MaxIdleTime"];
		config.properties.bFriendlyFire = (int)input["properties"]["bFriendlyFire"];

		config.hacks.disableOnMatchIdle = (int)input["hacks"]["disableOnMatchIdle"];
	}catch(json::exception e){
		throw(e);
	}
	return config;
}

static ServerConfig serverConfigFromFile(){
	std::string outputPath = getOutputPath();
	if(outputPath.length() == 0){
		logError(std::format("cannot load config from {0}", outputPath));
		return serverConfigFromJson(defaultConfigJson());
	}

	std::string serverConfigPath = std::format("{0}{1}", outputPath, "server_config.json");
	std::ifstream input(serverConfigPath);
	if(!input.is_open()){
		logDebug(std::format("{0} does not exist, writing default config", serverConfigPath));
		json config = defaultConfigJson();
		writeConfig(config, serverConfigPath);
		return serverConfigFromJson(config);
	}

	try{
		json inputJson = json::parse(input);
		input.close();
		return serverConfigFromJson(inputJson);
	}catch(json::exception e){
		logError(std::format("failed parsing {0}, using default config", serverConfigPath));
		logError(e.what());
		input.close();
		return serverConfigFromJson(defaultConfigJson());
	}
}

static void applyParametersToGameDefault(const ServerConfig &config){
	AFoxGame *game = UObject::GetInstanceOf<AFoxGame>(true);
	// game->MaxIntermissionIdle = config.properties.MaxIntermissionIdle;
	game->GameRespawnTime = config.properties.GameRespawnTime;
	game->GameForceRespawnTime = config.properties.GameForceRespawnTime;
	game->NumEnemyVotesRequiredForKick = config.properties.NumEnemyVotesRequiredForKick;
	game->NumFriendlyVotesRequiredForKick = config.properties.NumFriendlyVotesRequiredForKick;
	game->VoteKickBanSeconds = config.properties.VoteKickBanSeconds;
	game->MaxIdleTime = config.properties.MaxIdleTime;
	game->bFriendlyFire = config.properties.bFriendlyFire;
}


static void backupAndSetBotnamesToGameDefault(ServerConfig &config){
	AFoxGame *game = UObject::GetInstanceOf<AFoxGame>(true);
	for(int i = 0;i < NUM_BOT_NAMES;i++){
		memcpy(&(config.RandomBotNamesFStringBackup[i * sizeof(FString)]), &(game->RandomBotNames(i)), sizeof(FString));
		memcpy(&(game->RandomBotNames(i)), &(config.RandomBotNamesFString[i * sizeof(FString)]), sizeof(FString));
	}
}

static void restoreBotnamesToGameDefault(const ServerConfig &config){
	// replacing TArray is finicky, and FStrings are TArrays, so restore it before problematic uses
	AFoxGame *game = UObject::GetInstanceOf<AFoxGame>(true);
	for(int i = 0;i < NUM_BOT_NAMES;i++){
		memcpy(&(game->RandomBotNames(i)), &(config.RandomBotNamesFStringBackup[i * sizeof(FString)]), sizeof(FString));
	}
}

static void logOnMatchIdleHack(){
	logDebug("OnMatchIdle disabled, lobby does not kick players after idling for 3 minutes without starting");
}

static void applyHacks(const ServerConfig &config){
	if(config.hacks.disableOnMatchIdle){
		// location copied from sdk
		((UFunction*) UObject::GObjObjects()->Data[ 76108 ])->Func = (void *)logOnMatchIdleHack;
	}
}
