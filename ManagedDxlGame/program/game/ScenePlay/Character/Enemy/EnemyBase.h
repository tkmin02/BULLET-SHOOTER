#pragma once
#include "../Character.h"

namespace inl {

	class EnemyBase : public Character
	{
	public:

		// コンストラクタ・デストラクタ------------------------------------
		EnemyBase() {}
		~EnemyBase() override {}

		// ------------------------------------------------------------------
		float GetDistanceToPlayer();

		// 形状、テクスチャ、ポジション、スケール
		virtual void SetMeshInfo() {}

		virtual bool Update(const float deltaTime) { return true; }

		virtual void Render(Shared<dxe::Camera> camera) {}

	protected:

		// 待機、追跡、攻撃などのパターンを管理し実行
		virtual void DoRoutineMoves(const float& deltaTime) {}

		virtual void ChasePlayer(const float deltaTime) {}

		virtual void AttackPlayer(const float& deltaTime) {}

		void LookAtPlayer();

		tnl::Vector3 GetRandomPosition_Mt19337() const;

	protected:

		Shared<dxe::Camera>  _enemyCamera = nullptr;

	public:

		bool           _isDead = false; //敵単体の死亡フラグ

		// CSVからロード-----------------------

		int            _id{};
		float          _enemyMoveSpeed{};
		float          _scale{};
		std::string    _name{};
		int            _maxBulletSpawnCount{};

	private:

		const float _RANDOM_SPAWN_RANGE_RANGE_X = 800.0f;
		const float _RANDOM_SPAWN_RANGE_RANGE_Y = 100.0f;
		const float _RANDOM_SPAWN_RANGE_RANGE_Z = 500.0f;
	};
}