#define NOMINMAX
#include "Player.h"
#include <numbers>
#include "TextureManager.h"
#include <cassert>
#include <algorithm>
#include "MathUtilityForText.h"
#include "Input.h"
#include "MapChipField.h"
#include "DebugText.h"

void Player::Initialize(Model* model, uint32_t textureHandle, ViewProjection* viewProjection, const Vector3& position) {
	assert(model);

	model_ = model;
	textureHandle_ = textureHandle;

	worldTransform_.Initialize();
	viewProjection_ = viewProjection;
	worldTransform_.translation_ = position;
	worldTransform_.rotation_.y = std::numbers::pi_v<float> / 2.0f;

	/* worldTransform_.translation_.x = 2;
	worldTransform_.translation_.y = 2;*/
};

void Player::MapCollisionUpside(CollisionMapInfo& collisionMapInfo) {

	// 上昇あり？
	if (collisionMapInfo.MovePoint.y <= 0) {
		return;
	}

	// 衝突判定上方向
	// 移動後の4つの角の座標
	std::array<Vector3, kNumCorner> positionsNew;

	for (uint32_t i = 0; i < positionsNew.size(); i++) {
		positionsNew[i] = CornerPosition(worldTransform_.translation_ + collisionMapInfo.MovePoint, static_cast<Corner>(i));
	}
	MapChipType mapChipType;
	// 真上の当たり判定を行う
	bool hit = false;
	// 左上点の判定
	MapChipField::IndexSet indexSet;
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftTop]);
	mapChipType = mapChipField_->GetMapChiptypeByIndex(indexSet.xIndex, indexSet.yIndex);
	if (mapChipType == MapChipType::KBlock) {
		hit = true;
	}
	// 右上点の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightTop]);
	mapChipType = mapChipField_->GetMapChiptypeByIndex(indexSet.xIndex, indexSet.yIndex);
	if (mapChipType == MapChipType::KBlock) {
		hit = true;
	}
	// ブロックにヒット？
	if (hit) {
		// めり込みを排除する方向に移動量を設定する
		indexSet = mapChipField_->GetMapChipIndexSetByPosition(worldTransform_.translation_ + Vector3(0, +kHeight / 2.0f, 0));
		// めり込み先ブロックの範囲矩形
		MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
		collisionMapInfo.MovePoint.y = std::max(0.0f, rect.bottom - worldTransform_.translation_.y - (kHeight / 2.0f + kBlank));
		// 天井に当たったことを記録する
		collisionMapInfo.isHeadColFlag = true;
	}
}

// マップ衝突判定下方向
void Player::MapCollisionUnderside(CollisionMapInfo& info) {
	// 下降あり？
	if (info.MovePoint.y >= 0) {
		return;
	}

	MapChipType mapChipType;
	// 真下の当たり判定を行う
	bool hit = false;

	// 移動後の4つの角の座標
	std::array<Vector3, kNumCorner> positionsNew;

	for (uint32_t i = 0; i < positionsNew.size(); i++) {
		positionsNew[i] = CornerPosition(worldTransform_.translation_ + info.MovePoint, static_cast<Corner>(i));
	}
	// 左下点の判定
	MapChipField::IndexSet indexSet;
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftBottom]);
	mapChipType = mapChipField_->GetMapChiptypeByIndex(indexSet.xIndex, indexSet.yIndex);
	if (mapChipType == MapChipType::KBlock) {
		hit = true;
	}
	// 右下点の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightBottom]);
	mapChipType = mapChipField_->GetMapChiptypeByIndex(indexSet.xIndex, indexSet.yIndex);
	if (mapChipType == MapChipType::KBlock) {
		hit = true;
	}

	// ブロックにヒット？
	if (hit) {
		// めり込みを排除する方向に移動量を設定する
		indexSet = mapChipField_->GetMapChipIndexSetByPosition(worldTransform_.translation_ + Vector3(0, +kHeight * 2.0f, 0));
		// めり込み先ブロックの範囲矩形
		MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
		info.MovePoint.y = std::min(0.0f, rect.bottom - worldTransform_.translation_.y + (kHeight / 2.0f + kBlank));
		// 地面に当たったことを記録する
		info.landing = true;
	}
}


// 判定結果を反映して移動させる
void Player::ReturnMove(const CollisionMapInfo& info){
	// 移動
	worldTransform_.translation_ += info.MovePoint;
};

void Player::isOverheadCollision(const CollisionMapInfo& info) {
	// 天井に当たった？
	if (info.isHeadColFlag) {
		DebugText::GetInstance()->ConsolePrintf("hit ceiling\n");
		velocity_.y = 0;
	}
}

Vector3 Player::CornerPosition(const Vector3& center, Corner corner) {
	Vector3 offsetTable[kNumCorner] = {
	    {+kWidth / 2.0f, -kHeight / 2.0f, 0}, // kRightBottom
	    {-kWidth / 2.0f, -kHeight / 2.0f, 0}, // kLeftBottom
	    {+kWidth / 2.0f, +kHeight / 2.0f, 0}, // kRightTop
	    {-kWidth / 2.0f, +kHeight / 2.0f, 0}  // kLeftTop
	};
	return center + offsetTable[static_cast<uint32_t>(corner)];
}

void Player::Move(){
	///  移動入力
	///
	// 接地状態
	CollisionMapInfo info;
	if (onGround_) {
		// 左右移動操作
		if (Input::GetInstance()->PushKey(DIK_D) || Input::GetInstance()->PushKey(DIK_A)) {
			Vector3 acceleration = {};

			if (Input::GetInstance()->PushKey(DIK_D)) {
				acceleration.x += kAcceleration;
				if (lrDirection_ != LRDirection::kRight) {
					lrDirection_ = LRDirection::kRight;
					turnFirstRotationY_ = worldTransform_.rotation_.y;
					turnTimer_ = 0.01f;
				}
				if (velocity_.x < 0.0f) {
					// 速度と逆方向に入力中は急ブレーキ
					velocity_.x *= (1.0f - kAttenuation);
				}
			} else if (Input::GetInstance()->PushKey(DIK_A)) {
				acceleration.x -= kAcceleration;
				if (lrDirection_ != LRDirection::kLeft) {
					lrDirection_ = LRDirection::kLeft;
					turnFirstRotationY_ = worldTransform_.rotation_.y;
					turnTimer_ = 0.01f;
				}
				if (velocity_.x > 0.0f) {
					// 速度と逆方向に入力中は急ブレーキ
					velocity_.x *= (1.0f - kAttenuation);
				}
			}
			// 加速 // 減速
			velocity_.x += acceleration.x;
			// 最大速度制限
			velocity_.x = std::clamp(velocity_.x, -kLimitRunSpeed, kLimitRunSpeed);
		} else {
			// 非入力時は移動減衰をかける
			velocity_.x *= (1.0f - kAttenuation);
		}
		if (turnTimer_ > 0.0f) {
			turnTimer_ += kTimeTurn / 60;

			// 左右の自キャラ角度テーブル
			float destinationRotationYTable[] = {std::numbers::pi_v<float> / 2.0f, std::numbers::pi_v<float> * 3.0f / 2.0f};
			// 状態に応じた角度を取得する
			float destinationRotationY = destinationRotationYTable[static_cast<uint32_t>(lrDirection_)];
			// 自キャラの角度を設定する
			worldTransform_.rotation_.y = easeInOut(turnTimer_, turnFirstRotationY_, destinationRotationY);
		}
		if (turnTimer_ > 1.0f) {
			turnTimer_ = 0.0f;
		}

		if (Input::GetInstance()->PushKey(DIK_SPACE)) {
			Vector3 jumpAcceleration = {0, kJumpAcceleration, 0};
			// ジャンプ初速
			velocity_ += jumpAcceleration;
		}
		// ジャンプ開始
		if (velocity_.y > 0.0f) {
			// 空中状態に移行
			onGround_ = false;
		}
	} else {
		// 空中
		// 落下速度
		velocity_ += Vector3(0, -kGravityAcceleration, 0);
		// 落下速度制限
		velocity_.y = std::max(velocity_.y, -kLimitFallSpeed);
	}
};

void Player::changeOnGround(const CollisionMapInfo& info) {
	//info.landing = false;

	//// 地面との当たり判定
	//// 落下中か
	//if (velocity_.y < 0) {
	//	// Y座標が地面以下になったら着地
	//	if (worldTransform_.translation_.y <= 2.0f) {
	//		info.landing = true;
	//	}
	//}
	if (onGround_) {
		// 接地状態の処理
		// ジャンプ開始
		if (velocity_.y > 0.0f) {
			onGround_ = false;
		} else {
			// 落下判定

			// 落下なら空中状態に切り替え
			// 移動後の4つの角の座標
			std::array<Vector3, kNumCorner> positionsNew;

			for (uint32_t i = 0; i < positionsNew.size(); i++) {
				positionsNew[i] = CornerPosition(worldTransform_.translation_ + info.MovePoint, static_cast<Corner>(i));
			}
			MapChipType mapChipType;

			// 真下の当たり判定を行う
			bool hit = false;
			// 左下点の判定
			MapChipField::IndexSet indexSet;
			indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftBottom] /* + Vector3(0, -smallNum, 0)*/);
			mapChipType = mapChipField_->GetMapChiptypeByIndex(indexSet.xIndex, indexSet.yIndex);
			if (mapChipType == MapChipType::KBlock) {
				hit = true;
			}
			// 右下点の判定
			indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightBottom] /* + Vector3(0, -smallNum, 0) */);
			mapChipType = mapChipField_->GetMapChiptypeByIndex(indexSet.xIndex, indexSet.yIndex);
			if (mapChipType == MapChipType::KBlock) {
				hit = true;
			}

			

			// 落下開始
			if (!hit) {
				// 空中状態に切り替える
				onGround_ = false;
			}
		}
	} else {
		// 空中状態の処理
		if (info.landing) {
			// 着地状態に切り替える(落下を止める)
			onGround_ = true;
			// 着地時にX速度を減衰
			velocity_.x *= (1.0f - kAttenuationLoading);
			// Y速度をゼロにする
			velocity_.y = 0.0f;
		}
	}
}

void Player::Update() { 
	// 1移動入力
	Move();

	// 2 移動量を加味して衝突判定する

	// 3衝突情報を初期化
	CollisionMapInfo collisionMapInfo;
	// 移動量に速度の値をコピー
	collisionMapInfo.MovePoint = velocity_;
	// マップ衝突チェック
	// 4天井に接触している場合の処理
	isOverheadCollision(collisionMapInfo);
	// 判定結果を判定して移動させる
	ReturnMove(collisionMapInfo);

	MapCollisionUpside(collisionMapInfo);
	MapCollisionUnderside(collisionMapInfo);

	changeOnGround(collisionMapInfo);



	// 6接地状態の切り替え
	
	
	///
	

	




	// 8行列計算
	worldTransform_.UpdateMatrix();

};

void Player::Draw(Model* model) { model->Draw(worldTransform_, *viewProjection_); };

