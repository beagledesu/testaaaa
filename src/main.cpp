#include "ApexOverlayApp.h"
#include <iostream>

int main() {
    try {
        // アプリケーションのインスタンスを作成
        ApexOverlayApp app;
        
        // 初期化
        if (!app.initialize()) {
            std::cerr << "Failed to initialize application" << std::endl;
            return 1;
        }
        
        // アプリケーションを実行
        app.run();
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}