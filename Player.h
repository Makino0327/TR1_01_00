#pragma once
#include "Vector2.h"
#include <vector>
class Player
{
private:
    Vector2 position_;
    float speed_;

    // ジャンプ関連
    float velocityY_;
    float gravity_;
    bool isJumping_;
    float groundY_;
    std::vector<Vector2> trail_; // ← 軌跡を保存
    std::vector<Vector2> replayTrail_;  // 再生中の軌跡

public:
    Player();
    ~Player();
    void Update(const char* keys, bool isReplay);
    void Draw();
    void Reset();
    Vector2 GetPosition() const;
    void ClearTrail();  // 軌跡だけをリセットする関数（Resetとは別）

    const std::vector<Vector2>& GetTrail() const; // 軌跡取得用
    const std::vector<Vector2>& GetReplayTrail() const;
};

