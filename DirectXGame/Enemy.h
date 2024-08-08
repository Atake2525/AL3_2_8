#pragma once
#include "WorldTransform.h"
#include "ViewProjection.h"
#include "Model.h"
#include "AABB.h"

class Player;

class Enemy {
public:
	void Initialize(Model* model, ViewProjection* viewProjection, const Vector3& position);

	void Update();

	void Draw();

	// 衝突応答
	void OnCollision(const Player* player);

	WorldTransform worldTransform_;

	ViewProjection* viewProjection_ = nullptr;

	Model* model_ = nullptr;

	static inline const float kWidth = 1.0f;
	static inline const float kHeight = 1.0f;

	// 歩行の速さ
	static inline const float kWalkSpeed = 0.02f;

	// 速度
	Vector3 velocity_ = {};

	// 経過時間
	float walkTimer_ = 0.0f;

	// 最初の角度[度]
	static inline const float kWalkMotionAngleStart = 20.0f;
	// 最後の角度[度]
	static inline const float kWalkMotionAngleEnd = 45.0f;
	// アニメーションの周期となる時間[秒]
	static inline const float kWalkMotionTime = 2.0f;

	Vector3 GetWorldPosition();

	AABB GetAABB();

};
