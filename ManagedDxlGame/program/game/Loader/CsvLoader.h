#pragma once
#include "../ScenePlay/ScenePlay.h"
#include "../DxLibEngine.h"


struct EnemyZakoInfo
{
	// 本体
	int   _id{};
	float _scale{};
	int   _stageID{};
	int   _hp{};
	int   _maxTotalEnemy_spawnCount{}; // ゲーム全体で存在できる実体の最大数
	float _enemyMoveSpeed{};
	std::string _name{};

	// 弾
	int    _maxBulletSpawnCount{};
	int    _bullet_fireInterval{};
	float  _bullet_moveSpeed{};
	float  _bullet_reloadTimeInterval{};
};

struct EnemyBossInfo
{
	int    _id{};
	int    _stageID{};
	float  _scale{};
	int    _hp{};
	int    _maxBulletSpawnCount{};
	float  _enemyMoveSpeed{};
	std::string _name{};
};

struct BulletHellType_Info
{
	int   _id{};
	int   _maxBulletSpawnCount{};
	float _bulletSpawnRate{};
	std::string _typeName{};
};

struct PlayerStatus
{
	int _hp{};
	int _at{};
	int _def{};
};


class CsvLoader
{
public:

	CsvLoader() {}

	std::unordered_map<int, EnemyZakoInfo> LoadEnemyZakoInfos(const std::string enemy_csv);
	std::unordered_map<int, EnemyBossInfo> LoadEnemyBossInfos(const std::string enemy_csv);
	std::unordered_map<int, BulletHellType_Info> LoadBulletHellTypeInfos(const std::string bulletHell_csv);
	PlayerStatus& LoadPlayerStatus(const std::string status_csv);

private:

	const std::string _SELECTED_DIFFICULTY = ScenePlay::GetGameDifficulty();
};