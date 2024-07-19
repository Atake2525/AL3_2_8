#pragma once
#include "Model.h"
#include "WorldTransform.h"

enum class MapChipType {
	kBlank, // 空白
	KBlock, // ブロック
};

struct MapChipData {
	std::vector<std::vector<MapChipType>> data;
};



//std::unordered_map<std::string, MapChipType> mapChipTable = {
//    {"0", MapChipType::kBlank},
//    {"1", MapChipType::KBlock},
//};

class MapChipField {
public:
	struct IndexSet {
		uint32_t xIndex;
		uint32_t yIndex;
	};
	struct Rect {
		float left;   // 左端
		float right;  // 右端
		float bottom; // 下端
		float top;    // 上端
	};

public:
	// 1ブロックのサイズ
	static inline const float kBlockWidth = 1.0f;
	static inline const float kBlockHeight = 1.0f;
	// ブロックの個数
	static inline const uint32_t  kNumBlockVirtical = 20;
	static inline const uint32_t kNumBlockHorizontal = 100;

	const uint32_t GetNumBlockVirtical() { return kNumBlockVirtical; }

	const uint32_t GetNumBlockHorizontal() { return kNumBlockHorizontal; }



	// モデル
	Model* model_ = nullptr;

	// テクスチャハンドル
	uint32_t textureHandle_ = 0u;

	MapChipData mapChipData_;

	void ResetMapChipData();

	void LoadMapChipCsv(const std::string& filePath);

	MapChipType GetMapChiptypeByIndex(uint32_t xIndex, uint32_t yIndex);

	Vector3 GetMapChipPositionByIndex(uint32_t xIndex, uint32_t yIndex);

	IndexSet GetMapChipIndexSetByPosition(const Vector3& position);

	Rect GetRectByIndex(uint32_t xIndex, uint32_t yIndex);
	
};
