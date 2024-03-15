#pragma once

namespace inl {

	class Gunport
	{
	public:

		Gunport();

		// PlayerBullet::_bulletPowerRateの値が一定以上増えるごとに連装砲を追加する
		void ManageGunportCount(std::vector<Shared<Gunport>>& gunPort);

		void Render(const Shared<dxe::Camera> playerCamera);

	public:

		Shared<dxe::Mesh> _mesh;

	private:

		int _bulletPowerPhase{};
	};
}