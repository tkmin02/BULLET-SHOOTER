#include "Player.h"
#include "../../Sky/SkyBox.h"
#include "../../ScenePlay.h"
#include "../../Camera/FreeLookCamera.h"
#include "../../Pause/PauseMenu.h"
#include "../game/Loader/CsvLoader.h"
// 弾----------------------------------------------------------
#include "../../Bullet/Player/PlayerBullet.h"
#include "../game/ScenePlay/Bullet/Player/Gunport.h"
// 機能----------------------------------------------------------
#include "../../../Utility/InputFuncTable.h"
#include "../game/Utility/CustomException.h"
#include "../game/Utility/FilePathChecker.h"
// 敵-------------------------------------------------------------
#include "../../Character/Enemy/EnemyZakoBase.h"
#include "../../Character/Enemy/EnemyBossBase.h"
#include "../game/Manager/Enemy/EnemyManager.h"


namespace inl {

	Shared<dxe::Particle> Player::_bombParticle;

	namespace {

		// プレイヤーステータス -------------------------------------------------
		const float           _forwardVelocity{ 150.0f };
		const float           _HP_POS_X{ 60 };
		const float           _HP_POS_Y{ 50 };
		const tnl::Vector3    _START_POSITION{ 0, 100, -300 };

		// ボム------------------------------------------------------------------
		const float           _BOMBEFFECT_TIME_LIMIT{ 3.0f };

		// 無敵時間--------------------------------------------------------------

		const float           _INVINCIBLE_TIME_LIMIT{ 3.0f };

		// カメラ固定における敵補足可能範囲--------------------------------------
		const float           _CAPTURABLE_RANGE_ENEMY{ 500.0f };

		// カメラ----------------------------------------------------------------
		const tnl::Vector3    _DEFAULT_CAMERA_POSITION{ 0, 100, -150 };
		const tnl::Vector3    _CAMERA_OFFSET{ 0, -50, 20 };

		// 視点操作--------------------------------------------------------------
		const float           _VIEWPOINT_LERP_RATE_H{ 0.05f };
		const float           _VIEWPOINT_LERP_RATE_V{ 0.01f };
		const float           _CAMERAMOVE_DELAY_RATE{ 0.05f };

		// プレイヤーとの距離のオフセット---------------------------------------
		const float          _DISTANCE_OFFSET{ 300.0f };

		// プレイヤー操作-------------------------------------------------------
		const float          _CENTROID_RADIUS{ 100 };  // 重心
		const float          _MASS{ 100 };             // 質量
		const float          _FRICTION{ 0.6f };        // 摩擦
	}


	Player::Player(const Shared<FreeLookCamera> mainCamera) : _playerCamera(mainCamera) {

		Shared<CustomException> cus = std::make_shared<CustomException>();

		//　ロードに失敗したら例外発生----------------------------------------------------
		auto textureHandle = cus->TryLoadTexture("graphics/prismatic-star.png", "inl::Player::Player()");
		auto soundHandle = cus->TryLoadSound("sound/se/getHit.mp3", "inl::Player::Player()");
		auto bombParticleBinary = cus->TryLoadParticleBinaryFile("particle/preset/bombEffect.bin", "inl::Player::Player()");
		auto playerParticleBinary = cus->TryLoadParticleBinaryFile("particle/preset/playerParticle.bin", "inl::Player::Player()");
		auto csvPlayerStatus = cus->TryLoadCsvFile("csv/PlayerStatus.csv", "inl::Player::Player()");

		//--------------------------------------------------------------------------------

		InitPlayerStatus(csvPlayerStatus);

		// プレイヤー生成
		_mesh = dxe::Mesh::CreateCubeMV(20, 20, 20, false);
		_mesh->setTexture(textureHandle);
		_mesh->scl_ = { 1.0f, 1.0f, 1.0f };
		_mesh->pos_ = _START_POSITION;

		// ダメージSE
		_getDamageSE_hdl = soundHandle;

		// 爆発エフェクト
		_bombParticle = bombParticleBinary;

		//プレイヤー前方エフェクト
		_playerParticle = playerParticleBinary;

		// 連装砲
		_playerGunport = std::make_shared<inl::Gunport>();
	}


	void Player::InitPlayerStatus(std::vector<std::vector<tnl::CsvCell>> csv) {

		_csvLoader = std::make_shared<CsvLoader>();

		auto status = _csvLoader->LoadPlayerStatus(csv);

		_hp = status._hp;
		_MAX_HP = _hp;
		_at = status._at;
		_def = status._def;

		_collideSize = { 20, 20, 20 };
	}

	// 機能−-----------------------−-----------------------−-----------------------−--------------------------
	bool Player::DecreaseHP(int damage) {

		if (_hp > 0) {

			if (!_isInvincible) {

				damage = max(damage, 1);

				_hp -= damage;
				return true;
			}

			return false;
		}
		else if (_hp <= 0) {

			// ゲームオーバー
			_hp = 0;
			inl::PauseMenu::_isShowPauseOption = true;
			return false;
		}
		return false;
	}


	void Player::PlayDamageHitSE() noexcept {

		PlaySoundMem(_getDamageSE_hdl, DX_PLAYTYPE_BACK, TRUE);
	}


	void Player::NormalizeCameraSpeed(const float speed) {

		tnl::Vector3 zero = { 0,0,0 };

		if ((_moveVelocity - zero).length() > 0.0f) {

			// ベクトル正規化
			_moveVelocity = _moveVelocity.Normalize(_moveVelocity) * speed;
		}
	}


	bool Player::IsEnemyInCapturableRange() {

		if (!_enemyManager_ref)
			return false;

		tnl::Vector3 enemyPos;
		AssignEnemyPosition(enemyPos);

		//　プレイヤーと敵の距離
		float dis_playerAndEnemy = (enemyPos - _mesh->pos_).length();

		if (dis_playerAndEnemy < _CAPTURABLE_RANGE_ENEMY) {
			return true;
		}

		return false;
	}


	void Player::AssignEnemyPosition(tnl::Vector3& enemyPos) {

		int enemyZakoLeftCount = _enemyManager_ref->GetRemainingEnemyCount();
		auto zakoList = _enemyManager_ref->_enemyZakoList;
		auto it_zako = _enemyManager_ref->_itZako;

		auto bossList = _enemyManager_ref->_enemyBossList;
		auto it_boss = _enemyManager_ref->_itBoss;

		//　通常エネミーが全滅していれば
		if (_enemyManager_ref->_enemyZakoList.empty()) {
			enemyZakoLeftCount = 0;
		}

		if (!bossList.empty()) {

			for (auto& boss : bossList) {

				//　ボスが撃破されていれば
				if (boss->_isDead)
					enemyZakoLeftCount = 0;
			}
		}

		//　ザコ敵が全滅していなければ
		if (enemyZakoLeftCount != 0 && !zakoList.empty()) {

			// インデックスが置かれている敵のみイテレーターで取得
			if (_enemyIndex < zakoList.size()) {

				it_zako = std::next(zakoList.begin(), _enemyIndex);
				enemyPos = (*it_zako)->_mesh->pos_;
			}
			else {
				_enemyIndex = 0;
				it_zako = zakoList.begin();
				enemyPos = (*it_zako)->_mesh->pos_;
			}
		}
		//　ザコ敵が全滅しており、ボスがまだ生きていれば
		else if (enemyZakoLeftCount == 0 && zakoList.empty() && !bossList.empty()) {

			it_boss = bossList.begin();

			if ((*it_boss)->_mesh != nullptr) {
				enemyPos = (*it_boss)->_mesh->pos_;
			}
		}
	}


	const tnl::Vector3& Player::GetBulletMoveDirection() {

		tnl::Vector3 moveDir, enemyPos;

		if (_playerCamera->follow) {

			AssignEnemyPosition(enemyPos);
			moveDir = enemyPos - _mesh->pos_;
			moveDir.normalize();
		}
		else {
			moveDir = tnl::Vector3::TransformCoord({ 0,0,1 }, _mesh->rot_);
		}

		return moveDir;
	}

	// 操作−-----------------------−-----------------------−----------------------------------------------------
	void Player::ShotPlayerBullet() {

		if (!IsShooting()) return;

		tnl::Vector3 spawnPos = _mesh->pos_;
		tnl::Vector3 moveDir = GetBulletMoveDirection();

		_straightBullets_player.emplace_back(
			std::make_shared<inl::PlayerBullet>(
				spawnPos,                        //　位置
				moveDir,						 //　方向
				inl::PlayerBullet::COLOR::Gray,  //　色
				10.0f)							 //　サイズ
		);
	}


	void Player::ShotGunportBullet() {

		if (!IsShooting()) return;

		tnl::Vector3 moveDir = GetBulletMoveDirection();

		for (const auto& blt : _gunportVec) {

			if (!blt) return;

			_straightBullets_player.emplace_back(
				std::make_shared<inl::PlayerBullet>(
					blt->_mesh->pos_,                   //　位置
					moveDir,							//　方向
					inl::PlayerBullet::COLOR::White,	//　色
					7.0f                                //　サイズ
				)
			);
		}
	}


	void Player::ControlPlayerMoveByInput(const float deltaTime) {

		float speed = 4.f;

		// 減速  ゲームパッドの場合は△かＹ
		if (tnl::Input::IsKeyDown(eKeys::KB_LSHIFT) ||
			tnl::Input::IsPadDown(ePad::KEY_3))
		{
			speed = 2.5f;
		}

		// 左方向
		if (InputFuncTable::IsButtonDown_LEFT() &&
			!_playerCamera->follow)
		{

			_moveVelocity -= tnl::Vector3::TransformCoord({ 1.0f, 0, 0 }, _rotY);
			NormalizeCameraSpeed(speed);
		}

		// 右方向
		if (InputFuncTable::IsButtonDown_RIGHT() &&
			!_playerCamera->follow)
		{

			_moveVelocity += tnl::Vector3::TransformCoord({ 1.0f, 0, 0 }, _rotY);
			NormalizeCameraSpeed(speed);
		}

		if (tnl::Input::IsKeyDown(eKeys::KB_W) || tnl::Input::IsPadDown(ePad::KEY_UP))
		{

			_moveVelocity += tnl::Vector3::TransformCoord({ 0, 1.0f, 0.1f }, _rotY);
			NormalizeCameraSpeed(speed);
		}

		// 下方向
		if (InputFuncTable::IsButtonDown_DOWN())
		{

			_moveVelocity -= tnl::Vector3::TransformCoord({ 0, 1.0f, 0.1f }, _rotY);
			NormalizeCameraSpeed(speed);
		}

		// 前方向　ゲームパッドの場合はL1
		if (tnl::Input::IsKeyDown(eKeys::KB_SPACE) ||
			tnl::Input::IsPadDown(ePad::KEY_4))
		{

			_mesh->pos_ +=
				tnl::Vector3::TransformCoord({ 0,0,2 }, _mesh->rot_) * _forwardVelocity * deltaTime;
		}
	}


	void Player::ChangeTarget_ByMouseWheel() {

		tnl::Vector3 vel = tnl::Input::GetRightStick();

		int wheel = tnl::Input::GetMouseWheel();
		auto zakoList = _enemyManager_ref->_enemyZakoList;

		// マウスホイールの入力に応じて敵のインデックスを増減
		if (wheel > 0 || tnl::Input::IsPadDownTrigger(ePad::KEY_9)) {

			_enemyIndex++;

			if (_enemyIndex >= zakoList.size()) {
				_enemyIndex = 0;
			}
		}
		else if (wheel < 0 || 0) {

			_enemyIndex--;

			if (_enemyIndex < 0) {
				_enemyIndex = static_cast<int>(zakoList.size() - 1);
			}
		}
	}


	void Player::ControlRotationByPadOrMouse() {

		// ゲームパッド
		if (!_playerCamera->follow) {

			tnl::Vector3 vel = tnl::Input::GetRightStick();

			if (vel.x > 0.2f || vel.x < -0.2f) {

				// 左右視点
				_rotY *= tnl::Quaternion::RotationAxis({ 0, 1, 0 }, tnl::ToRadian(vel.x * 1.2f));
			}

			if (vel.y > 0.2f || vel.y < -0.2f) {

				// プレイヤーの前方
				tnl::Vector3 forward = tnl::Vector3::TransformCoord({ 0, 0, 1 }, _rotY);

				// 上下視点
				_rotX *= tnl::Quaternion::RotationAxis(
					tnl::Vector3::Cross({ 0, 1, 0 }, forward),  // 外積
					tnl::ToRadian(vel.y * 2.f)					// 回転（ラジアン）
				);
			}
		}

		// マウス
		if (!_playerCamera->follow
			&& tnl::Input::GetRightStick().x == 0
			&& tnl::Input::GetRightStick().y == 0
			&& tnl::Input::GetRightStick().z == 0) {

			tnl::Vector3 vel = tnl::Input::GetMouseVelocity();

			// 左右視点
			_rotY *= tnl::Quaternion::RotationAxis({ 0, 1, 0 }, tnl::ToRadian(vel.x * _VIEWPOINT_LERP_RATE_H));

			// プレイヤーの前方
			tnl::Vector3 forward = tnl::Vector3::TransformCoord({ 0, 0, 1 }, _rotY);

			// 上下視点
			_rotX *= tnl::Quaternion::RotationAxis(
				tnl::Vector3::Cross({ 0, 1, 0 }, forward),     // 外積
				tnl::ToRadian(vel.y * _VIEWPOINT_LERP_RATE_V)  // 回転（ラジアン）
			);
		}
	}


	void Player::AdjustPlayerVelocity() {

		// Time.deltaTimeのようなもの。これがないとプレイヤーが吹っ飛ぶ
		tnl::EasyAdjustObjectVelocity(
			_CENTROID_RADIUS,    //　重心
			_MASS,               //　質量
			_FRICTION,           //　摩擦
			_past_moveVelocity,  //　前回の移動ベクトル
			_moveVelocity,       //　現在の移動ベクトル
			_centerOfGravity     //　重心座標
		);

		if (_centerOfGravity.length() > FLT_EPSILON) {

			// 重心位置を利用して傾いてほしいアッパーベクトルを作成
			tnl::Vector3 upper =
				tnl::Vector3::Normalize({ _centerOfGravity.x, _CENTROID_RADIUS, _centerOfGravity.z });

			// 傾きの角度を計算
			float angle = upper.angle({ 0, 1, 0 });

			// 傾きベクトルと真上ベクトルの外積から回転軸を計算し、傾き角度を調整して回転クォータニオンを作成
			_rotXZ = tnl::Quaternion::RotationAxis(tnl::Vector3::Cross(upper, { 0, 1, 0 }), -(angle * 0.2f));
		}

		//　位置
		_mesh->pos_ += _moveVelocity;

		//_mesh->rot_ = rot_y_ * rot_xz_;
		// ControlRotationByMouse で上下視点も使用する場合は↓を使う
		//　回転（　最終的な姿勢　）
		_mesh->rot_ = _rotY * _rotX * _rotXZ;
	}

	// カメラ−-----------------------−-----------------------−-----------------------−-------------------------
	void Player::ActivateDarkSoulsCamera() {

		tnl::Vector3 playerPos = _mesh->pos_;
		tnl::Vector3 targetEnemyPos;

		//　敵座標取得
		AssignEnemyPosition(targetEnemyPos);

		//　敵とのユークリッド距離
		float dis_player_enemy = (targetEnemyPos - playerPos).length();

		if (_playerCamera->follow) {

			ControlCameraWithEnemyFocus(playerPos, targetEnemyPos);
		}
		else {

			ControlCameraWithoutEnemyFocus();
		}
	}



	void Player::ControlCameraWithoutEnemyFocus()
	{
		tnl::Vector3 playerPos = _mesh->pos_;

		// 敵にカメラを固定しない場合
		_playerCamera->target_ = playerPos;
		_playerCamera->target_ -= _CAMERA_OFFSET;

		ControlRotationByPadOrMouse();

		// カメラの動きの遅延処理
		tnl::Vector3 fixPos =
			playerPos + tnl::Vector3::TransformCoord(_DEFAULT_CAMERA_POSITION, _mesh->rot_);

		_playerCamera->pos_ += (fixPos - _playerCamera->pos_) * _CAMERAMOVE_DELAY_RATE;

		// 追従ポインターOFF
		_playerCamera->isShowTargetPointer = false;
	}


	void Player::ControlCameraWithEnemyFocus(const tnl::Vector3& playerPos, const tnl::Vector3& targetEnemyPos)
	{
		// 追従ポインターON（描画）
		_playerCamera->isShowTargetPointer = true;

		ChangeTarget_ByMouseWheel();
		RenderFollowPointer();

		tnl::Vector3 tmp{};
		tmp.y = playerPos.y + 100;
		// カメラをプレイヤーと敵の中間地点に固定
		_playerCamera->target_ = (tmp + targetEnemyPos) / 2;

		tnl::Quaternion q = tnl::Quaternion::RotationAxis({ 0,1,0 }, _playerCamera->axis_y_angle_);
		tnl::Vector3 xz = tnl::Vector3::TransformCoord({ 0,0,1 }, q);
		tnl::Vector3 localAxis_x = tnl::Vector3::Cross({ 0,1,0 }, xz);
		q *= tnl::Quaternion::RotationAxis(localAxis_x, _playerCamera->axis_x_angle_);

		_mesh->rot_ = tnl::Quaternion::LookAt(playerPos, targetEnemyPos, localAxis_x);

		float y = 0;

		ControlPlayerMoveWithEnemyFocus(q, y);

		// カメラの動きの遅延処理
		tnl::Vector3 fixPos = playerPos + tnl::Vector3::TransformCoord({ 50, y, -140 }, _mesh->rot_);
		_playerCamera->pos_ += (fixPos - _playerCamera->pos_) * 0.1f;
	}



	void Player::ControlPlayerMoveWithEnemyFocus(tnl::Quaternion& q, float& y)
	{
		//　左方向
		if (InputFuncTable::IsButtonDown_LEFT()) {

			tnl::Vector3 newPos =   //　カメラ焦点座標　+　指定の座標
				_playerCamera->target_ +
				tnl::Vector3::TransformCoord({ _DISTANCE_OFFSET, 0, _DISTANCE_OFFSET }, q);

			newPos.y = _mesh->pos_.y;

			y = 100;
			_mesh->pos_ = newPos;
			_playerCamera->axis_y_angle_ += tnl::ToRadian(2);
		}

		//　右方向
		if (InputFuncTable::IsButtonDown_RIGHT()) {

			tnl::Vector3 newPos =  //　カメラ焦点座標　+　指定の座標
				_playerCamera->target_ +
				tnl::Vector3::TransformCoord({ _DISTANCE_OFFSET, 0, _DISTANCE_OFFSET }, q);

			newPos.y = _mesh->pos_.y;

			y = -100;
			_mesh->pos_ = newPos;
			_playerCamera->axis_y_angle_ -= tnl::ToRadian(2);
		}
	}

	// ボム−-----------------------−-----------------------−-----------------------−-----------------------
	void Player::ValidateBombEffect() {

		float adaptRange = 200.f;

		float euclideanDistance = (_bombParticle->getPosition() - _mesh->pos_).length();

		if (euclideanDistance <= adaptRange) {
			ScenePlay::DeactivateAllEnemyBullets();
		}
	}


	void Player::UseBomb() {

		// マウス中央　ゲームパッドの場合はバツボタン、もしくはAボタンなど
		if (tnl::Input::IsMouseTrigger(eMouseTrigger::IN_MIDDLE) && _currentBomb_stockCount != 0 ||
			tnl::Input::IsPadDownTrigger(ePad::KEY_0) && _currentBomb_stockCount != 0) {

			_isTriggered_playersBombEffect = true;
			_currentBomb_stockCount--;
		}
	}


	void Player::InvalidateBombEffect(const float deltaTime) {

		if (_isTriggered_playersBombEffect) {

			_bombTimer += deltaTime;
			ValidateBombEffect();

			if (_bombTimer > _BOMBEFFECT_TIME_LIMIT) {

				_bombTimer = 0.0f;
				ScenePlay::DeactivateAllEnemyBullets();
				_isTriggered_playersBombEffect = false;
			}
		}
	}


	// 描画−-----------------------−-----------------------−-----------------------−-----------------------------
	void Player::RenderFollowPointer() {

		tnl::Vector3 enemyPos;

		if (_playerCamera->isShowTargetPointer) {

			AssignEnemyPosition(enemyPos);
		}

		// スクリーン座標へ変換
		tnl::Vector3 screenPos = tnl::Vector3::ConvertToScreen
		(
			{ enemyPos.x, enemyPos.y, enemyPos.z }
			, DXE_WINDOW_WIDTH
			, DXE_WINDOW_HEIGHT
			, _playerCamera->view_
			, _playerCamera->proj_
		);

		// 追従ポインター描画
		DrawCircleAA(screenPos.x, screenPos.y, 5, 10, GetColor(255, 0, 0), 1, 1);
	}


	void Player::RenderGunport(const Shared<inl::FreeLookCamera> camera) {

		if (_gunportVec.empty()) return;

		for (const auto& it : _gunportVec) {
			it->Render(camera);
		}
	}


	void Player::RenderBulletPowerRate(const int color) {

		std::ostringstream stream;
		stream << std::fixed << std::setprecision(2) << inl::PlayerBullet::_bulletPowerRate;

		std::string s = stream.str();

		SetDrawBlendMode(DX_BLENDMODE_ALPHA, 40);
		DrawBox(995, 135, 1150, 160, 1, true);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);

		DrawFormatString(1000, 140, color, "Power:%s / 5.00", s.c_str());
	}


	void Player::RenderBombRemainCount(const int color) noexcept {

		std::string s = std::to_string(_currentBomb_stockCount);

		SetDrawBlendMode(DX_BLENDMODE_ALPHA, 40);
		DrawBox(25, 77, 100, 100, 1, true);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);

		DrawFormatString(30, 80, color, "Bomb:%s", s.c_str());
	}


	void Player::RenderPlayerHp(const int color) {

		if (_hp <= 0) return;

		float x2 = _HP_POS_X + 150;

		float gageWidth = abs(x2 - _HP_POS_X);

		float average = (_MAX_HP > 0) ? gageWidth / _MAX_HP : 0;

		x2 = _HP_POS_X + static_cast<int>(average * _hp);

		// HPゲージの枠
		DrawBoxAA(_HP_POS_X,
			_HP_POS_Y,
			210,
			65,
			GetColor(150, 150, 150),
			true
		);
		// HPゲージ本体
		DrawBoxAA(_HP_POS_X,
			_HP_POS_Y,
			x2,
			65,
			GetColor(0, 255, 0),
			true
		);

		SetFontSize(16);
		DrawString(30, 50, "HP:", color);
	}


	void Player::Render(const Shared<inl::FreeLookCamera> camera) {

		RenderPlayerParticle(camera);

		TriggerInvincible(camera);

		RenderGunport(camera);

		int color = -1;

		switch (ScenePlay::GetStageID())
		{
		case 1:	color = 1;	break;
		case 2:	color = GetColor(0, 220, 0); break;
		}
		
		
		RenderPlayerHp(color);

		// ボムの残数
		SetFontSize(18);
		RenderBombRemainCount(color);

		RenderBulletPowerRate(color);
		
		for (auto& blt : _straightBullets_player) {
			blt->Render(camera);
		}

		if (_playerCamera->follow) {

			SetDrawBlendMode(DX_BLENDMODE_ALPHA, 40);
			DrawBox(20, 430, 410, 460, 1, true);
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);

			DrawStringEx(25, 435, color, "ターゲット変更：マウスホイール or 右スティック押下");
		}

		SetDrawBlendMode(DX_BLENDMODE_ALPHA, 40);
		DrawBox(20, 475, 280, 505, 1, true);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);

		DrawStringEx(25, 480, color, "姿勢を戻す: Key C or Pad X (□)");
	}


	void Player::RenderPlayerParticle(const Shared<inl::FreeLookCamera>& camera)
	{
		dxe::DirectXRenderBegin();
		inl::Player::_playerParticle->setPosition(_mesh->pos_);
		inl::Player::_playerParticle->start();
		inl::Player::_playerParticle->render(camera);
		dxe::DirectXRenderEnd();
	}


	void Player::TriggerInvincible(const Shared<inl::FreeLookCamera>& camera)
	{
		if (_isInvincible) {

			if (static_cast<int>(_invincibleTimer * 10) % 3 == 0) {

				//　プレイヤー半透明化
				_mesh->setBlendMode(DX_BLENDMODE_ADD);
				_mesh->setAlpha(0.8f);
				_mesh->render(camera);
			}
		}
		else {
			_mesh->setBlendMode(DX_BLENDMODE_NOBLEND);
			_mesh->render(camera);
		}
	}


	// 更新−-----------------------−-----------------------−-----------------------−-----------------------−-----
	void Player::UpdateStraightBullet(const float deltaTime)
	{
		auto it_blt = _straightBullets_player.begin();

		while (it_blt != _straightBullets_player.end()) {

			(*it_blt)->Update(deltaTime);

			if (!(*it_blt)->_isActive) {

				it_blt = _straightBullets_player.erase(it_blt);
				continue;
			}
			it_blt++;
		}
	}


	void Player::WatchInvincibleTimer(const float deltaTime) noexcept {

		if (_isInvincible) {

			_invincibleTimer += deltaTime;

			if (_invincibleTimer >= _INVINCIBLE_TIME_LIMIT) {

				_invincibleTimer = 0.0f;
				_isInvincible = false;
			}
		}
	}


	void Player::UpdateGunport_DRY(Shared<inl::Gunport>& gunportVec, const tnl::Vector3 coords) {

		//　位置
		gunportVec->_mesh->pos_ = GetPos() + tnl::Vector3::TransformCoord({ coords }, _mesh->rot_);
		//　回転
		gunportVec->_mesh->rot_ = _mesh->rot_;
	}


	void Player::UpdateGunport() {

		if (_gunportVec.empty()) return;

		//　各連装砲の配置座標（ 最大５ ）
		const tnl::Vector3 coords[] = {
		{  0, -25, -20},
		{-25,   0, -20},
		{ 25,   0, -20},
		{-15, -15, -20},
		{ 15, -15, -20}
		};


		switch (_gunportVec.size())
		{
		case 1:
		{
			UpdateGunport_DRY(_gunportVec[0], { coords[0] });
			break;
		}
		case 2:
		{
			UpdateGunport_DRY(_gunportVec[0], { coords[1] });
			UpdateGunport_DRY(_gunportVec[1], { coords[2] });
			break;
		}
		case 3:
		{
			UpdateGunport_DRY(_gunportVec[0], { coords[1] });
			UpdateGunport_DRY(_gunportVec[1], { coords[2] });
			UpdateGunport_DRY(_gunportVec[2], { coords[0] });
			break;
		}
		case 4:
		{
			UpdateGunport_DRY(_gunportVec[0], { coords[1] });
			UpdateGunport_DRY(_gunportVec[1], { coords[2] });
			UpdateGunport_DRY(_gunportVec[2], { coords[3] });
			UpdateGunport_DRY(_gunportVec[3], { coords[4] });
			break;
		}
		case 5:
		{
			UpdateGunport_DRY(_gunportVec[0], { coords[1] });
			UpdateGunport_DRY(_gunportVec[1], { coords[2] });
			UpdateGunport_DRY(_gunportVec[2], { coords[3] });
			UpdateGunport_DRY(_gunportVec[3], { coords[4] });
			UpdateGunport_DRY(_gunportVec[4], { coords[0] });
			break;
		}
		}
	}


	void Player::Update(const float deltaTime) {

		//　「 Begin 」テキスト
		WatchInvincibleTimer(deltaTime);

		// カメラを敵に固定するフラグを反転
		// マウス右　ゲームパッドの場合はR1（RB)
		if (tnl::Input::IsMouseTrigger(eMouseTrigger::IN_RIGHT) ||
			tnl::Input::IsPadDownTrigger(ePad::KEY_5))
		{
			if (IsEnemyInCapturableRange() || _playerCamera->follow)
				_playerCamera->follow = !_playerCamera->follow;
		}

		//　カメラ
		ActivateDarkSoulsCamera();
		_playerCamera->Update(deltaTime);

		//　プレイヤー操作
		ControlPlayerMoveByInput(deltaTime);
		AdjustPlayerVelocity();

		//　弾発射
		ShotPlayerBullet();
		ShotGunportBullet();
		UpdateStraightBullet(deltaTime);

		//　ガンポート
		_playerGunport->ManageGunportCount(_gunportVec);
		UpdateGunport();

		//　ボム
		UseBomb();
		InvalidateBombEffect(deltaTime);

		// 姿勢制御
		if (tnl::Input::IsKeyDownTrigger(eKeys::KB_C) || tnl::Input::IsPadDownTrigger(ePad::KEY_2)) {

			_rotY = tnl::Quaternion(0, 0, 0, 1);
		}
	}
}