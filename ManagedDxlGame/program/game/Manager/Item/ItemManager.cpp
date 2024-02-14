#include <random>
#include "../../Manager/Enemy/EnemyManager.h"
#include "../../ScenePlay/Character/Enemy/EnemyZakoBase.h"
#include "../../ScenePlay/Collision/Collision.h"
#include "../Score/ScoreManager.h"
#include "../../ScenePlay/Item/PowerUpItem.h"
#include "../../ScenePlay/Character/Player/Player.h"
#include "../../ScenePlay/EventMessage/EventNoticeText.h"
#include "ItemManager.h"


// 得点アイテム--------------------------------------------------------------------------------------------------------------------------
std::vector<Shared<ScoreItem>> ItemManager::_scoreItem_small;
std::vector<Shared<ScoreItem>> ItemManager::_scoreItem_medium;
std::vector<Shared<ScoreItem>> ItemManager::_scoreItem_large;

void ItemManager::CreateScoreItemPool(const std::string difficulty, const int stage_id) {

	struct ItemSize {
		int s, m, l;
	};

	// Easyを基準とした各得点アイテムの総数
	int s_cnt = 10;
	int m_cnt = 5;
	int l_cnt = 1;

	std::map<std::string, std::vector<ItemSize>> itemCountSetting
	{
		// 各ステージごとに{}で分け、{}の中で s,m,l を設定
		{"Easy",
		{{s_cnt,    m_cnt, l_cnt},               //stage1 
		{s_cnt,     m_cnt, l_cnt},                //stage2 
		{s_cnt,     m_cnt, l_cnt},}},             //stage3
		{"Normal",
		{{s_cnt - 2,m_cnt - 1, l_cnt * 2},   //stage1
		{s_cnt - 2, m_cnt, l_cnt * 2},        //stage2
		{s_cnt - 2, m_cnt, l_cnt * 2}}},      //stage3
		{"Hard",
		{{s_cnt - 4,m_cnt - 2, l_cnt * 3},   //stage1
		{s_cnt - 4, m_cnt - 2, l_cnt * 3},	  //stage2
		{s_cnt - 4, m_cnt - 2, l_cnt * 3}}},  //stage3
		{"Lunatic",
		{{s_cnt - 6,m_cnt - 3, l_cnt * 4},   //stage1
		{s_cnt - 6, m_cnt - 3, l_cnt * 4},	  //stage2
		{s_cnt - 6, m_cnt - 3, l_cnt * 4}}}	  //stage3
	};

	if (itemCountSetting.count(difficulty) > 0 && stage_id >= 1 && stage_id <= 3) {

		ItemSize& size = itemCountSetting[difficulty][stage_id - 1];

		for (int i = 0; i < size.s; i++) {
			Shared<ScoreItem> item = std::make_shared<ScoreItem>(ScoreItem::TYPE::Small);
			_scoreItem_small.push_back(item);
		}
		for (int i = 0; i < size.m; i++) {
			Shared<ScoreItem> item = std::make_shared<ScoreItem>(ScoreItem::TYPE::Medium);
			_scoreItem_medium.push_back(item);
		}
		for (int i = 0; i < size.l; i++) {
			Shared<ScoreItem> item = std::make_shared<ScoreItem>(ScoreItem::TYPE::Large);
			_scoreItem_large.push_back(item);
		}
	}
}



void ItemManager::SetScoreItem_CollisionPairLists_DRY(std::vector<Shared<ScoreItem>>& scoreItems, const ScoreItem::TYPE type) {

	for (auto it : scoreItems) {
		if (it->_mesh != nullptr && it->_isActive) {
			if (_collision_ref->CheckCollision_PlayerAndScoreItem(it, _player_ref)) {

				switch (type)
				{
				case ScoreItem::TYPE::Small:
				{
					OnCaughtItem("得点アイテム小", "500ポイント加算。");
					ScoreManager::GetInstance().AddScoreItemScore(500);
					break;
				}
				case ScoreItem::TYPE::Medium:
				{
					OnCaughtItem("得点アイテム中", "1000ポイント加算。");
					ScoreManager::GetInstance().AddScoreItemScore(100);
					break;
				}
				case ScoreItem::TYPE::Large:
				{
					OnCaughtItem("得点アイテム大", "1500ポイント加算。");
					ScoreManager::GetInstance().AddScoreItemScore(1500);
					break;
				}
				}

				it->_isActive = false;
			}
		}
	}
}

void ItemManager::SetScoreItem_CollisionPairLists()
{
	SetScoreItem_CollisionPairLists_DRY(_scoreItem_small, ScoreItem::TYPE::Small);
	SetScoreItem_CollisionPairLists_DRY(_scoreItem_medium, ScoreItem::TYPE::Medium);
	SetScoreItem_CollisionPairLists_DRY(_scoreItem_large, ScoreItem::TYPE::Large);
}


void ItemManager::UpdateScoreItem_DRY(std::vector<Shared<ScoreItem>>& scoreItems) {

	if (!scoreItems.empty()) {
		for (auto it_scrItm = scoreItems.begin(); it_scrItm != scoreItems.end();) {
			if ((*it_scrItm)->Update((*it_scrItm)) == false)
				it_scrItm = scoreItems.erase(it_scrItm);
			else
				it_scrItm++;
		}
	}
}
void ItemManager::UpdateScoreItem()
{
	UpdateScoreItem_DRY(_scoreItem_small);
	UpdateScoreItem_DRY(_scoreItem_medium);
	UpdateScoreItem_DRY(_scoreItem_large);
}

// プレイヤー強化アイテム------------------------------------------------------------------------------------------------------------------------------------
std::vector<Shared<PowerUpItem>> ItemManager::_powerUpItem_heal;
std::vector<Shared<PowerUpItem>> ItemManager::_powerUpItem_attack;
std::vector<Shared<PowerUpItem>> ItemManager::_powerUpItem_defense;
std::vector<Shared<PowerUpItem>> ItemManager::_powerUpItem_speed;
std::vector<Shared<PowerUpItem>> ItemManager::_powerUpItem_bomb;


void ItemManager::CreatePowerUpItemPool(const std::string difficulty, const int stage_id) {

	struct ItemType {
		int heal, attack, defense, speed, bomb;
	};

	// Easyを基準とした各プレイヤー強化アイテムの総数
	int heal_cnt = 10;
	int attack_cnt = 5;
	int defense_cnt = 1;
	int speed_cnt = 10;
	int bomb_cnt = 5;


	std::map<std::string, std::vector<ItemType>> itemCountSetting
	{
		// 各ステージごとに{}で分け、{}の中で s,m,l を設定
		{"Easy",
		{{heal_cnt, attack_cnt, defense_cnt, speed_cnt, bomb_cnt},     //stage1 
		{heal_cnt,  attack_cnt, defense_cnt, speed_cnt, bomb_cnt},      //stage2 
		{heal_cnt,  attack_cnt, defense_cnt, speed_cnt, bomb_cnt}, }},   //stage3
		{"Normal",
		{{heal_cnt, attack_cnt, defense_cnt, speed_cnt, bomb_cnt},     //stage1 
		{heal_cnt,  attack_cnt, defense_cnt, speed_cnt, bomb_cnt},      //stage2 
		{heal_cnt,  attack_cnt, defense_cnt, speed_cnt, bomb_cnt}, }},   //stage3
		{"Hard",
		{{heal_cnt, attack_cnt, defense_cnt, speed_cnt, bomb_cnt},     //stage1 
		{heal_cnt,  attack_cnt, defense_cnt, speed_cnt, bomb_cnt},      //stage2 
		{heal_cnt,  attack_cnt, defense_cnt, speed_cnt, bomb_cnt}, }},   //stage3
		{"Lunatic",
		{{heal_cnt, attack_cnt, defense_cnt, speed_cnt, bomb_cnt},     //stage1 
		{heal_cnt,  attack_cnt, defense_cnt, speed_cnt, bomb_cnt},      //stage2 
		{heal_cnt,  attack_cnt, defense_cnt, speed_cnt, bomb_cnt}, }},   //stage3
	};

	if (itemCountSetting.count(difficulty) > 0 && stage_id >= 1 && stage_id <= 3) {

		ItemType& type = itemCountSetting[difficulty][stage_id - 1];

		for (int i = 0; i < type.heal; i++) {
			Shared<PowerUpItem> item = std::make_shared<PowerUpItem>(PowerUpItem::TYPE::Heal);
			_powerUpItem_heal.push_back(item);
		}
		for (int i = 0; i < type.attack; i++) {
			Shared<PowerUpItem> item = std::make_shared<PowerUpItem>(PowerUpItem::TYPE::Attack);
			_powerUpItem_attack.push_back(item);
		}
		for (int i = 0; i < type.defense; i++) {
			Shared<PowerUpItem> item = std::make_shared<PowerUpItem>(PowerUpItem::TYPE::Defense);
			_powerUpItem_defense.push_back(item);
		}
		for (int i = 0; i < type.speed; i++) {
			Shared<PowerUpItem> item = std::make_shared<PowerUpItem>(PowerUpItem::TYPE::Speed);
			_powerUpItem_speed.push_back(item);
		}
		for (int i = 0; i < type.bomb; i++) {
			Shared<PowerUpItem> item = std::make_shared<PowerUpItem>(PowerUpItem::TYPE::Bomb);
			_powerUpItem_bomb.push_back(item);
		}
	}
}


void ItemManager::SetPowerUpItem_CollisionPairLists_DRY(std::vector<Shared<PowerUpItem>>& powerUpItems) {

	for (auto it : powerUpItems) {
		if (it->_mesh != nullptr && it->_isActive) {
			if (_collision_ref->CheckCollision_PlayerAndPowerUpItem(it, _player_ref)) {

				switch (it->type)
				{
				case PowerUpItem::TYPE::Heal:
				{
					if (Player::_hp < Player::_MAX_HP) {

						OnCaughtItem("回復アイテム", "HPが上昇。");

						Player::_hp += 10;

						if (Player::_hp > Player::_MAX_HP)
							Player::_hp = Player::_MAX_HP;
						break;

					}
				}
				case PowerUpItem::TYPE::Attack:
				{
					OnCaughtItem("攻撃強化アイテム", "攻撃力が上昇。");

					Player::_at += 2;
					break;
				}
				case PowerUpItem::TYPE::Defense:
				{
					OnCaughtItem("防御強化アイテム", "防御力が上昇。");

					Player::_def += 2;
					break;
				}
				case PowerUpItem::TYPE::Speed:
				{
					OnCaughtItem("スピード強化アイテム", "スピードが上昇。");

					Player::_moveSpeed += 0.1f;
					break;

				}
				case PowerUpItem::TYPE::Bomb:
				{
					OnCaughtItem("ボム", "ボムを1つ獲得。");

					Player::_current_bomb_stock_count += 1;
					break;
				}

				}
				it->_isActive = false;
			}
		}
	}
}

void ItemManager::SetPowerUpItem_CollisionPairLists() {
	SetPowerUpItem_CollisionPairLists_DRY(_powerUpItem_heal);
	SetPowerUpItem_CollisionPairLists_DRY(_powerUpItem_attack);
	SetPowerUpItem_CollisionPairLists_DRY(_powerUpItem_defense);
	SetPowerUpItem_CollisionPairLists_DRY(_powerUpItem_speed);
	SetPowerUpItem_CollisionPairLists_DRY(_powerUpItem_bomb);
}


void ItemManager::DestroyAllItems() {
	_scoreItem_small.clear();
	_scoreItem_medium.clear();
	_scoreItem_large.clear();

	_powerUpItem_heal.clear();
	_powerUpItem_attack.clear();
	_powerUpItem_defense.clear();
	_powerUpItem_speed.clear();
	_powerUpItem_bomb.clear();
}

// イベント通知----------------------------------------------------------------------------------------------------------------------------------
void ItemManager::OnCaughtItem(const std::string item_name, const std::string effect) {

	std::string msg = item_name + "を獲得。 \n" + effect;

	Shared<EventNoticeText> event_msg = std::make_shared<EventNoticeText>(msg, GetColor(0, 255, 0), 15);

	EventNoticeText::_message_queue.push_back(event_msg);
}


void ItemManager::RenderEventHitText() const {

	int index = 0;
	for (auto msg : EventNoticeText::_message_queue) {
		msg->Render(index);
		index++;
	}
}


void ItemManager::UpdateEventHitText(const float& deltaTime) {

	// イベント通知
	for (auto msg : EventNoticeText::_message_queue) {
		msg->Update(deltaTime);
	}
	// 表示時間が切れたメッセージを削除する
	auto iter = EventNoticeText::_message_queue.begin();
	while (iter != EventNoticeText::_message_queue.end()) {

		if ((*iter)->IsExpired()) {
			iter = EventNoticeText::_message_queue.erase(iter);
		}
		else {
			++iter;
		}
	}
}

// 当たり判定-----------------------------------------------------------------------------------------------------------------------------------
void ItemManager::ScoreItemCollisionPairLists_DRY(std::vector<Shared<ScoreItem>>& scoreItems_1, std::vector<Shared<ScoreItem>>& scoreItems_2) {

	for (auto it : scoreItems_1) {
		for (auto& it2 : scoreItems_2) {
			_collision_ref->CheckCollision_ScoreItemAndScoreItem(it, it2, it->_mesh->pos_, it2->_mesh->pos_);
		}
	}
}

void ItemManager::ScoreItemCollisionPairLists()
{
	ScoreItemCollisionPairLists_DRY(_scoreItem_small, _scoreItem_small);    // 小と小
	ScoreItemCollisionPairLists_DRY(_scoreItem_small, _scoreItem_medium);   // 小と中
	ScoreItemCollisionPairLists_DRY(_scoreItem_small, _scoreItem_large);    // 小と大
	ScoreItemCollisionPairLists_DRY(_scoreItem_medium, _scoreItem_medium);  // 中と中
	ScoreItemCollisionPairLists_DRY(_scoreItem_medium, _scoreItem_large);   // 中と大
	ScoreItemCollisionPairLists_DRY(_scoreItem_large, _scoreItem_large);    // 大と大
}


void ItemManager::PowerUpItemCollisionPairLists_DRY(std::vector<Shared<PowerUpItem>>& powerUpItems_1, std::vector<Shared<PowerUpItem>>& powerUpItems_2) {

	for (auto it : powerUpItems_1) {
		for (auto& it2 : powerUpItems_2) {
			_collision_ref->CheckCollision_PowerUpItemAndPowerUpItem(it, it2, it->_mesh->pos_, it2->_mesh->pos_);
		}
	}
}

void ItemManager::PowerUpItemCollisionPairLists() {
	PowerUpItemCollisionPairLists_DRY(_powerUpItem_heal, _powerUpItem_heal);    // 回復と回復
	PowerUpItemCollisionPairLists_DRY(_powerUpItem_heal, _powerUpItem_attack);  // 回復と攻撃
	PowerUpItemCollisionPairLists_DRY(_powerUpItem_heal, _powerUpItem_defense); // 回復と防御
	PowerUpItemCollisionPairLists_DRY(_powerUpItem_heal, _powerUpItem_speed);   // 回復とスピード
	PowerUpItemCollisionPairLists_DRY(_powerUpItem_heal, _powerUpItem_bomb);    // 回復とボム

	PowerUpItemCollisionPairLists_DRY(_powerUpItem_attack, _powerUpItem_attack);   // 攻撃と攻撃
	PowerUpItemCollisionPairLists_DRY(_powerUpItem_attack, _powerUpItem_defense);  // 攻撃と防御
	PowerUpItemCollisionPairLists_DRY(_powerUpItem_attack, _powerUpItem_speed);    // 攻撃とスピード
	PowerUpItemCollisionPairLists_DRY(_powerUpItem_attack, _powerUpItem_bomb);     // 攻撃とボム

	PowerUpItemCollisionPairLists_DRY(_powerUpItem_defense, _powerUpItem_defense); // 防御と防御
	PowerUpItemCollisionPairLists_DRY(_powerUpItem_defense, _powerUpItem_speed);   // 防御とスピード
	PowerUpItemCollisionPairLists_DRY(_powerUpItem_defense, _powerUpItem_bomb);    // 防御とボム

	PowerUpItemCollisionPairLists_DRY(_powerUpItem_speed, _powerUpItem_speed);    // スピードとスピード
	PowerUpItemCollisionPairLists_DRY(_powerUpItem_speed, _powerUpItem_bomb);     // スピードとボム

	PowerUpItemCollisionPairLists_DRY(_powerUpItem_bomb, _powerUpItem_bomb);      // ボムとボム
}


void ItemManager::ScoreItemAndPowerUpItem_CollisionPairLists_DRY(std::vector<Shared<ScoreItem>>& scoreItems, std::vector<Shared<PowerUpItem>>& powerUpItems) {

	for (auto it : scoreItems) {
		for (auto& it2 : powerUpItems) {
			_collision_ref->CheckCollision_ScoreItemAndPowerUpItem(it, it2, it->_mesh->pos_, it2->_mesh->pos_);
		}
	}
}


void ItemManager::ScoreItemAndPowerUpItem_CollisionPairLists() {
	ScoreItemAndPowerUpItem_CollisionPairLists_DRY(_scoreItem_small, _powerUpItem_heal);    // 小アイテムと回復
	ScoreItemAndPowerUpItem_CollisionPairLists_DRY(_scoreItem_small, _powerUpItem_attack);  // 小アイテムと攻撃
	ScoreItemAndPowerUpItem_CollisionPairLists_DRY(_scoreItem_small, _powerUpItem_defense); // 小アイテムと防御
	ScoreItemAndPowerUpItem_CollisionPairLists_DRY(_scoreItem_small, _powerUpItem_speed);   // 小アイテムとスピード
	ScoreItemAndPowerUpItem_CollisionPairLists_DRY(_scoreItem_small, _powerUpItem_bomb);    // 小アイテムとボム

	ScoreItemAndPowerUpItem_CollisionPairLists_DRY(_scoreItem_medium, _powerUpItem_heal);   // 中アイテムと回復
	ScoreItemAndPowerUpItem_CollisionPairLists_DRY(_scoreItem_medium, _powerUpItem_attack); // 中アイテムと攻撃
	ScoreItemAndPowerUpItem_CollisionPairLists_DRY(_scoreItem_medium, _powerUpItem_defense);// 中アイテムと防御
	ScoreItemAndPowerUpItem_CollisionPairLists_DRY(_scoreItem_medium, _powerUpItem_speed);  // 中アイテムとスピード
	ScoreItemAndPowerUpItem_CollisionPairLists_DRY(_scoreItem_medium, _powerUpItem_bomb);   // 中アイテムとボム

	ScoreItemAndPowerUpItem_CollisionPairLists_DRY(_scoreItem_large, _powerUpItem_heal);   // 大アイテムと回復
	ScoreItemAndPowerUpItem_CollisionPairLists_DRY(_scoreItem_large, _powerUpItem_attack); // 大アイテムと攻撃
	ScoreItemAndPowerUpItem_CollisionPairLists_DRY(_scoreItem_large, _powerUpItem_defense);// 大アイテムと防御
	ScoreItemAndPowerUpItem_CollisionPairLists_DRY(_scoreItem_large, _powerUpItem_speed);  // 大アイテムとスピード
	ScoreItemAndPowerUpItem_CollisionPairLists_DRY(_scoreItem_large, _powerUpItem_bomb);   // 大アイテムとボム
}


// 描画-----------------------------------------------------------------------------------------------------------------------------------
void ItemManager::RenderPowerUpItems(std::vector<Shared<PowerUpItem>>& powerUpItems, const Shared<dxe::Camera>& camera) {

	if (!powerUpItems.empty()) {
		for (auto it_scrItm_large : powerUpItems) {
			if (it_scrItm_large->_isActive)
				it_scrItm_large->_mesh->render(camera);
		}
	}
}

void ItemManager::RenderScoreItems(std::vector<Shared<ScoreItem>>& scoreItems, const Shared<dxe::Camera>& camera) {

	if (!scoreItems.empty()) {
		for (auto it_scrItm_small : scoreItems) {
			if (it_scrItm_small->_isActive)
				it_scrItm_small->_mesh->render(camera);
		}
	}
}

void ItemManager::Render(const Shared<dxe::Camera>& camera) {

	RenderEventHitText();

	RenderScoreItems(_scoreItem_small, camera);
	RenderScoreItems(_scoreItem_medium, camera);
	RenderScoreItems(_scoreItem_large, camera);

	RenderPowerUpItems(_powerUpItem_heal, camera);
	RenderPowerUpItems(_powerUpItem_attack, camera);
	RenderPowerUpItems(_powerUpItem_defense, camera);
	RenderPowerUpItems(_powerUpItem_speed, camera);
	RenderPowerUpItems(_powerUpItem_bomb, camera);
}


// 更新-----------------------------------------------------------------------------------------------------------------------------------
void ItemManager::UpdatePowerUpItem_DRY(std::vector<Shared<PowerUpItem>>& powerUpItems) {

	if (!powerUpItems.empty()) {
		for (auto it_powItm = powerUpItems.begin(); it_powItm != powerUpItems.end();) {
			if ((*it_powItm)->Update((*it_powItm)) == false)
				it_powItm = powerUpItems.erase(it_powItm);
			else
				it_powItm++;
		}
	}
}


void ItemManager::UpdatePowerUpItem() {
	UpdatePowerUpItem_DRY(_powerUpItem_heal);
	UpdatePowerUpItem_DRY(_powerUpItem_attack);
	UpdatePowerUpItem_DRY(_powerUpItem_defense);
	UpdatePowerUpItem_DRY(_powerUpItem_speed);
	UpdatePowerUpItem_DRY(_powerUpItem_bomb);
}


void ItemManager::SpawnItemsOnEnemyDeath(const tnl::Vector3& enemyPos, const bool isEnemyDead) {

	std::random_device rd;
	std::mt19937 mt(rd());

	// 得点アイテムに番号を割り振り、ランダムで選択
	std::uniform_int_distribution<int> rnd_scoreItem_num(1, 3);
	// プレイヤー強化アイテムに番号を割り振り、ランダムで選択
	std::uniform_int_distribution<int> rnd_powerUpItem_num(1, 5);


	std::uniform_real_distribution<float> rnd_valX(-100, 100);
	std::uniform_real_distribution<float> rnd_valY(-10, 10);
	std::uniform_real_distribution<float> rnd_valZ(-100, 100);

	// 1体撃破でスポーンするアイテムの数を指定（↓処理のループ回数）
	std::uniform_int_distribution<int> rnd_loop_count(0, 4);
	int loop_count = rnd_loop_count(mt);

	for (int i = 0; i < loop_count; ++i) {

		tnl::Vector3 rnd_offset;
		rnd_offset.x = (float)rnd_valX(mt);
		rnd_offset.y = (float)rnd_valY(mt);
		rnd_offset.z = (float)rnd_valZ(mt);

		std::vector<Shared<ScoreItem>>* scoreItems = nullptr;

		int chooseScoreItem = rnd_scoreItem_num(mt);

		switch (chooseScoreItem)
		{
		case 1:
			scoreItems = &_scoreItem_small;	 break;
		case 2:
			scoreItems = &_scoreItem_medium; break;
		case 3:
			scoreItems = &_scoreItem_large;  break;
		}

		if (scoreItems && !scoreItems->empty()) {
			for (auto& item : *scoreItems) {
				if (isEnemyDead && !item->_hasSpawned) {
					item->_mesh->pos_ = enemyPos + rnd_offset;
					item->_hasSpawned = true;
					item->_isActive = true;
					break;
				}
			}
		}

		std::vector<Shared<PowerUpItem>>* powerUpItems = nullptr;

		int choosePowerUpItem = rnd_powerUpItem_num(mt);

		switch (choosePowerUpItem)
		{
		case 1:
			powerUpItems = &_powerUpItem_heal;	  break;
		case 2:
			powerUpItems = &_powerUpItem_attack;  break;
		case 3:
			powerUpItems = &_powerUpItem_defense; break;
		case 4:
			powerUpItems = &_powerUpItem_speed;  break;
		case 5:
			powerUpItems = &_powerUpItem_bomb;   break;
		}

		if (powerUpItems && !powerUpItems->empty()) {
			for (auto& item : *powerUpItems) {
				if (isEnemyDead && !item->_hasSpawned) {
					item->_mesh->pos_ = enemyPos + rnd_offset;
					item->_hasSpawned = true;
					item->_isActive = true;
					break;
				}
			}
		}

	}

	ScoreItemCollisionPairLists();
	PowerUpItemCollisionPairLists();
	ScoreItemAndPowerUpItem_CollisionPairLists();
}

void ItemManager::Update(const float& deltaTime) {

	UpdateScoreItem();
	UpdatePowerUpItem();

	UpdateEventHitText(deltaTime);

	SetScoreItem_CollisionPairLists();
	SetPowerUpItem_CollisionPairLists();
}

/*
void EnemyManager::NotifyEnemyPosition_ToItemManager() {

	std::vector<Shared<ItemBase>> allItems;

	allItems.insert(allItems.end(), ItemManager::_scoreItem_small.begin(), ItemManager::_scoreItem_small.end());
	allItems.insert(allItems.end(), ItemManager::_scoreItem_medium.begin(), ItemManager::_scoreItem_medium.end());
	allItems.insert(allItems.end(), ItemManager::_scoreItem_large.begin(), ItemManager::_scoreItem_large.end());

	allItems.insert(allItems.end(), ItemManager::_powerUpItem_heal.begin(), ItemManager::_powerUpItem_heal.end());
	allItems.insert(allItems.end(), ItemManager::_powerUpItem_attack.begin(), ItemManager::_powerUpItem_attack.end());
	allItems.insert(allItems.end(), ItemManager::_powerUpItem_defense.begin(), ItemManager::_powerUpItem_defense.end());
	allItems.insert(allItems.end(), ItemManager::_powerUpItem_speed.begin(), ItemManager::_powerUpItem_speed.end());
	allItems.insert(allItems.end(), ItemManager::_powerUpItem_bomb.begin(), ItemManager::_powerUpItem_bomb.end());

	for (const auto& observer : _observerItems) {
		observer->SpawnItemsOnEnemyDeath(allItems, _enemyZako_position_ref, _isEnemyZako_dead_ref);
	}
}



void ItemManager::SpawnItemsOnEnemyDeath(std::vector<Shared<ItemBase>>& items, const tnl::Vector3& enemyPos, const bool isEnemyDead) {

	std::random_device rd;
	std::mt19937 mt(rd());

	std::uniform_real_distribution<float> rnd_valX(-100, 100);
	std::uniform_real_distribution<float> rnd_valY(-10, 10);
	std::uniform_real_distribution<float> rnd_valZ(-100, 100);

	// 1体撃破でスポーンするアイテムの数を指定（↓処理のループ回数）
	std::uniform_int_distribution<int> rnd_loop_count(0, 4);
	int loop_count = rnd_loop_count(mt);

	// アイテムのリストをシャッフル
	std::shuffle(items.begin(), items.end(), mt);

	for (int i = 0; i < loop_count; ++i) {

		tnl::Vector3 rnd_offset;
		rnd_offset.x = (float)rnd_valX(mt);
		rnd_offset.y = (float)rnd_valY(mt);
		rnd_offset.z = (float)rnd_valZ(mt);

		auto& item = items[i];

		if (isEnemyDead && !item->_hasSpawned) {
			item->_mesh->pos_ = enemyPos + rnd_offset;
			item->_hasSpawned = true;
			item->_isActive = true;
			break;
		}
	}

	ScoreItemCollisionPairLists();
	PowerUpItemCollisionPairLists();
	ScoreItemAndPowerUpItem_CollisionPairLists();
}

*/