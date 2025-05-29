#include <Novice.h>
#include "Player.h"
#include "InputRecorder.h"
#include <vector>
#include <string>
#include <filesystem>
#include "imgui.h"

// ウィンドウタイトル
const char kWindowTitle[] = "LC1C_24_マキノハルト_タイトル";

// フラグと仮想入力
bool isReplaying = false;
char replayKeys[256] = {};

// リプレイ用JSONファイルを取得（先頭が"replay_"で拡張子が.jsonのもの）
std::vector<std::string> GetReplayFileList(const std::string& folderPath = ".") {
	std::vector<std::string> files;
	for (const auto& entry : std::filesystem::directory_iterator(folderPath)) {
		if (entry.path().extension() == ".json" && entry.path().filename().string().find("replay_") == 0) {
			files.push_back(entry.path().filename().string());
		}
	}
	return files;
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	// 一時ファイル削除
	//std::remove("replay.json");

	// Novice初期化
	Novice::Initialize(kWindowTitle, 1280, 720);

	// 入力配列
	char keys[256] = { 0 };
	char preKeys[256] = { 0 };

	// インスタンス生成
	Player* player = new Player();
	InputRecorder* recorder = new InputRecorder();

	// 時間管理
	float time = 0.0f;
	const float deltaTime = 1.0f / 60.0f;

	// リプレイインデックスと状態管理
	int replayIndex = 0;
	bool waitForRecordStart = false;

	// メインループ
	while (Novice::ProcessMessage() == 0) {
		Novice::BeginFrame();

		// 入力取得
		memcpy(preKeys, keys, 256);
		Novice::GetHitKeyStateAll(keys);

		// TABキーでモード切替（記録 ↔ 再生）
		if (keys[DIK_TAB] && !preKeys[DIK_TAB]) {
			isReplaying = !isReplaying;
			player->Reset();
			time = 0.0f;
			replayIndex = 0;

			if (isReplaying) {
				// 自動保存
				static int autoSaveIndex = 0;
				std::string filename = "replay_" + std::to_string(autoSaveIndex++) + ".json";
				recorder->SaveToFile(filename);

				// 再生用データ読み込み
				recorder->LoadFromFile("replay.json");

				if (recorder->GetData().empty()) {
					isReplaying = false;
				}
			}
		}


		// 再生モードの処理
		if (isReplaying) {
			memset(replayKeys, 0, 256);

			const auto& inputs = recorder->GetData();
			while (replayIndex < inputs.size() && inputs[replayIndex].time <= time) {
				const std::string& input = inputs[replayIndex].input;
				if (input == "Left") replayKeys[DIK_LEFT] = 1;
				else if (input == "Right") replayKeys[DIK_RIGHT] = 1;
				else if (input == "Jump") replayKeys[DIK_SPACE] = 1;
				replayIndex++;
			}

			if (!inputs.empty() && time > inputs.back().time + 1.0f) {
				//std::remove("replay.json");
				recorder->Clear();
				time = 0.0f;
				replayIndex = 0;
				isReplaying = false;
				waitForRecordStart = true;
			}

			player->Update(replayKeys, true);
		}
		// 記録中の処理
		else if (!waitForRecordStart) {
			recorder->Record(time, keys);
			player->Update(keys, false);
		}

		// Rキーで記録再開
		if (waitForRecordStart && keys[DIK_R] && !preKeys[DIK_R]) {
			waitForRecordStart = false;
			time = 0.0f;
			replayIndex = 0;
			recorder->Clear();
			player->Reset();
			player->ClearTrail();
		}

		// 描画処理
		player->Draw();

		// 軌跡（記録中：赤 / 再生中：青）
		const auto& trail = player->GetTrail();
		const auto& replayTrail = player->GetReplayTrail();

		for (size_t i = 1; i < trail.size(); ++i) {
			const Vector2& p1 = trail[i - 1], & p2 = trail[i];
			if ((p2.x - p1.x) * (p2.x - p1.x) + (p2.y - p1.y) * (p2.y - p1.y) < 2500.0f) {
				Novice::DrawLine((int)p1.x + 32, (int)p1.y + 32, (int)p2.x + 32, (int)p2.y + 32, 0xFF0000FF);
			}
		}
		for (size_t i = 1; i < replayTrail.size(); ++i) {
			const Vector2& p1 = replayTrail[i - 1], & p2 = replayTrail[i];
			if ((p2.x - p1.x) * (p2.x - p1.x) + (p2.y - p1.y) * (p2.y - p1.y) < 2500.0f) {
				Novice::DrawLine((int)p1.x + 32, (int)p1.y + 32, (int)p2.x + 32, (int)p2.y + 32, 0x0000FFFF);
			}
		}

		// プレイヤー中心に目印
		Vector2 center = player->GetPosition();
		Novice::DrawBox((int)center.x + 30, (int)center.y + 30, 4, 4, 0.0f, 0x000000FF, kFillModeSolid);

		// モード表示
		if (isReplaying) {
			Novice::ScreenPrintf(10, 10, "Mode: Replay [TAB to toggle]");
			Novice::ScreenPrintf(10, 30, "Trail Color: Blue");
		} else if (waitForRecordStart) {
			Novice::ScreenPrintf(10, 10, "Mode: Replay Ended");
			Novice::ScreenPrintf(10, 30, "Press R to start recording.");
		} else {
			Novice::ScreenPrintf(10, 10, "Mode: Record [TAB to toggle]");
			Novice::ScreenPrintf(10, 30, "Trail Color: Red");
		}

		// ImGuiの再生セレクタ
		ImGui::Begin("Replay Selector");
		static int selectedIndex = -1;
		static std::string selectedFile;
		auto replayFiles = GetReplayFileList();

		for (int i = 0; i < replayFiles.size(); ++i) {
			bool isSelected = (selectedIndex == i);
			if (ImGui::Selectable(replayFiles[i].c_str(), isSelected)) {
				selectedIndex = i;
				selectedFile = replayFiles[i];
			}
		}

		// ボタンで読み込み
		if (ImGui::Button("Load Selected Replay") && selectedIndex >= 0) {
			recorder->LoadFromFile(selectedFile);
			isReplaying = true;
			time = 0.0f;
			replayIndex = 0;
			player->Reset();
		}
		ImGui::End();

		// 時計
		time += deltaTime;

		// 下に黒バー
		Novice::DrawBox(0, 520, 1280, 520, 0.0f, 0x000000FF, kFillModeSolid);
		Novice::EndFrame();

		// ESCキーで終了
		if (!preKeys[DIK_ESCAPE] && keys[DIK_ESCAPE]) {
			break;
		}
	}

	// 終了時の一度だけ自動保存（記録中のみ）
	static bool hasSaved = false;
	if (!isReplaying && !waitForRecordStart && !hasSaved) {
		std::string filename = "replay_" + std::to_string(replayIndex++) + ".json";
		recorder->SaveToFile(filename);
		hasSaved = true;
	}

	// 後片付け
	Novice::Finalize();
	//for (const auto& entry : std::filesystem::directory_iterator(".")) {
	//	if (entry.path().extension() == ".json" && entry.path().filename().string().find("replay_") == 0) {
	//		std::filesystem::remove(entry.path());
	//	}
	//

	delete player;
	delete recorder;

	return 0;
}
