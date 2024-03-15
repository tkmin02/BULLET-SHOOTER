#pragma once
#include "../Manager/Scene/SceneBase.h"


class SceneSelectDifficulty : public SceneBase
{
public:

	SceneSelectDifficulty();
	~SceneSelectDifficulty() {
		DeleteGraph(_backGround_hdl); 
	}

	void Render() override;
	void Update(const float deltaTime) override;

private:

	//@οΥxIπόΝiγΊj
	void UpdateSelectDifficultyCursor_ByInput() noexcept;
	//@οΥxΪ`ζ
	void RenderDifficultiesAndAnnotation() noexcept;
	//@wi`ζ
	void RenderBackGround() noexcept;
	//@οΥxθiScenePlayΦςΤj @JnXe[Wπ©RΙΟXΕ«ά·
	void DecideSelectedLevel_ByInput();

private:

	// wi
	int         _backGround_hdl{};
	int         _difficultyItemIndex{};
};