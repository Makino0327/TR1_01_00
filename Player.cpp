#include "Player.h"
#include <Novice.h>


Player::Player() {
    position_.x = 0.0f;
    position_.y = 520.0f;
    speed_ = 5.0f;

    velocityY_ = 0.0f;
    gravity_ = 0.5f;
    isJumping_ = false;
    groundY_ = 456.0f;
}

void Player::Reset() {
    position_.x = 0.0f;
    position_.y = 520.0f;
    velocityY_ = 0.0f;
    isJumping_ = false;
}

void Player::Update(const char* keys, bool isReplay) {
    if (keys[DIK_LEFT]) {
        position_.x -= speed_;
    }
    if (keys[DIK_RIGHT] ) {
        position_.x += speed_;
    }

    if (isReplay) {
        replayTrail_.push_back(position_);
        if (replayTrail_.size() > 1000) {
            replayTrail_.erase(replayTrail_.begin());
        }
    } else {
        trail_.push_back(position_);
        if (trail_.size() > 1000) {
            trail_.erase(trail_.begin());
        }
    }


    // ジャンプ
    if (!isJumping_ && keys[DIK_SPACE]) {
        velocityY_ = -10.0f; // 上方向にジャンプ
        isJumping_ = true;
    }

    // 重力適用
    velocityY_ += gravity_;
    position_.y += velocityY_;

    // 地面に着地
    if (position_.y >= groundY_) {
        position_.y = groundY_;
        velocityY_ = 0.0f;
        isJumping_ = false;
    }
}

void Player::ClearTrail() {
    trail_.clear();
    replayTrail_.clear();
}



void Player::Draw(){
    Novice::DrawBox((int)position_.x, (int)position_.y, 64, 64, 0.0f, 0xFFFFFFFF, kFillModeSolid);
}

Vector2 Player::GetPosition() const {
    return position_;
}

Player::~Player() {

}

const std::vector<Vector2>& Player::GetTrail() const {
    return trail_;
}

const std::vector<Vector2>& Player::GetReplayTrail() const {
    return replayTrail_;
}