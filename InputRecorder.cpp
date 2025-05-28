#include "InputRecorder.h"
#include <Novice.h> 
#include <iostream> // デバッグ用
#include <fstream>                  // ファイル読み込み用
#include "externals/json.hpp"        // JSONライブラリ

using json = nlohmann::json;

// 入力を記録する関数
void InputRecorder::Record(float time, const char* keys) {
    // キーが押されたかをチェック
    if (keys[DIK_LEFT]) {
        recordedInputs_.push_back({ time, "Left" });
    }
    if (keys[DIK_RIGHT]) {
        recordedInputs_.push_back({ time, "Right" });
    }
    if (keys[DIK_SPACE]) {
        recordedInputs_.push_back({ time, "Jump" });
    }

	// デバッグ用にコンソールに出力
    std::cout << "Time: " << time << ", Input: Jump/Left/Right..." << std::endl;
}

// JSONファイルに保存
void InputRecorder::SaveToFile(const std::string& filename) {
    json j;  // 空のJSON配列

    // 入力フレームを順番にJSONに変換して追加
    for (const auto& input : recordedInputs_) {
        j.push_back({
            { "time", input.time },
            { "input", input.input }
            });
    }

    // ファイルへ書き出し
    std::ofstream file(filename);
    if (file.is_open()) {
        file << j.dump(4);  // インデント4で読みやすく
        file.close();
    }
}

// JSONファイルから入力データを読み込む
void InputRecorder::LoadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) return;

    json j;
    file >> j;
    file.close();

    recordedInputs_.clear();  // ← ここで一度削除（重要）

    for (const auto& item : j) {
        InputFrame input;
        input.time = item["time"];
        input.input = item["input"];
        recordedInputs_.push_back(input);
    }
}

// 外から記録済みデータを取得
const std::vector<InputFrame>& InputRecorder::GetData() const {
    return recordedInputs_;
}

void InputRecorder::Clear() {
    recordedInputs_.clear();  // データを全消去
}
