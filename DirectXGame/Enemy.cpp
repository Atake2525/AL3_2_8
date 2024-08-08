#include "Enemy.h"
#include <cassert>
#include <cmath>
#include <numbers>
#include "MathUtilityForText.h"

void Enemy::Initialize(Model* model, ViewProjection* viewProjection, const Vector3& position){
	assert(model);

	model_ = model;

	// 速度を設定する
	velocity_ = {-kWalkSpeed, 0, 0};
	worldTransform_.Initialize();
	viewProjection_ = viewProjection;
	worldTransform_.translation_ = position;
	worldTransform_.rotation_.y = std::numbers::pi_v<float> / -2.0f;
	walkTimer_ = 0.0f;
}

void Enemy::Update() {
	// タイマーを加算
	walkTimer_ += 1.0f / 60.0f;	

	float param = std::sin(std::numbers::pi_v<float> * 2.0f * walkTimer_ / kWalkMotionTime);
	float radian = kWalkMotionAngleStart + kWalkMotionAngleEnd * (param + 1.0f) / -50.0f;
	worldTransform_.rotation_.x = radian;
	
	// 移動
	worldTransform_.translation_ += velocity_;

	
	worldTransform_.UpdateMatrix(); 
}

Vector3 Enemy::GetWorldPosition() {
	// ワールド座標を入れる変数
	Vector3 worldPos;
	// ワールド行列の平行移動成分を取得(ワールド座標)
	worldPos.x = worldTransform_.matWorld_.m[3][0];
	worldPos.y = worldTransform_.matWorld_.m[3][1];
	worldPos.z = worldTransform_.matWorld_.m[3][2];

	return worldPos;
}

AABB Enemy::GetAABB() {
	Vector3 worldPos = GetWorldPosition();

	AABB aabb;

	aabb.min = {worldPos.x - kWidth / 2.0f, worldPos.y - kHeight / 2.0f, worldPos.z - kWidth / 2.0f};
	aabb.max = {worldPos.x + kWidth / 2.0f, worldPos.y + kHeight / 2.0f, worldPos.z + kWidth / 2.0f};

	return aabb;
}

void Enemy::OnCollision(const Player* player) {
	(void)player;
}



void Enemy::Draw() { model_->Draw(worldTransform_, *viewProjection_); }
