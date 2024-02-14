#pragma once
#include "EnemyBullet.h"


class HomingBullet : public EnemyBullet
{
public:

	enum class USER {

		None,
		ZakoBox,
		ZakoDome,
		ZakoCylinder
	};


	HomingBullet();

	void CheckLifeTimeDistance(const Shared<HomingBullet>& homingBullet);

	void Render(Shared<dxe::Camera> _mainCamera) override;
	void Update(const float deltaTime) override;

public:

	Shared<dxe::Mesh> _mesh = nullptr;
};