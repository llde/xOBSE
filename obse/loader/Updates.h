#pragma once
enum class UpdateType {
	None, Major, Minor, Hotfix,
};

std::tuple<UpdateType, std::string> CheckForUpdate();