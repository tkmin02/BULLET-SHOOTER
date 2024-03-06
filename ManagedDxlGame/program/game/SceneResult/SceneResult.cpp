#include "../DxLibEngine.h"
#include "../SceneTitle/SceneTitle.h"
#include "../Manager/Scene/SceneManager.h"
#include "SceneResult.h"


namespace {

	const int   _backGround_posX{ 640 };
	const int   _backGround_posY{ 380 };
	const int   _backGround_baseAlpha{ 220 };
	const float _backGround_extendRate{ 0.8f };

	// Result
	const int   _resultText_posX{ 530 };
	const int   _resultText_posY{ 150 };

	// ��Փx
	const int   _difficultyText_posX{ 320 };
	const int   _difficultyText_posY{ 250 };

	// TotalScore
	const int   _totalScoreText_posX{ 800 };
	const int   _totalScoreText_posY{ 270 };
}


SceneResult::SceneResult(const std::string difficulty, const int totalScore)
	: _totalScore(totalScore), _difficulty(difficulty) {

	_backGround_hdl = LoadGraph("graphics/Scene/resultBackGround.png");

	_resultSE_hdl = LoadSoundMem("sound/se/result.mp3");
	PlaySoundMem(_resultSE_hdl, DX_PLAYTYPE_BACK);
}


void SceneResult::Render() {

	// �w�i�摜
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, _backGround_baseAlpha);
	DrawRotaGraph(_backGround_posX, _backGround_posY, _backGround_extendRate, 0, _backGround_hdl, true);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

	SetFontSize(70);
	DrawString(_resultText_posX, _resultText_posY, "Result", GetColor(0, 200, 100));

	SetFontSize(40);
	DrawFormatString(_difficultyText_posX, _difficultyText_posY, GetColor(0, 200, 100), "%s", _difficulty.c_str());
	SetFontSize(30);
	DrawFormatString(_totalScoreText_posX, _totalScoreText_posY, GetColor(0, 200, 100), "TotalScore  %d", _totalScore);

	SetFontSize(DEFAULT_FONT_SIZE);
}


void SceneResult::Update(float deltaTime) {
	_sequence.update(deltaTime);
}


bool SceneResult::SeqIdle(float deltaTime) {

	if (tnl::Input::IsKeyDownTrigger(eKeys::KB_RETURN) || tnl::Input::IsPadDownTrigger(ePad::KEY_1)) {

		auto mgr = SceneManager::GetInstance();
		mgr->ChangeScene(new SceneTitle());
	}

	return true;
}