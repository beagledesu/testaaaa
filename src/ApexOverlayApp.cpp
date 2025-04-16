#include "ApexOverlayApp.h"
#include <iostream>
#include <fstream>
#include <filesystem>

ApexOverlayApp::ApexOverlayApp() : isRunning_(false), serverPort_(8080), apexServerUri_("ws://127.0.0.1:7777") {
}

ApexOverlayApp::~ApexOverlayApp() {
    shutdown();
}

bool ApexOverlayApp::initialize() {
    // IXWebSocket初期化
    ix::initNetSystem();

    // 設定を読み込み
    if (!loadConfig()) {
        std::cerr << "Failed to load configuration" << std::endl;
        return false;
    }
    
    // Apexからのメッセージ受信コールバックを設定
    apexLiveApiClient_.setMessageCallback([this](const std::string& message) {
        onApexMessage(message);
    });
    
    // クライアントからのメッセージ受信コールバックを設定
    webSocketServer_.setMessageCallback([this](const std::string& message) {
        onClientMessage(message);
    });
    
    return true;
}

void ApexOverlayApp::run() {
    isRunning_ = true;
    
    // WebSocketサーバーを起動
    if (!webSocketServer_.run(serverPort_)) {
        std::cerr << "Failed to start WebSocket server" << std::endl;
        isRunning_ = false;
        return;
    }
    
    std::cout << "Apex Overlay application started" << std::endl;
    std::cout << "WebSocket server running on port " << serverPort_ << std::endl;
    
    // Apex LiveAPIに接続
    if (!apexLiveApiClient_.connect(apexServerUri_)) {
        std::cerr << "Failed to connect to Apex LiveAPI" << std::endl;
    }
    
    // メインスレッドでコンソールコマンドを待機
    std::string command;
    while (isRunning_) {
        std::cout << "> ";
        std::getline(std::cin, command);
        
        if (command == "exit" || command == "quit") {
            isRunning_ = false;
        } else if (command == "status") {
            std::cout << "Server is running on port " << serverPort_ << std::endl;
        }
    }
    
    shutdown();
}

void ApexOverlayApp::shutdown() {
    if (isRunning_) {
        isRunning_ = false;
        
        // Apex LiveAPIから切断
        apexLiveApiClient_.disconnect();
        
        // WebSocketサーバーを停止
        webSocketServer_.stop();
        
        // IXWebSocketのシャットダウン
        ix::uninitNetSystem();
        
        std::cout << "Apex Overlay application stopped" << std::endl;
    }
}

bool ApexOverlayApp::loadConfig() {
    try {
        // 設定ファイルを開く
        std::ifstream file("config/config.json");
        if (!file.is_open()) {
            // デフォルト設定ファイルを作成
            std::filesystem::create_directories("config");
            std::ofstream defaultFile("config/config.json");
            if (defaultFile.is_open()) {
                json defaultConfig = {
                    {"server", {
                        {"port", serverPort_}
                    }},
                    {"apex", {
                        {"uri", apexServerUri_}
                    }}
                };
                defaultFile << defaultConfig.dump(4);
                defaultFile.close();
                std::cout << "Created default configuration file" << std::endl;
                return true;
            } else {
                std::cerr << "Failed to create default configuration file" << std::endl;
                return false;
            }
        }
        
        // 設定をJSONとして解析
        json config;
        file >> config;
        
        // 設定を適用
        if (config.contains("server") && config["server"].contains("port")) {
            serverPort_ = config["server"]["port"];
        }
        
        if (config.contains("apex") && config["apex"].contains("uri")) {
            apexServerUri_ = config["apex"]["uri"];
        }
        
        std::cout << "Configuration loaded" << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error loading configuration: " << e.what() << std::endl;
        return false;
    }
}

void ApexOverlayApp::onApexMessage(const std::string& message) {
    try {
        // Apexからのメッセージを解析（オプション）
        json data = json::parse(message);
        
        // データを必要に応じて処理
        
        // 全クライアントにデータを転送
        webSocketServer_.broadcastMessage(message);
    } catch (const std::exception& e) {
        std::cerr << "Error processing Apex message: " << e.what() << std::endl;
    }
}

void ApexOverlayApp::onClientMessage(const std::string& message) {
    try {
        // クライアントからのメッセージを解析
        json data = json::parse(message);
        
        // コマンドを処理（例：設定変更）
        if (data.contains("command")) {
            std::string command = data["command"];
            
            if (command == "settings") {
                // 設定を返す
                json settingsResponse = {
                    {"type", "settings"},
                    {"server", {
                        {"port", serverPort_}
                    }},
                    {"apex", {
                        {"uri", apexServerUri_}
                    }}
                };
                webSocketServer_.broadcastMessage(settingsResponse.dump());
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error processing client message: " << e.what() << std::endl;
    }
}