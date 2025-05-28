#include <Novice.h>
#include "Player.h"
#include "InputRecorder.h"
#include <vector>
#include <string>
#include <filesystem>
#include "imgui.h"

// ウィンドウタイトル
const char kWindowTitle[] = "LC1C_24_マキノハルト_タイトル";

// 状態フラグ
bool isReplaying = false;
// 仮想入力（再生用）
char replayKeys[256] = {};

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

	std::remove("replay.json");

	// Novice初期化
	Novice::Initialize(kWindowTitle, 1280, 720);

	// キー入力用配列
	char keys[256] = { 0 };
	char preKeys[256] = { 0 };

	// プレイヤー生成
	Player* player = new Player();

	// 入力記録用
	InputRecorder* recorder = new InputRecorder();

	// 時間管理
	float time = 0.0f;
	float deltaTime = 1.0f / 60.0f;

	// 再生データ
	recorder->LoadFromFile("replay.json");
	int replayIndex = 0;

	std::filesystem::remove("replay.json");

	bool waitForRecordStart = false;  // Rキー待ち状態か

	// メインループ
	while (Novice::ProcessMessage() == 0) {
		Novice::BeginFrame();

		memcpy(preKeys, keys, 256);
		Novice::GetHitKeyStateAll(keys);

		// Tabキーでモード切り替え（再生 / 記録）
		if (keys[DIK_TAB] && !preKeys[DIK_TAB]) {
			isReplaying = !isReplaying;

			player->Reset();
			time = 0.0f;
			replayIndex = 0;

			if (isReplaying) {
				// 録画データを自動保存（まるごと1本）
				static int autoSaveIndex = 0;
				std::string filename = "replay_" + std::to_string(autoSaveIndex++) + ".json";
				recorder->SaveToFile(filename);

				recorder->LoadFromFile("replay.json");

				if (recorder->GetData().empty()) {
					isReplaying = false;
				}
			}
		}


		if (keys[DIK_S] && !preKeys[DIK_S]) {
			std::string filename = "replay_" + std::to_string(replayIndex) + ".json";
			recorder->SaveToFile(filename);
			replayIndex++;
		}


		// 入力処理
		if (isReplaying) {
			// 仮想キーを毎回初期化
			memset(replayKeys, 0, 256);

			// 毎回最新の入力データを参照（前のデータが混ざらないように）
			const auto& inputs = recorder->GetData();

			// 入力データに応じて仮想キーに反映
			while (replayIndex < inputs.size() && inputs[replayIndex].time <= time) {
				std::string input = inputs[replayIndex].input;
				if (input == "Left") {
					replayKeys[DIK_LEFT] = 1;
				} else if (input == "Right") {
					replayKeys[DIK_RIGHT] = 1;
				} else if (input == "Jump") {
					replayKeys[DIK_SPACE] = 1;
				}
				replayIndex++;
			}

			// 再生が最後まで終わったら完全にリセット
			if (!inputs.empty()) {
				float lastInputTime = inputs.back().time;
				if (time > lastInputTime + 1.0f) { // ← ★ ここ変更
					std::remove("replay.json");
					recorder->Clear();
					time = 0.0f;
					replayIndex = 0;
					isReplaying = false;
					waitForRecordStart = true;
				}
			}


			// 仮想キーでプレイヤーを動かす
			player->Update(replayKeys,true);
		}
		else if (!waitForRecordStart) {
			// 通常の記録処理
			recorder->Record(time, keys);
			player->Update(keys,false);

			
		}


		// Rキーが押されたら記録再開
		if (waitForRecordStart && keys[DIK_R] && !preKeys[DIK_R]) {
			waitForRecordStart = false;  // Rキー待ち終了
			time = 0.0f;
			replayIndex = 0;
			recorder->Clear();           // 念のため新規記録に備えて初期化
			player->Reset();      
			player->ClearTrail();// プレイヤー初期化
		}

		// プレイヤー描画
		player->Draw();

		const auto& trail = player->GetTrail();
		const auto& replayTrail = player->GetReplayTrail();
		// 記録中の軌跡（赤）
		for (size_t i = 1; i < trail.size(); ++i) {
			const Vector2& p1 = trail[i - 1];
			const Vector2& p2 = trail[i];

			float dx = p2.x - p1.x;
			float dy = p2.y - p1.y;
			if ((dx * dx + dy * dy) < (50.0f * 50.0f)) {
				Novice::DrawLine((int)p1.x + 32, (int)p1.y + 32, (int)p2.x + 32, (int)p2.y + 32, 0xFF0000FF);
			}
		}

		// 再生中の軌跡（青）
		for (size_t i = 1; i < replayTrail.size(); ++i) {
			const Vector2& p1 = replayTrail[i - 1];
			const Vector2& p2 = replayTrail[i];

			float dx = p2.x - p1.x;
			float dy = p2.y - p1.y;
			if ((dx * dx + dy * dy) < (50.0f * 50.0f)) {
				Novice::DrawLine((int)p1.x + 32, (int)p1.y + 32, (int)p2.x + 32, (int)p2.y + 32, 0x0000FFFF);
			}
		}




		// プレイヤー中心に黒点（4x4）を表示（目印として）
		Vector2 center = player->GetPosition();  // プレイヤー左上
		Novice::DrawBox((int)center.x + 32 - 2, (int)center.y + 32 - 2, 4, 4, 0.0f, 0x000000FF, kFillModeSolid);


		

		// モード表示（Replay / Record + 軌跡の色）
		if (isReplaying) {
			Novice::ScreenPrintf(10, 10, "Mode: Replay [TAB to toggle]");
			Novice::ScreenPrintf(10, 30, "Trail Color: Blue");
		} else if (waitForRecordStart) {
			Novice::ScreenPrintf(10, 10, "Mode: Replay Ended");
			Novice::ScreenPrintf(10, 30, "Press R to start recording.");
			Novice::ScreenPrintf(10, 50, "Trail Color: (None)");
		} else {
			Novice::ScreenPrintf(10, 10, "Mode: Record [TAB to toggle]");
			Novice::ScreenPrintf(10, 30, "Trail Color: Red");
		}

		// --- ImGuiの描画開始（ImGuiを使っている前提） ---
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

		// ボタンで読み込む
		if (ImGui::Button("Load Selected Replay") && selectedIndex >= 0) {
			recorder->LoadFromFile(selectedFile);
			isReplaying = true;
			time = 0.0f;
			replayIndex = 0;
			player->Reset();
		}

		ImGui::End();


		// 取得して表示だけする例（デバッグ用）
		
		for (size_t i = 0; i < replayFiles.size(); ++i) {
			Novice::ScreenPrintf(400, 30 + 20 * (int)i, replayFiles[i].c_str());
		}


		time += deltaTime;

		Novice::DrawBox(0, 520, 1280, 520, 0.0f, 0x000000FF, kFillModeSolid); 

		Novice::EndFrame();

		// ESCキーで終了
		if (preKeys[DIK_ESCAPE] == 0 && keys[DIK_ESCAPE] != 0) {
			break;
		}
	}

	// 保存トリガー
	static bool hasSaved = false;
	if (!isReplaying && !waitForRecordStart && !hasSaved) {
		std::string filename = "replay_" + std::to_string(replayIndex) + ".json";
		recorder->SaveToFile(filename);
		replayIndex++;
		hasSaved = true; // 一度だけ保存
	}


	Novice::Finalize();

	for (const auto& entry : std::filesystem::directory_iterator(".")) {
		if (entry.path().extension() == ".json" && entry.path().filename().string().find("replay_") == 0) {
			std::filesystem::remove(entry.path());
		}
	}

	// メモリ解放
	delete player;
	delete recorder;

	return 0;
}
