#pragma once
#include "ItemBase.h"


class ScenePlay;

// �v���C���[�����A�C�e��
class PowerUpItem : public ItemBase
{
public:

	enum class TYPE {

		Default,
		Heal,
		Attack,
		Defense,
		Speed,
		Bomb
	};

	PowerUpItem::TYPE type = PowerUpItem::TYPE::Default;

	PowerUpItem() {}
	explicit PowerUpItem(Shared<PowerUpItem> item) {}
	explicit PowerUpItem(const PowerUpItem::TYPE item_type);

	bool Update(Shared<PowerUpItem>& item);
};