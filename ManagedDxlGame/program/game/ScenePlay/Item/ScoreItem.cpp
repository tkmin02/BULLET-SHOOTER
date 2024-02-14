#include "ScoreItem.h"
#include "../../ScenePlay/ScenePlay.h"


ScoreItem::ScoreItem(const ScoreItem::TYPE item_type) {

	switch (item_type)
	{
	case ScoreItem::TYPE::Small:
	{
		float size_small = 10;
		_mesh = dxe::Mesh::CreateSphereMV(size_small);
		_mesh->setTexture(dxe::Texture::CreateFromFile("graphics/itemTexture/prism.jpg"));
		_collisionSize = { size_small * 2.0f,size_small * 2.0f,size_small * 2.0f };
		_sizeName = "small";
		break;
	}
	case ScoreItem::TYPE::Medium:
	{
		float size_medium = 15;
		_mesh = dxe::Mesh::CreateSphereMV(size_medium);
		_mesh->setTexture(dxe::Texture::CreateFromFile("graphics/itemTexture/sapphire.jpg"));
		_collisionSize = { size_medium * 2.0f,size_medium * 2.0f,size_medium * 2.0f };
		_sizeName = "medium";
		break;
	}
	case ScoreItem::TYPE::Large:
	{
		float size_large = 20;
		_mesh = dxe::Mesh::CreateSphereMV(size_large);
		_mesh->setTexture(dxe::Texture::CreateFromFile("graphics/itemTexture/gold.jpg"));
		_collisionSize = { size_large * 2.0f,size_large * 2.0f,size_large * 2.0f };
		_sizeName = "large";
		break;
	}
	}
}


bool ScoreItem::Update(Shared<ScoreItem>& item) {

	if (item->_isActive) {
		item->_lifeTimer += ScenePlay::GetDeltaTime();

		_velocity += _gravity * ScenePlay::GetDeltaTime() * 2;
		item->_mesh->pos_ += _velocity * ScenePlay::GetDeltaTime();


		if (item->_lifeTimer > 20) {
			item->_lifeTimer = 0;
			item->_isActive = false;
			return false;
		}
	}

	return true;
}