#include "WebSocketServer.h"
#include <iostream>

WebSocketServer::WebSocketServer() : running_(false) {
    // �T�[�o�[�̏�����
    server_ = std::make_unique<ix::WebSocketServer>();
}

WebSocketServer::~WebSocketServer() {
    stop();
}

bool WebSocketServer::run(uint16_t port) {
    if (running_) {
        return false;
    }

    // �|�[�g�ݒ�
    server_->configure(port);  // ���ڃR���X�g���N�^�Őݒ肳��Ă��邽�߁A�ݒ胁�\�b�h�͕K�v�Ȃ�

    // �N���C�A���g���b�Z�[�W�R�[���o�b�N�̐ݒ�
    server_->setOnClientMessageCallback(
        [this](std::shared_ptr<ix::ConnectionState> connectionState,
            ix::WebSocket& webSocket,
            const ix::WebSocketMessagePtr& msg) {
                // ���b�Z�[�W�^�C�v�ɉ���������
                if (msg->type == ix::WebSocketMessageType::Open) {
                    // �N���C�A���g�ڑ���
                    auto webSocketPtr = std::shared_ptr<ix::WebSocket>(&webSocket, [](ix::WebSocket*) {});
                    onClientConnected(webSocketPtr);
                }
                else if (msg->type == ix::WebSocketMessageType::Message) {
                    // �e�L�X�g���b�Z�[�W����M�����ꍇ
                    if (messageCallback_) {
                        messageCallback_(msg->str);
                    }
                }
                else if (msg->type == ix::WebSocketMessageType::Close) {
                    // �ڑ�������ꂽ�ꍇ
                    auto webSocketPtr = std::shared_ptr<ix::WebSocket>(&webSocket, [](ix::WebSocket*) {});
                    std::lock_guard<std::mutex> lock(clientsMutex_);
                    clients_.erase(webSocketPtr);
                    std::cout << "Client disconnected. Total connections: " << clients_.size() << std::endl;
                }
        });

    // �T�[�o�[�N���ƃ��b�X�� (listenAndStart���\�b�h���g�p)
    if (!server_->listenAndStart()) {
        std::cerr << "Failed to start WebSocket server on port " << port << std::endl;
        return false;
    }

    running_ = true;
    std::cout << "WebSocket server started on port " << port << std::endl;
    return true;
}

void WebSocketServer::stop() {
    if (!running_) {
        return;
    }

    // �T�[�o�[��~
    server_->stop();
    running_ = false;

    // �N���C�A���g�ؒf
    {
        std::lock_guard<std::mutex> lock(clientsMutex_);
        clients_.clear();
    }

    std::cout << "WebSocket server stopped" << std::endl;
}

void WebSocketServer::broadcastMessage(const std::string& message) {
    if (!running_) {
        return;
    }

    // �T�[�o�[���炷�ׂẴN���C�A���g���擾
    auto serverClients = server_->getClients();

    for (auto& client : serverClients) {
        if (client->getReadyState() == ix::ReadyState::Open) {
            client->send(message);
        }
    }
}

void WebSocketServer::setMessageCallback(MessageCallback callback) {
    messageCallback_ = callback;
}

void WebSocketServer::onClientConnected(std::shared_ptr<ix::WebSocket> webSocket) {
    // �N���C�A���g�ڑ����̏���
    {
        std::lock_guard<std::mutex> lock(clientsMutex_);
        clients_.insert(webSocket);
    }

    std::cout << "Client connected. Total connections: " << clients_.size() << std::endl;
}