#pragma once
#include <vector>
#include <string>

// 入力フレーム1つ分の構造
struct InputFrame {
    float time;         // 入力が発生した時間（秒）
    std::string input;  // 入力内容
};

// 入力を記録するためのクラス
class InputRecorder {
private:
    // 入力記録のリスト（時系列順に保存）
    std::vector<InputFrame> recordedInputs_;

public:
    // 入力を記録する関数
    void Record(float time, const char* keys);

    // JSONファイルに記録内容を保存
    void SaveToFile(const std::string& filename);

    // JSONファイルから記録を読み込む
    void LoadFromFile(const std::string& filename);

    // 記録されたデータを取得（再生用）
    const std::vector<InputFrame>& GetData() const;

    void Clear();
};
